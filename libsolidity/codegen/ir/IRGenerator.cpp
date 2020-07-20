/*
	This file is part of solidity.

	solidity is free software: you can redistribute it and/or modify
	it under the terms of the GNU General Public License as published by
	the Free Software Foundation, either version 3 of the License, or
	(at your option) any later version.

	solidity is distributed in the hope that it will be useful,
	but WITHOUT ANY WARRANTY; without even the implied warranty of
	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
	GNU General Public License for more details.

	You should have received a copy of the GNU General Public License
	along with solidity.  If not, see <http://www.gnu.org/licenses/>.
*/
// SPDX-License-Identifier: GPL-3.0
/**
 * @author Alex Beregszaszi
 * @date 2017
 * Component that translates Solidity code into Yul.
 */

#include <libsolidity/codegen/ir/IRGenerator.h>

#include <libsolidity/codegen/ir/IRGeneratorForStatements.h>

#include <libsolidity/ast/AST.h>
#include <libsolidity/ast/ASTVisitor.h>
#include <libsolidity/codegen/ABIFunctions.h>
#include <libsolidity/codegen/CompilerUtils.h>

#include <libyul/AssemblyStack.h>
#include <libyul/Utilities.h>

#include <libsolutil/CommonData.h>
#include <libsolutil/Whiskers.h>
#include <libsolutil/StringUtils.h>

#include <liblangutil/SourceReferenceFormatter.h>

#include <boost/range/adaptor/map.hpp>

#include <sstream>

using namespace std;
using namespace solidity;
using namespace solidity::util;
using namespace solidity::frontend;

pair<string, string> IRGenerator::run(
	ContractDefinition const& _contract,
	map<ContractDefinition const*, string const> const& _otherYulSources
)
{
	string const ir = yul::reindent(generate(_contract, _otherYulSources));

	yul::AssemblyStack asmStack(m_evmVersion, yul::AssemblyStack::Language::StrictAssembly, m_optimiserSettings);
	if (!asmStack.parseAndAnalyze("", ir))
	{
		string errorMessage;
		for (auto const& error: asmStack.errors())
			errorMessage += langutil::SourceReferenceFormatter::formatErrorInformation(*error);
		solAssert(false, ir + "\n\nInvalid IR generated:\n" + errorMessage + "\n");
	}
	asmStack.optimize();

	string warning =
		"/*******************************************************\n"
		" *                       WARNING                       *\n"
		" *  Solidity to Yul compilation is still EXPERIMENTAL  *\n"
		" *       It can result in LOSS OF FUNDS or worse       *\n"
		" *                !USE AT YOUR OWN RISK!               *\n"
		" *******************************************************/\n\n";

	return {warning + ir, warning + asmStack.print()};
}

