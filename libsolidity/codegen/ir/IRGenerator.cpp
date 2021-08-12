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

#include <libsolidity/codegen/ir/Common.h>
#include <libsolidity/codegen/ir/IRGenerator.h>
#include <libsolidity/codegen/ir/IRGeneratorForStatements.h>

#include <libsolidity/ast/AST.h>
#include <libsolidity/ast/ASTVisitor.h>
#include <libsolidity/codegen/ABIFunctions.h>
#include <libsolidity/codegen/CompilerUtils.h>

#include <libyul/AssemblyStack.h>
#include <libyul/Utilities.h>

#include <libsolutil/Algorithms.h>
#include <libsolutil/CommonData.h>
#include <libsolutil/StringUtils.h>
#include <libsolutil/Whiskers.h>

#include <liblangutil/SourceReferenceFormatter.h>

#include <sstream>
#include <variant>

using namespace std;
using namespace solidity;
using namespace solidity::util;
using namespace solidity::frontend;

namespace
{

void verifyCallGraph(
	set<CallableDeclaration const*, ASTNode::CompareByID> const& _expectedCallables,
	set<FunctionDefinition const*> _generatedFunctions
)
{
	for (auto const& expectedCallable: _expectedCallables)
		if (auto const* expectedFunction = dynamic_cast<FunctionDefinition const*>(expectedCallable))
		{
			solAssert(
				_generatedFunctions.count(expectedFunction) == 1 || expectedFunction->isConstructor(),
				"No code generated for function " + expectedFunction->name() + " even though it is not a constructor."
			);
			_generatedFunctions.erase(expectedFunction);
		}

	solAssert(
		_generatedFunctions.size() == 0,
		"Of the generated functions " + toString(_generatedFunctions.size()) + " are not in the call graph."
	);
}

set<CallableDeclaration const*, ASTNode::CompareByID> collectReachableCallables(
	CallGraph const& _graph
)
{
	set<CallableDeclaration const*, ASTNode::CompareByID> reachableCallables;
	for (CallGraph::Node const& reachableNode: _graph.edges | ranges::views::keys)
		if (holds_alternative<CallableDeclaration const*>(reachableNode))
			reachableCallables.emplace(get<CallableDeclaration const*>(reachableNode));

	return reachableCallables;
}

}

pair<string, string> IRGenerator::run(
	ContractDefinition const& _contract,
	bytes const& _cborMetadata,
	map<ContractDefinition const*, string_view const> const& _otherYulSources
)
{
	string const ir = yul::reindent(generate(_contract, _cborMetadata, _otherYulSources));

	yul::AssemblyStack asmStack(m_evmVersion, yul::AssemblyStack::Language::StrictAssembly, m_optimiserSettings);
	if (!asmStack.parseAndAnalyze("", ir))
	{
		string errorMessage;
		for (auto const& error: asmStack.errors())
			errorMessage += langutil::SourceReferenceFormatter::formatErrorInformation(
				*error,
				asmStack.charStream("")
			);
		solAssert(false, ir + "\n\nInvalid IR generated:\n" + errorMessage + "\n");
	}
	asmStack.optimize();

	string warning =
		"/*=====================================================*\n"
		" *                       WARNING                       *\n"
		" *  Solidity to Yul compilation is still EXPERIMENTAL  *\n"
		" *       It can result in LOSS OF FUNDS or worse       *\n"
		" *                !USE AT YOUR OWN RISK!               *\n"
		" *=====================================================*/\n\n";

	return {warning + ir, warning + asmStack.print()};
}

