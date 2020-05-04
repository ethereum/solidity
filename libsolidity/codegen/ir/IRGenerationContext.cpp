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
/**
 * Class that contains contextual information during IR generation.
 */

#include <libsolidity/codegen/ir/IRGenerationContext.h>

#include <libsolidity/codegen/YulUtilFunctions.h>
#include <libsolidity/codegen/ABIFunctions.h>
#include <libsolidity/codegen/CompilerUtils.h>
#include <libsolidity/ast/AST.h>
#include <libsolidity/ast/TypeProvider.h>

#include <libsolutil/Whiskers.h>
#include <libsolutil/StringUtils.h>

using namespace std;
using namespace solidity;
using namespace solidity::util;
using namespace solidity::frontend;

string IRGenerationContext::enqueueFunctionForCodeGeneration(FunctionDefinition const& _function)
{
	string name = functionName(_function);

	if (!m_functions.contains(name))
		m_functionGenerationQueue.insert(&_function);

	return name;
}

FunctionDefinition const* IRGenerationContext::dequeueFunctionForCodeGeneration()
{
	solAssert(!m_functionGenerationQueue.empty(), "");

	FunctionDefinition const* result = *m_functionGenerationQueue.begin();
	m_functionGenerationQueue.erase(m_functionGenerationQueue.begin());
	return result;
}

ContractDefinition const& IRGenerationContext::mostDerivedContract() const
{
	solAssert(m_mostDerivedContract, "Most derived contract requested but not set.");
	return *m_mostDerivedContract;
}

IRVariable const& IRGenerationContext::addLocalVariable(VariableDeclaration const& _varDecl)
{
	auto const& [it, didInsert] = m_localVariables.emplace(
		std::make_pair(&_varDecl, IRVariable{_varDecl})
	);
	solAssert(didInsert, "Local variable added multiple times.");
	return it->second;
}

IRVariable const& IRGenerationContext::localVariable(VariableDeclaration const& _varDecl)
{
	solAssert(
		m_localVariables.count(&_varDecl),
		"Unknown variable: " + _varDecl.name()
	);
	return m_localVariables.at(&_varDecl);
}

void IRGenerationContext::registerImmutableVariable(VariableDeclaration const& _variable)
{
	solAssert(_variable.immutable(), "Attempted to register a non-immutable variable as immutable.");
	solUnimplementedAssert(
		_variable.annotation().type->isValueType(),
		"Only immutable variables of value type are supported."
	);
	solAssert(m_reservedMemory.has_value(), "Reserved memory has already been reset.");
	m_immutableVariables[&_variable] = CompilerUtils::generalPurposeMemoryStart + *m_reservedMemory;
	solAssert(_variable.annotation().type->memoryHeadSize() == 32, "Memory writes might overlap.");
	*m_reservedMemory += _variable.annotation().type->memoryHeadSize();
}

size_t IRGenerationContext::immutableMemoryOffset(VariableDeclaration const& _variable) const
{
	solAssert(
		m_immutableVariables.count(&_variable),
		"Unknown immutable variable: " + _variable.name()
	);
	return m_immutableVariables.at(&_variable);
}

size_t IRGenerationContext::reservedMemory()
{
	solAssert(m_reservedMemory.has_value(), "Reserved memory was used before.");
	size_t reservedMemory = *m_reservedMemory;
	m_reservedMemory = std::nullopt;
	return reservedMemory;
}

void IRGenerationContext::addStateVariable(
	VariableDeclaration const& _declaration,
	u256 _storageOffset,
	unsigned _byteOffset
)
{
	m_stateVariables[&_declaration] = make_pair(move(_storageOffset), _byteOffset);
}

string IRGenerationContext::functionName(FunctionDefinition const& _function)
{
	// @TODO previously, we had to distinguish creation context and runtime context,
	// but since we do not work with jump positions anymore, this should not be a problem, right?
	return "fun_" + _function.name() + "_" + to_string(_function.id());
}

string IRGenerationContext::functionName(VariableDeclaration const& _varDecl)
{
	return "getter_fun_" + _varDecl.name() + "_" + to_string(_varDecl.id());
}

string IRGenerationContext::creationObjectName(ContractDefinition const& _contract) const
{
	return _contract.name() + "_" + toString(_contract.id());
}
string IRGenerationContext::runtimeObjectName(ContractDefinition const& _contract) const
{
	return _contract.name() + "_" + toString(_contract.id()) + "_deployed";
}

string IRGenerationContext::newYulVariable()
{
	return "_" + to_string(++m_varCounter);
}

string IRGenerationContext::trySuccessConditionVariable(Expression const& _expression) const
{
	// NB: The TypeChecker already ensured that the Expression is of type FunctionCall.
	solAssert(
		static_cast<FunctionCallAnnotation const&>(_expression.annotation()).tryCall,
		"Parameter must be a FunctionCall with tryCall-annotation set."
	);

	return "trySuccessCondition_" + to_string(_expression.id());
}

string IRGenerationContext::internalDispatch(size_t _in, size_t _out)
{
	string funName = "dispatch_internal_in_" + to_string(_in) + "_out_" + to_string(_out);
	return m_functions.createFunction(funName, [&]() {
		Whiskers templ(R"(
			function <functionName>(fun <comma> <in>) <arrow> <out> {
				switch fun
				<#cases>
				case <funID>
				{
					<out> <assignment_op> <name>(<in>)
				}
				</cases>
				default { invalid() }
			}
		)");
		templ("functionName", funName);
		templ("comma", _in > 0 ? "," : "");
		YulUtilFunctions utils(m_evmVersion, m_revertStrings, m_functions);
		templ("in", suffixedVariableNameList("in_", 0, _in));
		templ("arrow", _out > 0 ? "->" : "");
		templ("assignment_op", _out > 0 ? ":=" : "");
		templ("out", suffixedVariableNameList("out_", 0, _out));

		// UNIMPLEMENTED: Internal library calls via pointers are not implemented yet.
		// We're not generating code for internal library functions here even though it's possible
		// to call them via pointers. Right now such calls end up triggering the `default` case in
		// the switch above.
		vector<map<string, string>> functions;
		for (auto const& contract: mostDerivedContract().annotation().linearizedBaseContracts)
			for (FunctionDefinition const* function: contract->definedFunctions())
				if (
					FunctionType const* functionType = TypeProvider::function(*function)->asCallableFunction(false);
					!function->isConstructor() &&
					TupleType(functionType->parameterTypes()).sizeOnStack() == _in &&
					TupleType(functionType->returnParameterTypes()).sizeOnStack() == _out
				)
				{
					// 0 is reserved for uninitialized function pointers
					solAssert(function->id() != 0, "Unexpected function ID: 0");

					functions.emplace_back(map<string, string> {
						{ "funID", to_string(function->id()) },
						{ "name", functionName(*function)}
					});

					enqueueFunctionForCodeGeneration(*function);
				}
		templ("cases", move(functions));
		return templ.render();
	});
}

YulUtilFunctions IRGenerationContext::utils()
{
	return YulUtilFunctions(m_evmVersion, m_revertStrings, m_functions);
}

ABIFunctions IRGenerationContext::abiFunctions()
{
	return ABIFunctions(m_evmVersion, m_revertStrings, m_functions);
}

std::string IRGenerationContext::revertReasonIfDebug(std::string const& _message)
{
	return YulUtilFunctions::revertReasonIfDebug(m_revertStrings, _message);
}