string IRGenerator::generate(
	ContractDefinition const& _contract,
	map<ContractDefinition const*, string const> const& _otherYulSources
)
{
	auto subObjectSources = [&_otherYulSources](std::set<ContractDefinition const*, ASTNode::CompareByID> const& subObjects) -> string
	{
		std::string subObjectsSources;
		for (ContractDefinition const* subObject: subObjects)
			subObjectsSources += _otherYulSources.at(subObject);
		return subObjectsSources;
	};

	Whiskers t(R"(
		object "<CreationObject>" {
			code {
				<memoryInit>
				<callValueCheck>
				<?notLibrary>
				<?constructorHasParams> let <constructorParams> := <copyConstructorArguments>() </constructorHasParams>
				<implicitConstructor>(<constructorParams>)
				</notLibrary>
				<deploy>
				<functions>
			}
			object "<RuntimeObject>" {
				code {
					<memoryInit>
					<dispatch>
					<runtimeFunctions>
				}
				<runtimeSubObjects>
			}
			<subObjects>
		}
	)");

	resetContext(_contract);
	for (VariableDeclaration const* var: ContractType(_contract).immutableVariables())
		m_context.registerImmutableVariable(*var);

	t("CreationObject", IRNames::creationObject(_contract));
	t("memoryInit", memoryInit());
	t("notLibrary", !_contract.isLibrary());

	FunctionDefinition const* constructor = _contract.constructor();
	t("callValueCheck", !constructor || !constructor->isPayable() ? callValueCheck() : "");
	vector<string> constructorParams;
	if (constructor && !constructor->parameters().empty())
	{
		for (size_t i = 0; i < constructor->parameters().size(); ++i)
			constructorParams.emplace_back(m_context.newYulVariable());
		t(
			"copyConstructorArguments",
			m_utils.copyConstructorArgumentsToMemoryFunction(_contract, IRNames::creationObject(_contract))
		);
	}
	t("constructorParams", joinHumanReadable(constructorParams));
	t("constructorHasParams", !constructorParams.empty());
	t("implicitConstructor", IRNames::implicitConstructor(_contract));

	t("deploy", deployCode(_contract));
	generateImplicitConstructors(_contract);
	generateQueuedFunctions();
	InternalDispatchMap internalDispatchMap = generateInternalDispatchFunctions();
	t("functions", m_context.functionCollector().requestedFunctions());
	t("subObjects", subObjectSources(m_context.subObjectsCreated()));

	resetContext(_contract);

	// NOTE: Function pointers can be passed from creation code via storage variables. We need to
	// get all the functions they could point to into the dispatch functions even if they're never
	// referenced by name in the runtime code.
	m_context.initializeInternalDispatch(move(internalDispatchMap));

	// Do not register immutables to avoid assignment.
	t("RuntimeObject", IRNames::runtimeObject(_contract));
	t("dispatch", dispatchRoutine(_contract));
	generateQueuedFunctions();
	generateInternalDispatchFunctions();
	t("runtimeFunctions", m_context.functionCollector().requestedFunctions());
	t("runtimeSubObjects", subObjectSources(m_context.subObjectsCreated()));
	return t.render();
}

string IRGenerator::generate(Block const& _block)
{
	IRGeneratorForStatements generator(m_context, m_utils);
	_block.accept(generator);
	return generator.code();
}

void IRGenerator::generateQueuedFunctions()
{
	while (!m_context.functionGenerationQueueEmpty())
		// NOTE: generateFunction() may modify function generation queue
		generateFunction(*m_context.dequeueFunctionForCodeGeneration());
}

InternalDispatchMap IRGenerator::generateInternalDispatchFunctions()
{
	solAssert(
		m_context.functionGenerationQueueEmpty(),
		"At this point all the enqueued functions should have been generated. "
		"Otherwise the dispatch may be incomplete."
	);

	InternalDispatchMap internalDispatchMap = m_context.consumeInternalDispatchMap();
	for (YulArity const& arity: internalDispatchMap | boost::adaptors::map_keys)
	{
		string funName = IRNames::internalDispatch(arity);
		m_context.functionCollector().createFunction(funName, [&]() {
			Whiskers templ(R"(
				function <functionName>(fun<?+in>, <in></+in>) <?+out>-> <out></+out> {
					switch fun
					<#cases>
					case <funID>
					{
						<?+out> <out> :=</+out> <name>(<in>)
					}
					</cases>
					default { invalid() }
				}
			)");
			templ("functionName", funName);
			templ("in", suffixedVariableNameList("in_", 0, arity.in));
			templ("out", suffixedVariableNameList("out_", 0, arity.out));

			vector<map<string, string>> cases;
			for (FunctionDefinition const* function: internalDispatchMap.at(arity))
			{
				solAssert(function, "");
				solAssert(
					YulArity::fromType(*TypeProvider::function(*function, FunctionType::Kind::Internal)) == arity,
					"A single dispatch function can only handle functions of one arity"
				);
				solAssert(!function->isConstructor(), "");
				// 0 is reserved for uninitialized function pointers
				solAssert(function->id() != 0, "Unexpected function ID: 0");
				solAssert(m_context.functionCollector().contains(IRNames::function(*function)), "");

				cases.emplace_back(map<string, string>{
					{"funID", to_string(function->id())},
					{"name", IRNames::function(*function)}
				});
			}

			templ("cases", move(cases));
			return templ.render();
		});
	}

	solAssert(m_context.internalDispatchClean(), "");
	solAssert(
		m_context.functionGenerationQueueEmpty(),
		"Internal dispatch generation must not add new functions to generation queue because they won't be proeessed."
	);

	return internalDispatchMap;
}