string IRGenerator::generate(
	ContractDefinition const& _contract,
	bytes const& _cborMetadata,
	map<ContractDefinition const*, string_view const> const& _otherYulSources
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
		/// @use-src <useSrcMap>
		object "<CreationObject>" {
			code {
				<sourceLocationComment>
				<memoryInitCreation>
				<callValueCheck>
				<?library>
				<!library>
				<?constructorHasParams> let <constructorParams> := <copyConstructorArguments>() </constructorHasParams>
				<constructor>(<constructorParams>)
				</library>
				<deploy>
				<functions>
			}
			object "<DeployedObject>" {
				code {
					<sourceLocationComment>
					<memoryInitDeployed>
					<?library>
					let called_via_delegatecall := iszero(eq(loadimmutable("<library_address>"), address()))
					</library>
					<dispatch>
					<deployedFunctions>
				}
				<deployedSubObjects>
				data "<metadataName>" hex"<cborMetadata>"
			}
			<subObjects>
		}
	)");

	resetContext(_contract);
	for (VariableDeclaration const* var: ContractType(_contract).immutableVariables())
		m_context.registerImmutableVariable(*var);

	auto invertedSourceIndicies = invertMap(m_context.sourceIndices());

	string useSrcMap = joinHumanReadable(
		ranges::views::transform(invertedSourceIndicies, [](auto&& _pair) {
			return to_string(_pair.first) + ":" + escapeAndQuoteString(_pair.second);
		}),
		", "
	);

	t("useSrcMap", useSrcMap);
	t("sourceLocationComment", sourceLocationComment(_contract, m_context));

	t("CreationObject", IRNames::creationObject(_contract));
	t("library", _contract.isLibrary());

	FunctionDefinition const* constructor = _contract.constructor();
	t("callValueCheck", !constructor || !constructor->isPayable() ? callValueCheck() : "");
	vector<string> constructorParams;
	if (constructor && !constructor->parameters().empty())
	{
		for (size_t i = 0; i < CompilerUtils::sizeOnStack(constructor->parameters()); ++i)
			constructorParams.emplace_back(m_context.newYulVariable());
		t(
			"copyConstructorArguments",
			m_utils.copyConstructorArgumentsToMemoryFunction(_contract, IRNames::creationObject(_contract))
		);
	}
	t("constructorParams", joinHumanReadable(constructorParams));
	t("constructorHasParams", !constructorParams.empty());
	t("constructor", IRNames::constructor(_contract));

	t("deploy", deployCode(_contract));
	generateConstructors(_contract);
	set<FunctionDefinition const*> creationFunctionList = generateQueuedFunctions();
	InternalDispatchMap internalDispatchMap = generateInternalDispatchFunctions(_contract);

	t("functions", m_context.functionCollector().requestedFunctions());
	t("subObjects", subObjectSources(m_context.subObjectsCreated()));

	// This has to be called only after all other code generation for the creation object is complete.
	bool creationInvolvesAssembly = m_context.inlineAssemblySeen();
	t("memoryInitCreation", memoryInit(!creationInvolvesAssembly));

	resetContext(_contract);

	// NOTE: Function pointers can be passed from creation code via storage variables. We need to
	// get all the functions they could point to into the dispatch functions even if they're never
	// referenced by name in the deployed code.
	m_context.initializeInternalDispatch(move(internalDispatchMap));

	// Do not register immutables to avoid assignment.
	t("DeployedObject", IRNames::deployedObject(_contract));
	t("library_address", IRNames::libraryAddressImmutable());
	t("dispatch", dispatchRoutine(_contract));
	set<FunctionDefinition const*> deployedFunctionList = generateQueuedFunctions();
	generateInternalDispatchFunctions(_contract);
	t("deployedFunctions", m_context.functionCollector().requestedFunctions());
	t("deployedSubObjects", subObjectSources(m_context.subObjectsCreated()));
	t("metadataName", yul::Object::metadataName());
	t("cborMetadata", toHex(_cborMetadata));


	// This has to be called only after all other code generation for the deployed object is complete.
	bool deployedInvolvesAssembly = m_context.inlineAssemblySeen();
	t("memoryInitDeployed", memoryInit(!deployedInvolvesAssembly));

	solAssert(_contract.annotation().creationCallGraph->get() != nullptr, "");
	solAssert(_contract.annotation().deployedCallGraph->get() != nullptr, "");
	verifyCallGraph(collectReachableCallables(**_contract.annotation().creationCallGraph), move(creationFunctionList));
	verifyCallGraph(collectReachableCallables(**_contract.annotation().deployedCallGraph), move(deployedFunctionList));

	return t.render();
}

string IRGenerator::generate(Block const& _block)
{
	IRGeneratorForStatements generator(m_context, m_utils);
	generator.generate(_block);
	return generator.code();
}

set<FunctionDefinition const*> IRGenerator::generateQueuedFunctions()
{
	set<FunctionDefinition const*> functions;

	while (!m_context.functionGenerationQueueEmpty())
	{
		FunctionDefinition const& functionDefinition = *m_context.dequeueFunctionForCodeGeneration();

		functions.emplace(&functionDefinition);
		// NOTE: generateFunction() may modify function generation queue
		generateFunction(functionDefinition);
	}

	return functions;
}

