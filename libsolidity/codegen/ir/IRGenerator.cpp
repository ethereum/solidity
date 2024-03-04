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

#include <libyul/YulStack.h>
#include <libyul/Utilities.h>

#include <libsolutil/Algorithms.h>
#include <libsolutil/CommonData.h>
#include <libsolutil/StringUtils.h>
#include <libsolutil/Whiskers.h>

#include <json/json.h>

#include <sstream>
#include <variant>

using namespace solidity;
using namespace solidity::frontend;
using namespace solidity::langutil;
using namespace solidity::util;
using namespace std::string_literals;

namespace
{

void verifyCallGraph(
	std::set<CallableDeclaration const*, ASTNode::CompareByID> const& _expectedCallables,
	std::set<FunctionDefinition const*> _generatedFunctions
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

std::set<CallableDeclaration const*, ASTNode::CompareByID> collectReachableCallables(
	CallGraph const& _graph
)
{
	std::set<CallableDeclaration const*, ASTNode::CompareByID> reachableCallables;
	for (CallGraph::Node const& reachableNode: _graph.edges | ranges::views::keys)
		if (std::holds_alternative<CallableDeclaration const*>(reachableNode))
			reachableCallables.emplace(std::get<CallableDeclaration const*>(reachableNode));

	return reachableCallables;
}

}

std::string IRGenerator::run(
	ContractDefinition const& _contract,
	bytes const& _cborMetadata,
	std::map<ContractDefinition const*, std::string_view const> const& _otherYulSources
)
{
	return yul::reindent(generate(_contract, _cborMetadata, _otherYulSources));
}

std::string IRGenerator::generate(
	ContractDefinition const& _contract,
	bytes const& _cborMetadata,
	std::map<ContractDefinition const*, std::string_view const> const& _otherYulSources
)
{
	auto subObjectSources = [&_otherYulSources](std::set<ContractDefinition const*, ASTNode::CompareByID> const& subObjects) -> std::string
	{
		std::string subObjectsSources;
		for (ContractDefinition const* subObject: subObjects)
			subObjectsSources += _otherYulSources.at(subObject);
		return subObjectsSources;
	};
	auto formatUseSrcMap = [](IRGenerationContext const& _context) -> std::string
	{
		return joinHumanReadable(
			ranges::views::transform(_context.usedSourceNames(), [_context](std::string const& _sourceName) {
				return std::to_string(_context.sourceIndices().at(_sourceName)) + ":" + escapeAndQuoteString(_sourceName);
			}),
			", "
		);
	};

	Whiskers t(R"(
		/// @use-src <useSrcMapCreation>
		object "<CreationObject>" {
			code {
				<sourceLocationCommentCreation>
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
			/// @use-src <useSrcMapDeployed>
			object "<DeployedObject>" {
				code {
					<sourceLocationCommentDeployed>
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

	resetContext(_contract, ExecutionContext::Creation);
	for (VariableDeclaration const* var: ContractType(_contract).immutableVariables())
		m_context.registerImmutableVariable(*var);

	t("CreationObject", IRNames::creationObject(_contract));
	t("sourceLocationCommentCreation", dispenseLocationComment(_contract));
	t("library", _contract.isLibrary());

	FunctionDefinition const* constructor = _contract.constructor();
	t("callValueCheck", !constructor || !constructor->isPayable() ? callValueCheck() : "");
	std::vector<std::string> constructorParams;
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
	std::set<FunctionDefinition const*> creationFunctionList = generateQueuedFunctions();
	InternalDispatchMap internalDispatchMap = generateInternalDispatchFunctions(_contract);

	t("functions", m_context.functionCollector().requestedFunctions());
	t("subObjects", subObjectSources(m_context.subObjectsCreated()));

	// This has to be called only after all other code generation for the creation object is complete.
	bool creationInvolvesMemoryUnsafeAssembly = m_context.memoryUnsafeInlineAssemblySeen();
	t("memoryInitCreation", memoryInit(!creationInvolvesMemoryUnsafeAssembly));
	t("useSrcMapCreation", formatUseSrcMap(m_context));

	resetContext(_contract, ExecutionContext::Deployed);

	// NOTE: Function pointers can be passed from creation code via storage variables. We need to
	// get all the functions they could point to into the dispatch functions even if they're never
	// referenced by name in the deployed code.
	m_context.initializeInternalDispatch(std::move(internalDispatchMap));

	// Do not register immutables to avoid assignment.
	t("DeployedObject", IRNames::deployedObject(_contract));
	t("sourceLocationCommentDeployed", dispenseLocationComment(_contract));
	t("library_address", IRNames::libraryAddressImmutable());
	t("dispatch", dispatchRoutine(_contract));
	std::set<FunctionDefinition const*> deployedFunctionList = generateQueuedFunctions();
	generateInternalDispatchFunctions(_contract);
	t("deployedFunctions", m_context.functionCollector().requestedFunctions());
	t("deployedSubObjects", subObjectSources(m_context.subObjectsCreated()));
	t("metadataName", yul::Object::metadataName());
	t("cborMetadata", util::toHex(_cborMetadata));

	t("useSrcMapDeployed", formatUseSrcMap(m_context));

	// This has to be called only after all other code generation for the deployed object is complete.
	bool deployedInvolvesMemoryUnsafeAssembly = m_context.memoryUnsafeInlineAssemblySeen();
	t("memoryInitDeployed", memoryInit(!deployedInvolvesMemoryUnsafeAssembly));

	solAssert(_contract.annotation().creationCallGraph->get() != nullptr, "");
	solAssert(_contract.annotation().deployedCallGraph->get() != nullptr, "");
	verifyCallGraph(collectReachableCallables(**_contract.annotation().creationCallGraph), std::move(creationFunctionList));
	verifyCallGraph(collectReachableCallables(**_contract.annotation().deployedCallGraph), std::move(deployedFunctionList));

	return t.render();
}

std::string IRGenerator::generate(Block const& _block)
{
	IRGeneratorForStatements generator(m_context, m_utils, m_optimiserSettings);
	generator.generate(_block);
	return generator.code();
}

std::set<FunctionDefinition const*> IRGenerator::generateQueuedFunctions()
{
	std::set<FunctionDefinition const*> functions;

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
		std::string funName = IRNames::internalDispatch(arity);
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
			templ("sourceLocationComment", dispenseLocationComment(_contract));
			templ("functionName", funName);
			templ("panic", m_utils.panicFunction(PanicCode::InvalidInternalFunction));
			templ("in", suffixedVariableNameList("in_", 0, arity.in));
			templ("out", suffixedVariableNameList("out_", 0, arity.out));

			std::vector<std::map<std::string, std::string>> cases;
			std::set<int64_t> caseValues;
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
				solAssert(caseValues.count(function->id()) == 0, "Duplicate function ID");
				solAssert(m_context.functionCollector().contains(IRNames::function(*function)), "");

				cases.emplace_back(std::map<std::string, std::string>{
					{"funID", std::to_string(m_context.mostDerivedContract().annotation().internalFunctionIDs.at(function))},
					{"name", IRNames::function(*function)}
				});
				caseValues.insert(function->id());
			}

			templ("cases", std::move(cases));
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

std::string IRGenerator::generateFunction(FunctionDefinition const& _function)
{
	std::string functionName = IRNames::function(_function);
	return m_context.functionCollector().createFunction(functionName, [&]() {
		m_context.resetLocalVariables();
		Whiskers t(R"(
			<astIDComment><sourceLocationComment>
			function <functionName>(<params>)<?+retParams> -> <retParams></+retParams> {
				<retInit>
				<body>
			}
			<contractSourceLocationComment>
		)");

		if (m_context.debugInfoSelection().astID)
			t("astIDComment", "/// @ast-id " + std::to_string(_function.id()) + "\n");
		else
			t("astIDComment", "");
		t("sourceLocationComment", dispenseLocationComment(_function));
		t(
			"contractSourceLocationComment",
			dispenseLocationComment(m_context.mostDerivedContract())
		);

		t("functionName", functionName);
		std::vector<std::string> params;
		for (auto const& varDecl: _function.parameters())
			params += m_context.addLocalVariable(*varDecl).stackSlots();
		t("params", joinHumanReadable(params));
		std::vector<std::string> retParams;
		std::string retInit;
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
				std::string next =
					i + 1 < _function.modifiers().size() ?
					IRNames::modifierInvocation(*_function.modifiers().at(i + 1)) :
					IRNames::functionWithModifierInner(_function);
				generateModifier(modifier, _function, next);
			}
			t("body",
				(retParams.empty() ? std::string{} : joinHumanReadable(retParams) + " := ") +
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

std::string IRGenerator::generateModifier(
	ModifierInvocation const& _modifierInvocation,
	FunctionDefinition const& _function,
	std::string const& _nextFunction
)
{
	std::string functionName = IRNames::modifierInvocation(_modifierInvocation);
	return m_context.functionCollector().createFunction(functionName, [&]() {
		m_context.resetLocalVariables();
		Whiskers t(R"(
			<astIDComment><sourceLocationComment>
			function <functionName>(<params>)<?+retParams> -> <retParams></+retParams> {
				<assignRetParams>
				<evalArgs>
				<body>
			}
			<contractSourceLocationComment>
		)");

		t("functionName", functionName);
		std::vector<std::string> retParamsIn;
		for (auto const& varDecl: _function.returnParameters())
			retParamsIn += m_context.addLocalVariable(*varDecl).stackSlots();
		std::vector<std::string> params = retParamsIn;
		for (auto const& varDecl: _function.parameters())
			params += m_context.addLocalVariable(*varDecl).stackSlots();
		t("params", joinHumanReadable(params));
		std::vector<std::string> retParams;
		std::string assignRetParams;
		for (size_t i = 0; i < retParamsIn.size(); ++i)
		{
			retParams.emplace_back(m_context.newYulVariable());
			assignRetParams += retParams.at(i) + " := " + retParamsIn.at(i) + "\n";
		}
		t("retParams", joinHumanReadable(retParams));
		t("assignRetParams", assignRetParams);

		ModifierDefinition const* modifier = dynamic_cast<ModifierDefinition const*>(
			_modifierInvocation.name().annotation().referencedDeclaration
		);
		solAssert(modifier, "");

		if (m_context.debugInfoSelection().astID)
			t("astIDComment", "/// @ast-id " + std::to_string(modifier->id()) + "\n");
		else
			t("astIDComment", "");
		t("sourceLocationComment", dispenseLocationComment(*modifier));
		t(
			"contractSourceLocationComment",
			dispenseLocationComment(m_context.mostDerivedContract())
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
		IRGeneratorForStatements expressionEvaluator(m_context, m_utils, m_optimiserSettings);
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
		IRGeneratorForStatements generator(m_context, m_utils, m_optimiserSettings, [&]() {
			std::string ret = joinHumanReadable(retParams);
			return
				(ret.empty() ? "" : ret + " := ") +
				_nextFunction + "(" + joinHumanReadable(params) + ")\n";
		});
		generator.generate(modifier->body());
		t("body", generator.code());
		return t.render();
	});
}

std::string IRGenerator::generateFunctionWithModifierInner(FunctionDefinition const& _function)
{
	std::string functionName = IRNames::functionWithModifierInner(_function);
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
		t("sourceLocationComment", dispenseLocationComment(_function));
		t(
			"contractSourceLocationComment",
			dispenseLocationComment(m_context.mostDerivedContract())
		);
		t("functionName", functionName);
		std::vector<std::string> retParams;
		std::vector<std::string> retParamsIn;
		for (auto const& varDecl: _function.returnParameters())
			retParams += m_context.addLocalVariable(*varDecl).stackSlots();
		std::string assignRetParams;
		for (size_t i = 0; i < retParams.size(); ++i)
		{
			retParamsIn.emplace_back(m_context.newYulVariable());
			assignRetParams += retParams.at(i) + " := " + retParamsIn.at(i) + "\n";
		}
		std::vector<std::string> params = retParamsIn;
		for (auto const& varDecl: _function.parameters())
			params += m_context.addLocalVariable(*varDecl).stackSlots();
		t("params", joinHumanReadable(params));
		t("retParams", joinHumanReadable(retParams));
		t("assignRetParams", assignRetParams);
		t("body", generate(_function.body()));
		return t.render();
	});
}

std::string IRGenerator::generateGetter(VariableDeclaration const& _varDecl)
{
	std::string functionName = IRNames::function(_varDecl);
	return m_context.functionCollector().createFunction(functionName, [&]() {
		Type const* type = _varDecl.annotation().type;

		solAssert(_varDecl.isStateVariable(), "");

		FunctionType accessorType(_varDecl);
		TypePointers paramTypes = accessorType.parameterTypes();
		if (_varDecl.immutable())
		{
			solAssert(paramTypes.empty(), "");
			solUnimplementedAssert(type->sizeOnStack() == 1);
			return Whiskers(R"(
				<astIDComment><sourceLocationComment>
				function <functionName>() -> rval {
					rval := loadimmutable("<id>")
				}
				<contractSourceLocationComment>
			)")
			(
				"astIDComment",
				m_context.debugInfoSelection().astID ?
					"/// @ast-id " + std::to_string(_varDecl.id()) + "\n" :
					""
			)
			("sourceLocationComment", dispenseLocationComment(_varDecl))
			(
				"contractSourceLocationComment",
				dispenseLocationComment(m_context.mostDerivedContract())
			)
			("functionName", functionName)
			("id", std::to_string(_varDecl.id()))
			.render();
		}
		else if (_varDecl.isConstant())
		{
			solAssert(paramTypes.empty(), "");
			return Whiskers(R"(
				<astIDComment><sourceLocationComment>
				function <functionName>() -> <ret> {
					<ret> := <constantValueFunction>()
				}
				<contractSourceLocationComment>
			)")
			(
				"astIDComment",
				m_context.debugInfoSelection().astID ?
					"/// @ast-id " + std::to_string(_varDecl.id()) + "\n" :
					""
			)
			("sourceLocationComment", dispenseLocationComment(_varDecl))
			(
				"contractSourceLocationComment",
				dispenseLocationComment(m_context.mostDerivedContract())
			)
			("functionName", functionName)
			("constantValueFunction", IRGeneratorForStatements(m_context, m_utils, m_optimiserSettings).constantValueFunction(_varDecl))
			("ret", suffixedVariableNameList("ret_", 0, _varDecl.type()->sizeOnStack()))
			.render();
		}

		std::string code;

		auto const& location = m_context.storageLocationOfStateVariable(_varDecl);
		code += Whiskers(R"(
			let slot := <slot>
			let offset := <offset>
		)")
		("slot", location.first.str())
		("offset", std::to_string(location.second))
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

		std::vector<std::string> parameters;
		std::vector<std::string> returnVariables;

		for (size_t i = 0; i < paramTypes.size(); ++i)
		{
			MappingType const* mappingType = dynamic_cast<MappingType const*>(currentType);
			ArrayType const* arrayType = dynamic_cast<ArrayType const*>(currentType);
			solAssert(mappingType || arrayType, "");

			std::vector<std::string> keys = IRVariable("key_" + std::to_string(i),
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
					arrayType && !arrayType->isByteArrayOrString()
				)
					continue;

				std::pair<u256, unsigned> const& offsets = structType->storageOffsetsOfMember(names[i]);
				std::vector<std::string> retVars = IRVariable("ret_" + std::to_string(returnVariables.size()), *returnTypes[i]).stackSlots();
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
				solAssert(arrayType->isByteArrayOrString(), "");
			std::vector<std::string> retVars = IRVariable("ret", *returnTypes.front()).stackSlots();
			returnVariables += retVars;
			code += Whiskers(R"(
				<ret> := <readStorage>(slot, offset)
			)")
			("ret", joinHumanReadable(retVars))
			("readStorage", m_utils.readFromStorageDynamic(*returnTypes.front(), true))
			.render();
		}

		return Whiskers(R"(
			<astIDComment><sourceLocationComment>
			function <functionName>(<params>) -> <retVariables> {
				<code>
			}
			<contractSourceLocationComment>
		)")
		("functionName", functionName)
		("params", joinHumanReadable(parameters))
		("retVariables", joinHumanReadable(returnVariables))
		("code", std::move(code))
		(
			"astIDComment",
			m_context.debugInfoSelection().astID ?
				"/// @ast-id " + std::to_string(_varDecl.id()) + "\n" :
				""
		)
		("sourceLocationComment", dispenseLocationComment(_varDecl))
		(
			"contractSourceLocationComment",
			dispenseLocationComment(m_context.mostDerivedContract())
		)
		.render();
	});
}

std::string IRGenerator::generateExternalFunction(ContractDefinition const& _contract, FunctionType const& _functionType)
{
	std::string functionName = IRNames::externalFunctionABIWrapper(_functionType.declaration());
	return m_context.functionCollector().createFunction(functionName, [&](std::vector<std::string>&, std::vector<std::string>&) -> std::string {
		Whiskers t(R"X(
			<callValueCheck>
			<?+params>let <params> := </+params> <abiDecode>(4, calldatasize())
			<?+retParams>let <retParams> := </+retParams> <function>(<params>)
			let memPos := <allocateUnbounded>()
			let memEnd := <abiEncode>(memPos <?+retParams>,</+retParams> <retParams>)
			return(memPos, sub(memEnd, memPos))
		)X");
		t("callValueCheck", (_functionType.isPayable() || _contract.isLibrary()) ? "" : callValueCheck());

		unsigned paramVars = std::make_shared<TupleType>(_functionType.parameterTypes())->sizeOnStack();
		unsigned retVars = std::make_shared<TupleType>(_functionType.returnParameterTypes())->sizeOnStack();

		ABIFunctions abiFunctions(m_evmVersion, m_context.revertStrings(), m_context.functionCollector());
		t("abiDecode", abiFunctions.tupleDecoder(_functionType.parameterTypes()));
		t("params",  suffixedVariableNameList("param_", 0, paramVars));
		t("retParams",  suffixedVariableNameList("ret_", 0, retVars));

		if (FunctionDefinition const* funDef = dynamic_cast<FunctionDefinition const*>(&_functionType.declaration()))
		{
			solAssert(!funDef->isConstructor());
			t("function", m_context.enqueueFunctionForCodeGeneration(*funDef));
		}
		else if (VariableDeclaration const* varDecl = dynamic_cast<VariableDeclaration const*>(&_functionType.declaration()))
			t("function", generateGetter(*varDecl));
		else
			solAssert(false, "Unexpected declaration for function!");

		t("allocateUnbounded", m_utils.allocateUnboundedFunction());
		t("abiEncode", abiFunctions.tupleEncoder(_functionType.returnParameterTypes(), _functionType.returnParameterTypes(), _contract.isLibrary()));
		return t.render();
	});
}

std::string IRGenerator::generateInitialAssignment(VariableDeclaration const& _varDecl)
{
	IRGeneratorForStatements generator(m_context, m_utils, m_optimiserSettings);
	generator.initializeLocalVar(_varDecl);
	return generator.code();
}

std::pair<std::string, std::map<ContractDefinition const*, std::vector<std::string>>> IRGenerator::evaluateConstructorArguments(
	ContractDefinition const& _contract
)
{
	struct InheritanceOrder
	{
		bool operator()(ContractDefinition const* _c1, ContractDefinition const* _c2) const
		{
			solAssert(util::contains(linearizedBaseContracts, _c1) && util::contains(linearizedBaseContracts, _c2), "");
			auto it1 = find(linearizedBaseContracts.begin(), linearizedBaseContracts.end(), _c1);
			auto it2 = find(linearizedBaseContracts.begin(), linearizedBaseContracts.end(), _c2);
			return it1 < it2;
		}
		std::vector<ContractDefinition const*> const& linearizedBaseContracts;
	} inheritanceOrder{_contract.annotation().linearizedBaseContracts};

	std::map<ContractDefinition const*, std::vector<std::string>> constructorParams;

	std::map<ContractDefinition const*, std::vector<ASTPointer<Expression>>const *, InheritanceOrder>
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

	IRGeneratorForStatements generator{m_context, m_utils, m_optimiserSettings};
	for (auto&& [baseContract, arguments]: baseConstructorArguments)
	{
		solAssert(baseContract && arguments, "");
		if (baseContract->constructor() && !arguments->empty())
		{
			std::vector<std::string> params;
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

std::string IRGenerator::initStateVariables(ContractDefinition const& _contract)
{
	IRGeneratorForStatements generator{m_context, m_utils, m_optimiserSettings};
	for (VariableDeclaration const* variable: _contract.stateVariables())
		if (!variable->isConstant())
			generator.initializeStateVar(*variable);

	return generator.code();
}


void IRGenerator::generateConstructors(ContractDefinition const& _contract)
{
	auto listAllParams =
		[&](std::map<ContractDefinition const*, std::vector<std::string>> const& baseParams) -> std::vector<std::string>
		{
			std::vector<std::string> params;
			for (ContractDefinition const* contract: _contract.annotation().linearizedBaseContracts)
				if (baseParams.count(contract))
					params += baseParams.at(contract);
			return params;
		};

	std::map<ContractDefinition const*, std::vector<std::string>> baseConstructorParams;
	for (size_t i = 0; i < _contract.annotation().linearizedBaseContracts.size(); ++i)
	{
		ContractDefinition const* contract = _contract.annotation().linearizedBaseContracts[i];
		baseConstructorParams.erase(contract);

		m_context.resetLocalVariables();
		m_context.functionCollector().createFunction(IRNames::constructor(*contract), [&]() {
			Whiskers t(R"(
				<astIDComment><sourceLocationComment>
				function <functionName>(<params><comma><baseParams>) {
					<evalBaseArguments>
					<sourceLocationComment>
					<?hasNextConstructor> <nextConstructor>(<nextParams>) </hasNextConstructor>
					<initStateVariables>
					<userDefinedConstructorBody>
				}
				<contractSourceLocationComment>
			)");
			std::vector<std::string> params;
			if (contract->constructor())
				for (ASTPointer<VariableDeclaration> const& varDecl: contract->constructor()->parameters())
					params += m_context.addLocalVariable(*varDecl).stackSlots();

			if (m_context.debugInfoSelection().astID && contract->constructor())
				t("astIDComment", "/// @ast-id " + std::to_string(contract->constructor()->id()) + "\n");
			else
				t("astIDComment", "");
			t("sourceLocationComment", dispenseLocationComment(
				contract->constructor() ?
				dynamic_cast<ASTNode const&>(*contract->constructor()) :
				dynamic_cast<ASTNode const&>(*contract)
			));
			t(
				"contractSourceLocationComment",
				dispenseLocationComment(m_context.mostDerivedContract())
			);

			t("params", joinHumanReadable(params));
			std::vector<std::string> baseParams = listAllParams(baseConstructorParams);
			t("baseParams", joinHumanReadable(baseParams));
			t("comma", !params.empty() && !baseParams.empty() ? ", " : "");
			t("functionName", IRNames::constructor(*contract));
			std::pair<std::string, std::map<ContractDefinition const*, std::vector<std::string>>> evaluatedArgs = evaluateConstructorArguments(*contract);
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
			std::string body;
			if (FunctionDefinition const* constructor = contract->constructor())
			{
				std::vector<ModifierInvocation*> realModifiers;
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
						std::string next =
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
			t("userDefinedConstructorBody", std::move(body));

			return t.render();
		});
	}
}

std::string IRGenerator::deployCode(ContractDefinition const& _contract)
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

	std::vector<std::map<std::string, std::string>> immutables;
	if (_contract.isLibrary())
	{
		solAssert(ContractType(_contract).immutableVariables().empty(), "");
		immutables.emplace_back(std::map<std::string, std::string>{
			{"immutableName"s, IRNames::libraryAddressImmutable()},
			{"value"s, "address()"}
		});

	}
	else
		for (VariableDeclaration const* immutable: ContractType(_contract).immutableVariables())
		{
			solUnimplementedAssert(immutable->type()->isValueType());
			solUnimplementedAssert(immutable->type()->sizeOnStack() == 1);
			immutables.emplace_back(std::map<std::string, std::string>{
				{"immutableName"s, std::to_string(immutable->id())},
				{"value"s, "mload(" + std::to_string(m_context.immutableMemoryOffset(*immutable)) + ")"}
			});
		}
	t("immutables", std::move(immutables));
	return t.render();
}

std::string IRGenerator::callValueCheck()
{
	return "if callvalue() { " + m_utils.revertReasonIfDebugFunction("Ether sent to non-payable function") + "() }";
}

std::string IRGenerator::dispatchRoutine(ContractDefinition const& _contract)
{
	Whiskers t(R"X(
		<?+cases>if iszero(lt(calldatasize(), 4))
		{
			let selector := <shr224>(calldataload(0))
			switch selector
			<#cases>
			case <functionSelector>
			{
				// <functionName>
				<delegatecallCheck>
				<externalFunction>()
			}
			</cases>
			default {}
		}</+cases>
		<?+receiveEther>if iszero(calldatasize()) { <receiveEther> }</+receiveEther>
		<fallback>
	)X");
	t("shr224", m_utils.shiftRightFunction(224));
	std::vector<std::map<std::string, std::string>> functions;
	for (auto const& function: _contract.interfaceFunctions())
	{
		functions.emplace_back();
		std::map<std::string, std::string>& templ = functions.back();
		templ["functionSelector"] = "0x" + function.first.hex();
		FunctionTypePointer const& type = function.second;
		templ["functionName"] = type->externalSignature();
		std::string delegatecallCheck;
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

		templ["externalFunction"] = generateExternalFunction(_contract, *type);
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
		std::string fallbackCode;
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

std::string IRGenerator::memoryInit(bool _useMemoryGuard)
{
	// This function should be called at the beginning of the EVM call frame
	// and thus can assume all memory to be zero, including the contents of
	// the "zero memory area" (the position CompilerUtils::zeroPointer points to).
	return
		Whiskers{
			_useMemoryGuard ?
			"mstore(<memPtr>, memoryguard(<freeMemoryStart>))" :
			"mstore(<memPtr>, <freeMemoryStart>)"
		}
		("memPtr", std::to_string(CompilerUtils::freeMemoryPointer))
		(
			"freeMemoryStart",
			std::to_string(CompilerUtils::generalPurposeMemoryStart + m_context.reservedMemory())
		).render();
}

void IRGenerator::resetContext(ContractDefinition const& _contract, ExecutionContext _context)
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
	IRGenerationContext newContext(
		m_evmVersion,
		_context,
		m_context.revertStrings(),
		m_context.sourceIndices(),
		m_context.debugInfoSelection(),
		m_context.soliditySourceProvider()
	);
	m_context = std::move(newContext);

	m_context.setMostDerivedContract(_contract);
	for (auto const& var: ContractType(_contract).stateVariables())
		m_context.addStateVariable(*std::get<0>(var), std::get<1>(var), std::get<2>(var));
}

std::string IRGenerator::dispenseLocationComment(ASTNode const& _node)
{
	return ::dispenseLocationComment(_node, m_context);
}