string IRGenerator::generateFunction(FunctionDefinition const& _function)
{
	string functionName = IRNames::function(_function);
	return m_context.functionCollector().createFunction(functionName, [&]() {
		Whiskers t(R"(
			function <functionName>(<params>)<?+retParams> -> <retParams></+retParams> {
				<initReturnVariables>
				<body>
			}
		)");
		t("functionName", functionName);
		vector<string> params;
		for (auto const& varDecl: _function.parameters())
			params += m_context.addLocalVariable(*varDecl).stackSlots();
		t("params", joinHumanReadable(params));
		vector<string> retParams;
		string retInit;
		for (auto const& varDecl: _function.returnParameters())
		{
			retParams += m_context.addLocalVariable(*varDecl).stackSlots();
			retInit += generateInitialAssignment(*varDecl);
		}
		t("retParams", joinHumanReadable(retParams));
		t("initReturnVariables", retInit);
		t("body", generate(_function.body()));
		return t.render();
	});
}

string IRGenerator::generateGetter(VariableDeclaration const& _varDecl)
{
	string functionName = IRNames::function(_varDecl);
	return m_context.functionCollector().createFunction(functionName, [&]() {
		Type const* type = _varDecl.annotation().type;

		solAssert(_varDecl.isStateVariable(), "");

		FunctionType accessorType(_varDecl);
		TypePointers paramTypes = accessorType.parameterTypes();
		if (_varDecl.immutable())
		{
			solAssert(paramTypes.empty(), "");
			solUnimplementedAssert(type->sizeOnStack() == 1, "");
			return Whiskers(R"(
				function <functionName>() -> rval {
					rval := loadimmutable("<id>")
				}
			)")
			("functionName", functionName)
			("id", to_string(_varDecl.id()))
			.render();
		}
		else if (_varDecl.isConstant())
		{
			solAssert(paramTypes.empty(), "");
			return Whiskers(R"(
				function <functionName>() -> <ret> {
					<ret> := <constantValueFunction>()
				}
			)")
			("functionName", functionName)
			("constantValueFunction", IRGeneratorForStatements(m_context, m_utils).constantValueFunction(_varDecl))
			("ret", suffixedVariableNameList("ret_", 0, _varDecl.type()->sizeOnStack()))
			.render();
		}

		string code;

		auto const& location = m_context.storageLocationOfVariable(_varDecl);
		code += Whiskers(R"(
			let slot := <slot>
			let offset := <offset>
		)")
		("slot", location.first.str())
		("offset", to_string(location.second))
		.render();

		if (!paramTypes.empty())
			solAssert(
				location.second == 0,
				"If there are parameters, we are dealing with structs or mappings and thus should have offset zero."
			);

		// The code of an accessor is of the form `x[a][b][c]` (it is slightly more complicated
		// if the final type is a struct).
		// In each iteration of the loop below, we consume one parameter, perform an
		// index access, reassign the yul variable `slot` and move @a currentType further "down".
		// The initial value of @a currentType is only used if we skip the loop completely.
		TypePointer currentType = _varDecl.annotation().type;

		vector<string> parameters;
		vector<string> returnVariables;

		for (size_t i = 0; i < paramTypes.size(); ++i)
		{
			MappingType const* mappingType = dynamic_cast<MappingType const*>(currentType);
			ArrayType const* arrayType = dynamic_cast<ArrayType const*>(currentType);
			solAssert(mappingType || arrayType, "");

			vector<string> keys = IRVariable("key_" + to_string(i),
				mappingType ? *mappingType->keyType() : *TypeProvider::uint256()
			).stackSlots();
			parameters += keys;
			code += Whiskers(R"(
				slot<?array>, offset</array> := <indexAccess>(slot<?+keys>, <keys></+keys>)
			)")
			(
				"indexAccess",
				mappingType ?
				m_utils.mappingIndexAccessFunction(*mappingType, *mappingType->keyType()) :
				m_utils.storageArrayIndexAccessFunction(*arrayType)
			)
			("array", arrayType != nullptr)
			("keys", joinHumanReadable(keys))
			.render();

			currentType = mappingType ? mappingType->valueType() : arrayType->baseType();
		}

		auto returnTypes = accessorType.returnParameterTypes();
		solAssert(returnTypes.size() >= 1, "");
		if (StructType const* structType = dynamic_cast<StructType const*>(currentType))
		{
			solAssert(location.second == 0, "");
			auto const& names = accessorType.returnParameterNames();
			for (size_t i = 0; i < names.size(); ++i)
			{
				if (returnTypes[i]->category() == Type::Category::Mapping)
					continue;
				if (auto arrayType = dynamic_cast<ArrayType const*>(returnTypes[i]))
					if (!arrayType->isByteArray())
						continue;

				// TODO conversion from storage byte arrays is not yet implemented.
				pair<u256, unsigned> const& offsets = structType->storageOffsetsOfMember(names[i]);
				vector<string> retVars = IRVariable("ret_" + to_string(returnVariables.size()), *returnTypes[i]).stackSlots();
				returnVariables += retVars;
				code += Whiskers(R"(
					<ret> := <readStorage>(add(slot, <slotOffset>))
				)")
				("ret", joinHumanReadable(retVars))
				("readStorage", m_utils.readFromStorage(*returnTypes[i], offsets.second, true))
				("slotOffset", offsets.first.str())
				.render();
			}
		}
		else
		{
			solAssert(returnTypes.size() == 1, "");
			vector<string> retVars = IRVariable("ret", *returnTypes.front()).stackSlots();
			returnVariables += retVars;
			// TODO conversion from storage byte arrays is not yet implemented.
			code += Whiskers(R"(
				<ret> := <readStorage>(slot, offset)
			)")
			("ret", joinHumanReadable(retVars))
			("readStorage", m_utils.readFromStorageDynamic(*returnTypes.front(), true))
			.render();
		}

		return Whiskers(R"(
			function <functionName>(<params>) -> <retVariables> {
				<code>
			}
		)")
		("functionName", functionName)
		("params", joinHumanReadable(parameters))
		("retVariables", joinHumanReadable(returnVariables))
		("code", std::move(code))
		.render();
	});
}