InternalDispatchMap IRGenerator::generateInternalDispatchFunctions(ContractDefinition const& _contract)
{
	solAssert(
		m_context.functionGenerationQueueEmpty(),
		"At this point all the enqueued functions should have been generated. "
		"Otherwise the dispatch may be incomplete."
	);

	InternalDispatchMap internalDispatchMap = m_context.consumeInternalDispatchMap();
	for (YulArity const& arity: internalDispatchMap | ranges::views::keys)
	{
		string funName = IRNames::internalDispatch(arity);
		m_context.functionCollector().createFunction(funName, [&]() {
			Whiskers templ(R"(
				<sourceLocationComment>
				function <functionName>(fun<?+in>, <in></+in>) <?+out>-> <out></+out> {
					switch fun
					<#cases>
					case <funID>
					{
						<?+out> <out> :=</+out> <name>(<in>)
					}
					</cases>
					default { <panic>() }
				}
				<sourceLocationComment>
			)");
			templ("sourceLocationComment", sourceLocationComment(_contract, m_context));
			templ("functionName", funName);
			templ("panic", m_utils.panicFunction(PanicCode::InvalidInternalFunction));
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
					{"funID", to_string(m_context.internalFunctionID(*function, true))},
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
		m_context.resetLocalVariables();
		Whiskers t(R"(
			<sourceLocationComment>
			function <functionName>(<params>)<?+retParams> -> <retParams></+retParams> {
				<retInit>
				<body>
			}
			<contractSourceLocationComment>
		)");

		t("sourceLocationComment", sourceLocationComment(_function, m_context));
		t(
			"contractSourceLocationComment",
			sourceLocationComment(m_context.mostDerivedContract(), m_context)
		);

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
		t("retInit", retInit);

		if (_function.modifiers().empty())
			t("body", generate(_function.body()));
		else
		{
			for (size_t i = 0; i < _function.modifiers().size(); ++i)
			{
				ModifierInvocation const& modifier = *_function.modifiers().at(i);
				string next =
					i + 1 < _function.modifiers().size() ?
					IRNames::modifierInvocation(*_function.modifiers().at(i + 1)) :
					IRNames::functionWithModifierInner(_function);
				generateModifier(modifier, _function, next);
			}
			t("body",
				(retParams.empty() ? string{} : joinHumanReadable(retParams) + " := ") +
				IRNames::modifierInvocation(*_function.modifiers().at(0)) +
				"(" +
				joinHumanReadable(retParams + params) +
				")"
			);
			// Now generate the actual inner function.
			generateFunctionWithModifierInner(_function);
		}
		return t.render();
	});
}

string IRGenerator::generateModifier(
	ModifierInvocation const& _modifierInvocation,
	FunctionDefinition const& _function,
	string const& _nextFunction
)
{
	string functionName = IRNames::modifierInvocation(_modifierInvocation);
	return m_context.functionCollector().createFunction(functionName, [&]() {
		m_context.resetLocalVariables();
		Whiskers t(R"(
			<sourceLocationComment>
			function <functionName>(<params>)<?+retParams> -> <retParams></+retParams> {
				<assignRetParams>
				<evalArgs>
				<body>
			}
			<contractSourceLocationComment>
		)");
		t("functionName", functionName);
		vector<string> retParamsIn;
		for (auto const& varDecl: _function.returnParameters())
			retParamsIn += m_context.addLocalVariable(*varDecl).stackSlots();
		vector<string> params = retParamsIn;
		for (auto const& varDecl: _function.parameters())
			params += m_context.addLocalVariable(*varDecl).stackSlots();
		t("params", joinHumanReadable(params));
		vector<string> retParams;
		string assignRetParams;
		for (size_t i = 0; i < retParamsIn.size(); ++i)
		{
			retParams.emplace_back(m_context.newYulVariable());
			assignRetParams += retParams.back() + " := " + retParamsIn[i] + "\n";
		}
		t("retParams", joinHumanReadable(retParams));
		t("assignRetParams", assignRetParams);

		ModifierDefinition const* modifier = dynamic_cast<ModifierDefinition const*>(
			_modifierInvocation.name().annotation().referencedDeclaration
		);
		solAssert(modifier, "");
		t("sourceLocationComment", sourceLocationComment(*modifier, m_context));
		t(
			"contractSourceLocationComment",
			sourceLocationComment(m_context.mostDerivedContract(), m_context)
		);

		switch (*_modifierInvocation.name().annotation().requiredLookup)
		{
		case VirtualLookup::Virtual:
			modifier = &modifier->resolveVirtual(m_context.mostDerivedContract());
			solAssert(modifier, "");
			break;
		case VirtualLookup::Static:
			break;
		case VirtualLookup::Super:
			solAssert(false, "");
		}

		solAssert(
			modifier->parameters().empty() ==
			(!_modifierInvocation.arguments() || _modifierInvocation.arguments()->empty()),
			""
		);
		IRGeneratorForStatements expressionEvaluator(m_context, m_utils);
		if (_modifierInvocation.arguments())
			for (size_t i = 0; i < _modifierInvocation.arguments()->size(); i++)
			{
				IRVariable argument = expressionEvaluator.evaluateExpression(
					*_modifierInvocation.arguments()->at(i),
					*modifier->parameters()[i]->annotation().type
				);
				expressionEvaluator.define(
					m_context.addLocalVariable(*modifier->parameters()[i]),
					argument
				);
			}

		t("evalArgs", expressionEvaluator.code());
		IRGeneratorForStatements generator(m_context, m_utils, [&]() {
			string ret = joinHumanReadable(retParams);
			return
				(ret.empty() ? "" : ret + " := ") +
				_nextFunction + "(" + joinHumanReadable(params) + ")\n";
		});
		generator.generate(modifier->body());
		t("body", generator.code());
		return t.render();
	});
}

string IRGenerator::generateFunctionWithModifierInner(FunctionDefinition const& _function)
{
	string functionName = IRNames::functionWithModifierInner(_function);
	return m_context.functionCollector().createFunction(functionName, [&]() {
		m_context.resetLocalVariables();
		Whiskers t(R"(
			<sourceLocationComment>
			function <functionName>(<params>)<?+retParams> -> <retParams></+retParams> {
				<assignRetParams>
				<body>
			}
			<contractSourceLocationComment>
		)");
		t("sourceLocationComment", sourceLocationComment(_function, m_context));
		t(
			"contractSourceLocationComment",
			sourceLocationComment(m_context.mostDerivedContract(), m_context)
		);
		t("functionName", functionName);
		vector<string> retParams;
		vector<string> retParamsIn;
		for (auto const& varDecl: _function.returnParameters())
			retParams += m_context.addLocalVariable(*varDecl).stackSlots();
		string assignRetParams;
		for (size_t i = 0; i < retParams.size(); ++i)
		{
			retParamsIn.emplace_back(m_context.newYulVariable());
			assignRetParams += retParams.back() + " := " + retParamsIn[i] + "\n";
		}
		vector<string> params = retParamsIn;
		for (auto const& varDecl: _function.parameters())
			params += m_context.addLocalVariable(*varDecl).stackSlots();
		t("params", joinHumanReadable(params));
		t("retParams", joinHumanReadable(retParams));
		t("assignRetParams", assignRetParams);
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
				<sourceLocationComment>
				function <functionName>() -> rval {
					rval := loadimmutable("<id>")
				}
				<contractSourceLocationComment>
			)")
			("sourceLocationComment", sourceLocationComment(_varDecl, m_context))
			(
				"contractSourceLocationComment",
				sourceLocationComment(m_context.mostDerivedContract(), m_context)
			)
			("functionName", functionName)
			("id", to_string(_varDecl.id()))
			.render();
		}
		else if (_varDecl.isConstant())
		{
			solAssert(paramTypes.empty(), "");
			return Whiskers(R"(
				<sourceLocationComment>
				function <functionName>() -> <ret> {
					<ret> := <constantValueFunction>()
				}
				<contractSourceLocationComment>
			)")
			("sourceLocationComment", sourceLocationComment(_varDecl, m_context))
			(
				"contractSourceLocationComment",
				sourceLocationComment(m_context.mostDerivedContract(), m_context)
			)
			("functionName", functionName)
			("constantValueFunction", IRGeneratorForStatements(m_context, m_utils).constantValueFunction(_varDecl))
			("ret", suffixedVariableNameList("ret_", 0, _varDecl.type()->sizeOnStack()))
			.render();
		}

		string code;

		auto const& location = m_context.storageLocationOfStateVariable(_varDecl);
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
		Type const* currentType = _varDecl.annotation().type;

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

			Whiskers templ(R"(
				<?array>
					if iszero(lt(<keys>, <length>(slot))) { revert(0, 0) }
				</array>
				slot<?array>, offset</array> := <indexAccess>(slot<?+keys>, <keys></+keys>)
			)");
			templ(
				"indexAccess",
				mappingType ?
				m_utils.mappingIndexAccessFunction(*mappingType, *mappingType->keyType()) :
				m_utils.storageArrayIndexAccessFunction(*arrayType)
			)
			("array", arrayType != nullptr)
			("keys", joinHumanReadable(keys));
			if (arrayType)
				templ("length", m_utils.arrayLengthFunction(*arrayType));

			code += templ.render();

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
				if (
					auto const* arrayType = dynamic_cast<ArrayType const*>(returnTypes[i]);
					arrayType && !arrayType->isByteArray()
				)
					continue;

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
			auto const* arrayType = dynamic_cast<ArrayType const*>(returnTypes.front());
			if (arrayType)
				solAssert(arrayType->isByteArray(), "");
			vector<string> retVars = IRVariable("ret", *returnTypes.front()).stackSlots();
			returnVariables += retVars;
			code += Whiskers(R"(
				<ret> := <readStorage>(slot, offset)
			)")
			("ret", joinHumanReadable(retVars))
			("readStorage", m_utils.readFromStorageDynamic(*returnTypes.front(), true))
			.render();
		}

		return Whiskers(R"(
			<sourceLocationComment>
			function <functionName>(<params>) -> <retVariables> {
				<code>
			}
			<contractSourceLocationComment>
		)")
		("functionName", functionName)
		("params", joinHumanReadable(parameters))
		("retVariables", joinHumanReadable(returnVariables))
		("code", std::move(code))
		("sourceLocationComment", sourceLocationComment(_varDecl, m_context))
		(
			"contractSourceLocationComment",
			sourceLocationComment(m_context.mostDerivedContract(), m_context)
		)
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
	struct InheritanceOrder
	{
		bool operator()(ContractDefinition const* _c1, ContractDefinition const* _c2) const
		{
			solAssert(contains(linearizedBaseContracts, _c1) && contains(linearizedBaseContracts, _c2), "");
			auto it1 = find(linearizedBaseContracts.begin(), linearizedBaseContracts.end(), _c1);
			auto it2 = find(linearizedBaseContracts.begin(), linearizedBaseContracts.end(), _c2);
			return it1 < it2;
		}
		vector<ContractDefinition const*> const& linearizedBaseContracts;
	} inheritanceOrder{_contract.annotation().linearizedBaseContracts};

	map<ContractDefinition const*, vector<string>> constructorParams;

	map<ContractDefinition const*, std::vector<ASTPointer<Expression>>const *, InheritanceOrder>
		baseConstructorArguments(inheritanceOrder);

	for (ASTPointer<InheritanceSpecifier> const& base: _contract.baseContracts())
		if (FunctionDefinition const* baseConstructor = dynamic_cast<ContractDefinition const*>(
				base->name().annotation().referencedDeclaration
		)->constructor(); baseConstructor && base->arguments())
			solAssert(baseConstructorArguments.emplace(
				dynamic_cast<ContractDefinition const*>(baseConstructor->scope()),
				base->arguments()
			).second, "");

	if (FunctionDefinition const* constructor = _contract.constructor())
		for (ASTPointer<ModifierInvocation> const& modifier: constructor->modifiers())
			if (auto const* baseContract = dynamic_cast<ContractDefinition const*>(
				modifier->name().annotation().referencedDeclaration
			))
				if (
					FunctionDefinition const* baseConstructor = baseContract->constructor();
					baseConstructor && modifier->arguments()
				)
					solAssert(baseConstructorArguments.emplace(
						dynamic_cast<ContractDefinition const*>(baseConstructor->scope()),
						modifier->arguments()
					).second, "");

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


void IRGenerator::generateConstructors(ContractDefinition const& _contract)
{
	auto listAllParams =
		[&](map<ContractDefinition const*, vector<string>> const& baseParams) -> vector<string>
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

		m_context.resetLocalVariables();
		m_context.functionCollector().createFunction(IRNames::constructor(*contract), [&]() {
			Whiskers t(R"(
				<sourceLocationComment>
				function <functionName>(<params><comma><baseParams>) {
					<evalBaseArguments>
					<sourceLocationComment>
					<?hasNextConstructor> <nextConstructor>(<nextParams>) </hasNextConstructor>
					<initStateVariables>
					<userDefinedConstructorBody>
				}
				<contractSourceLocationComment>
			)");
			vector<string> params;
			if (contract->constructor())
				for (ASTPointer<VariableDeclaration> const& varDecl: contract->constructor()->parameters())
					params += m_context.addLocalVariable(*varDecl).stackSlots();

			t("sourceLocationComment", sourceLocationComment(
				contract->constructor() ?
				contract->constructor()->location() :
				contract->location(),
				m_context
			));
			t(
				"contractSourceLocationComment",
				sourceLocationComment(m_context.mostDerivedContract(), m_context)
			);

			t("params", joinHumanReadable(params));
			vector<string> baseParams = listAllParams(baseConstructorParams);
			t("baseParams", joinHumanReadable(baseParams));
			t("comma", !params.empty() && !baseParams.empty() ? ", " : "");
			t("functionName", IRNames::constructor(*contract));
			pair<string, map<ContractDefinition const*, vector<string>>> evaluatedArgs = evaluateConstructorArguments(*contract);
			baseConstructorParams.insert(evaluatedArgs.second.begin(), evaluatedArgs.second.end());
			t("evalBaseArguments", evaluatedArgs.first);
			if (i < _contract.annotation().linearizedBaseContracts.size() - 1)
			{
				t("hasNextConstructor", true);
				ContractDefinition const* nextContract = _contract.annotation().linearizedBaseContracts[i + 1];
				t("nextConstructor", IRNames::constructor(*nextContract));
				t("nextParams", joinHumanReadable(listAllParams(baseConstructorParams)));
			}
			else
				t("hasNextConstructor", false);
			t("initStateVariables", initStateVariables(*contract));
			string body;
			if (FunctionDefinition const* constructor = contract->constructor())
			{
				vector<ModifierInvocation*> realModifiers;
				for (auto const& modifierInvocation: constructor->modifiers())
					// Filter out the base constructor calls
					if (dynamic_cast<ModifierDefinition const*>(modifierInvocation->name().annotation().referencedDeclaration))
						realModifiers.emplace_back(modifierInvocation.get());
				if (realModifiers.empty())
					body = generate(constructor->body());
				else
				{
					for (size_t i = 0; i < realModifiers.size(); ++i)
					{
						ModifierInvocation const& modifier = *realModifiers.at(i);
						string next =
							i + 1 < realModifiers.size() ?
							IRNames::modifierInvocation(*realModifiers.at(i + 1)) :
							IRNames::functionWithModifierInner(*constructor);
						generateModifier(modifier, *constructor, next);
					}
					body =
						IRNames::modifierInvocation(*realModifiers.at(0)) +
						"(" +
						joinHumanReadable(params) +
						")";
					// Now generate the actual inner function.
					generateFunctionWithModifierInner(*constructor);
				}
			}
			t("userDefinedConstructorBody", move(body));

			return t.render();
		});
	}
}