string IRGenerator::generateInitialAssignment(VariableDeclaration const& _varDecl)
{
	IRGeneratorForStatements generator(m_context, m_utils);
	generator.initializeLocalVar(_varDecl);
	return generator.code();
}

pair<string, map<ContractDefinition const*, vector<string>>> IRGenerator::evaluateConstructorArguments(
	ContractDefinition const& _contract
)
{
	map<ContractDefinition const*, vector<string>> constructorParams;
	vector<pair<ContractDefinition const*, std::vector<ASTPointer<Expression>>const *>> baseConstructorArguments;

	for (ASTPointer<InheritanceSpecifier> const& base: _contract.baseContracts())
		if (FunctionDefinition const* baseConstructor = dynamic_cast<ContractDefinition const*>(
				base->name().annotation().referencedDeclaration
		)->constructor(); baseConstructor && base->arguments())
			baseConstructorArguments.emplace_back(
				dynamic_cast<ContractDefinition const*>(baseConstructor->scope()),
				base->arguments()
			);

	if (FunctionDefinition const* constructor = _contract.constructor())
		for (ASTPointer<ModifierInvocation> const& modifier: constructor->modifiers())
			if (auto const* baseContract = dynamic_cast<ContractDefinition const*>(
				modifier->name()->annotation().referencedDeclaration
			))
				if (
					FunctionDefinition const* baseConstructor = baseContract->constructor();
					baseConstructor && modifier->arguments()
				)
					baseConstructorArguments.emplace_back(
						dynamic_cast<ContractDefinition const*>(baseConstructor->scope()),
						modifier->arguments()
					);

	IRGeneratorForStatements generator{m_context, m_utils};
	for (auto&& [baseContract, arguments]: baseConstructorArguments)
	{
		solAssert(baseContract && arguments, "");
		if (baseContract->constructor() && !arguments->empty())
		{
			vector<string> params;
			for (size_t i = 0; i < arguments->size(); ++i)
				params += generator.evaluateExpression(
					*(arguments->at(i)),
					*(baseContract->constructor()->parameters()[i]->type())
				).stackSlots();
			constructorParams[baseContract] = std::move(params);
		}
	}

	return {generator.code(), constructorParams};
}

string IRGenerator::initStateVariables(ContractDefinition const& _contract)
{
	IRGeneratorForStatements generator{m_context, m_utils};
	for (VariableDeclaration const* variable: _contract.stateVariables())
		if (!variable->isConstant())
			generator.initializeStateVar(*variable);

	return generator.code();
}