string IRGenerator::deployCode(ContractDefinition const& _contract)
{
	Whiskers t(R"X(
		let <codeOffset> := <allocateUnbounded>()
		codecopy(<codeOffset>, dataoffset("<object>"), datasize("<object>"))
		<#immutables>
			setimmutable(<codeOffset>, "<immutableName>", <value>)
		</immutables>
		return(<codeOffset>, datasize("<object>"))
	)X");
	t("allocateUnbounded", m_utils.allocateUnboundedFunction());
	t("codeOffset", m_context.newYulVariable());
	t("object", IRNames::deployedObject(_contract));

	vector<map<string, string>> immutables;
	if (_contract.isLibrary())
	{
		solAssert(ContractType(_contract).immutableVariables().empty(), "");
		immutables.emplace_back(map<string, string>{
			{"immutableName"s, IRNames::libraryAddressImmutable()},
			{"value"s, "address()"}
		});

	}
	else
		for (VariableDeclaration const* immutable: ContractType(_contract).immutableVariables())
		{
			solUnimplementedAssert(immutable->type()->isValueType(), "");
			solUnimplementedAssert(immutable->type()->sizeOnStack() == 1, "");
			immutables.emplace_back(map<string, string>{
				{"immutableName"s, to_string(immutable->id())},
				{"value"s, "mload(" + to_string(m_context.immutableMemoryOffset(*immutable)) + ")"}
			});
		}
	t("immutables", std::move(immutables));
	return t.render();
}

string IRGenerator::callValueCheck()
{
	return "if callvalue() { " + m_utils.revertReasonIfDebugFunction("Ether sent to non-payable function") + "() }";
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
				<delegatecallCheck>
				<callValueCheck>
				<?+params>let <params> := </+params> <abiDecode>(4, calldatasize())
				<?+retParams>let <retParams> := </+retParams> <function>(<params>)
				let memPos := <allocateUnbounded>()
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
		string delegatecallCheck;
		if (_contract.isLibrary())
		{
			solAssert(!type->isPayable(), "");
			if (type->stateMutability() > StateMutability::View)
				// If the function is not a view function and is called without DELEGATECALL,
				// we revert.
				delegatecallCheck =
					"if iszero(called_via_delegatecall) { " +
					m_utils.revertReasonIfDebugFunction("Non-view function of library called without DELEGATECALL") +
					"() }";
		}
		templ["delegatecallCheck"] = delegatecallCheck;
		templ["callValueCheck"] = (type->isPayable() || _contract.isLibrary()) ? "" : callValueCheck();

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

		templ["allocateUnbounded"] = m_utils.allocateUnboundedFunction();
		templ["abiEncode"] = abiFunctions.tupleEncoder(type->returnParameterTypes(), type->returnParameterTypes(), _contract.isLibrary());
	}
	t("cases", functions);
	FunctionDefinition const* etherReceiver = _contract.receiveFunction();
	if (etherReceiver)
	{
		solAssert(!_contract.isLibrary(), "");
		t("receiveEther", m_context.enqueueFunctionForCodeGeneration(*etherReceiver) + "() stop()");
	}
	else
		t("receiveEther", "");
	if (FunctionDefinition const* fallback = _contract.fallbackFunction())
	{
		solAssert(!_contract.isLibrary(), "");
		string fallbackCode;
		if (!fallback->isPayable())
			fallbackCode += callValueCheck() + "\n";
		if (fallback->parameters().empty())
			fallbackCode += m_context.enqueueFunctionForCodeGeneration(*fallback) + "() stop()";
		else
		{
			solAssert(fallback->parameters().size() == 1 && fallback->returnParameters().size() == 1, "");
			fallbackCode += "let retval := " + m_context.enqueueFunctionForCodeGeneration(*fallback) + "(0, calldatasize())\n";
			fallbackCode += "return(add(retval, 0x20), mload(retval))\n";

		}

		t("fallback", fallbackCode);
	}
	else
		t("fallback", (
			etherReceiver ?
			m_utils.revertReasonIfDebugFunction("Unknown signature and no fallback defined") :
			m_utils.revertReasonIfDebugFunction("Contract does not have fallback nor receive functions")
		) + "()");
	return t.render();
}