void IRGenerator::generateImplicitConstructors(ContractDefinition const& _contract)
{
	auto listAllParams = [&](
		map<ContractDefinition const*, vector<string>> const& baseParams) -> vector<string>
	{
		vector<string> params;
		for (ContractDefinition const* contract: _contract.annotation().linearizedBaseContracts)
			if (baseParams.count(contract))
				params += baseParams.at(contract);
		return params;
	};

	map<ContractDefinition const*, vector<string>> baseConstructorParams;
	for (size_t i = 0; i < _contract.annotation().linearizedBaseContracts.size(); ++i)
	{
		ContractDefinition const* contract = _contract.annotation().linearizedBaseContracts[i];
		baseConstructorParams.erase(contract);

		m_context.functionCollector().createFunction(IRNames::implicitConstructor(*contract), [&]() {
			Whiskers t(R"(
				function <functionName>(<params><comma><baseParams>) {
					<evalBaseArguments>
					<?hasNextConstructor> <nextConstructor>(<nextParams>) </hasNextConstructor>
					<initStateVariables>
					<userDefinedConstructorBody>
				}
			)");
			vector<string> params;
			if (contract->constructor())
				for (ASTPointer<VariableDeclaration> const& varDecl: contract->constructor()->parameters())
					params += m_context.addLocalVariable(*varDecl).stackSlots();
			t("params", joinHumanReadable(params));
			vector<string> baseParams = listAllParams(baseConstructorParams);
			t("baseParams", joinHumanReadable(baseParams));
			t("comma", !params.empty() && !baseParams.empty() ? ", " : "");
			t("functionName", IRNames::implicitConstructor(*contract));
			pair<string, map<ContractDefinition const*, vector<string>>> evaluatedArgs = evaluateConstructorArguments(*contract);
			baseConstructorParams.insert(evaluatedArgs.second.begin(), evaluatedArgs.second.end());
			t("evalBaseArguments", evaluatedArgs.first);
			if (i < _contract.annotation().linearizedBaseContracts.size() - 1)
			{
				t("hasNextConstructor", true);
				ContractDefinition const* nextContract = _contract.annotation().linearizedBaseContracts[i + 1];
				t("nextConstructor", IRNames::implicitConstructor(*nextContract));
				t("nextParams", joinHumanReadable(listAllParams(baseConstructorParams)));
			}
			else
				t("hasNextConstructor", false);
			t("initStateVariables", initStateVariables(*contract));
			t("userDefinedConstructorBody", contract->constructor() ? generate(contract->constructor()->body()) : "");

			return t.render();
		});
	}
}

string IRGenerator::deployCode(ContractDefinition const& _contract)
{
	Whiskers t(R"X(
		<#loadImmutables>
			let <var> := mload(<memoryOffset>)
		</loadImmutables>

		codecopy(0, dataoffset("<object>"), datasize("<object>"))

		<#storeImmutables>
			setimmutable("<immutableName>", <var>)
		</storeImmutables>

		return(0, datasize("<object>"))
	)X");
	t("object", IRNames::runtimeObject(_contract));

	vector<map<string, string>> loadImmutables;
	vector<map<string, string>> storeImmutables;

	for (VariableDeclaration const* immutable: ContractType(_contract).immutableVariables())
	{
		solUnimplementedAssert(immutable->type()->isValueType(), "");
		solUnimplementedAssert(immutable->type()->sizeOnStack() == 1, "");
		string yulVar = m_context.newYulVariable();
		loadImmutables.emplace_back(map<string, string>{
			{"var"s, yulVar},
			{"memoryOffset"s, to_string(m_context.immutableMemoryOffset(*immutable))}
		});
		storeImmutables.emplace_back(map<string, string>{
			{"var"s, yulVar},
			{"immutableName"s, to_string(immutable->id())}
		});
	}
	t("loadImmutables", std::move(loadImmutables));
	// reverse order to ease stack strain
	reverse(storeImmutables.begin(), storeImmutables.end());
	t("storeImmutables", std::move(storeImmutables));
	return t.render();
}

string IRGenerator::callValueCheck()
{
	return "if callvalue() { revert(0, 0) }";
}