string IRGenerator::memoryInit(bool _useMemoryGuard)
{
	// TODO: Remove once we have made sure it is safe, i.e. after "Yul memory objects lite".
	//       Also restore the tests removed in the commit that adds this comment.
	// _useMemoryGuard = false;
	// This function should be called at the beginning of the EVM call frame
	// and thus can assume all memory to be zero, including the contents of
	// the "zero memory area" (the position CompilerUtils::zeroPointer points to).
	return
		Whiskers{
			_useMemoryGuard ?
			"mstore(<memPtr>, memoryguard(<freeMemoryStart>))" :
			"mstore(<memPtr>, <freeMemoryStart>)"
		}
		("memPtr", to_string(CompilerUtils::freeMemoryPointer))
		(
			"freeMemoryStart",
			to_string(CompilerUtils::generalPurposeMemoryStart + m_context.reservedMemory())
		).render();
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
	IRGenerationContext newContext(m_evmVersion, m_context.revertStrings(), m_optimiserSettings, m_context.sourceIndices());
	newContext.copyFunctionIDsFrom(m_context);
	m_context = move(newContext);

	m_context.setMostDerivedContract(_contract);
	for (auto const& var: ContractType(_contract).stateVariables())
		m_context.addStateVariable(*get<0>(var), get<1>(var), get<2>(var));
}