string IRGenerator::dispatchRoutine(ContractDefinition const& _contract)
{
	Whiskers t(R"X(
		if iszero(lt(calldatasize(), 4))
		{
			let selector := <shr224>(calldataload(0))
			switch selector
			<#cases>
			case <functionSelector>
			{
				// <functionName>
				<callValueCheck>
				<?+params>let <params> := </+params> <abiDecode>(4, calldatasize())
				<?+retParams>let <retParams> := </+retParams> <function>(<params>)
				let memPos := <allocate>(0)
				let memEnd := <abiEncode>(memPos <?+retParams>,</+retParams> <retParams>)
				return(memPos, sub(memEnd, memPos))
			}
			</cases>
			default {}
		}
		if iszero(calldatasize()) { <receiveEther> }
		<fallback>
	)X");
	t("shr224", m_utils.shiftRightFunction(224));
	vector<map<string, string>> functions;
	for (auto const& function: _contract.interfaceFunctions())
	{
		functions.emplace_back();
		map<string, string>& templ = functions.back();
		templ["functionSelector"] = "0x" + function.first.hex();
		FunctionTypePointer const& type = function.second;
		templ["functionName"] = type->externalSignature();
		templ["callValueCheck"] = type->isPayable() ? "" : callValueCheck();

		unsigned paramVars = make_shared<TupleType>(type->parameterTypes())->sizeOnStack();
		unsigned retVars = make_shared<TupleType>(type->returnParameterTypes())->sizeOnStack();

		ABIFunctions abiFunctions(m_evmVersion, m_context.revertStrings(), m_context.functionCollector());
		templ["abiDecode"] = abiFunctions.tupleDecoder(type->parameterTypes());
		templ["params"] = suffixedVariableNameList("param_", 0, paramVars);
		templ["retParams"] = suffixedVariableNameList("ret_", 0, retVars);

		if (FunctionDefinition const* funDef = dynamic_cast<FunctionDefinition const*>(&type->declaration()))
			templ["function"] = m_context.enqueueFunctionForCodeGeneration(*funDef);
		else if (VariableDeclaration const* varDecl = dynamic_cast<VariableDeclaration const*>(&type->declaration()))
			templ["function"] = generateGetter(*varDecl);
		else
			solAssert(false, "Unexpected declaration for function!");

		templ["allocate"] = m_utils.allocationFunction();
		templ["abiEncode"] = abiFunctions.tupleEncoder(type->returnParameterTypes(), type->returnParameterTypes(), false);
	}
	t("cases", functions);
	if (FunctionDefinition const* fallback = _contract.fallbackFunction())
	{
		string fallbackCode;
		if (!fallback->isPayable())
			fallbackCode += callValueCheck();
		fallbackCode += m_context.enqueueFunctionForCodeGeneration(*fallback) + "() stop()";

		t("fallback", fallbackCode);
	}
	else
		t("fallback", "revert(0, 0)");
	if (FunctionDefinition const* etherReceiver = _contract.receiveFunction())
		t("receiveEther", m_context.enqueueFunctionForCodeGeneration(*etherReceiver) + "() stop()");
	else
		t("receiveEther", "");
	return t.render();
}

string IRGenerator::memoryInit()
{
	// This function should be called at the beginning of the EVM call frame
	// and thus can assume all memory to be zero, including the contents of
	// the "zero memory area" (the position CompilerUtils::zeroPointer points to).
	return
		Whiskers{"mstore(<memPtr>, <freeMemoryStart>)"}
		("memPtr", to_string(CompilerUtils::freeMemoryPointer))
		("freeMemoryStart", to_string(CompilerUtils::generalPurposeMemoryStart + m_context.reservedMemory()))
		.render();
}

void IRGenerator::resetContext(ContractDefinition const& _contract)
{
	solAssert(
		m_context.functionGenerationQueueEmpty(),
		"Reset function generation queue while it still had functions."
	);
	solAssert(
		m_context.functionCollector().requestedFunctions().empty(),
		"Reset context while it still had functions."
	);
	solAssert(
		m_context.internalDispatchClean(),
		"Reset internal dispatch map without consuming it."
	);
	m_context = IRGenerationContext(m_evmVersion, m_context.revertStrings(), m_optimiserSettings);

	m_context.setMostDerivedContract(_contract);
	for (auto const& var: ContractType(_contract).stateVariables())
		m_context.addStateVariable(*get<0>(var), get<1>(var), get<2>(var));
}
