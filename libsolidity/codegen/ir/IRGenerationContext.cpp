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
#include <libsolidity/ast/AST.h>

#include <libdevcore/Whiskers.h>
#include <libdevcore/StringUtils.h>

using namespace dev;
using namespace dev::solidity;
using namespace std;

string IRGenerationContext::addLocalVariable(VariableDeclaration const& _varDecl)
{
	solUnimplementedAssert(
		_varDecl.annotation().type->sizeOnStack() == 1,
		"Multi-slot types not yet implemented."
	);

	return m_localVariables[&_varDecl] = "vloc_" + _varDecl.name() + "_" + to_string(_varDecl.id());
}

string IRGenerationContext::localVariableName(VariableDeclaration const& _varDecl)
{
	solAssert(
		m_localVariables.count(&_varDecl),
		"Unknown variable: " + _varDecl.name()
	);
	return m_localVariables[&_varDecl];
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

FunctionDefinition const& IRGenerationContext::virtualFunction(FunctionDefinition const& _function)
{
	// @TODO previously, we had to distinguish creation context and runtime context,
	// but since we do not work with jump positions anymore, this should not be a problem, right?
	string name = _function.name();
	FunctionType functionType(_function);
	for (auto const& contract: m_inheritanceHierarchy)
		for (FunctionDefinition const* function: contract->definedFunctions())
			if (
				function->name() == name &&
				!function->isConstructor() &&
				FunctionType(*function).asCallableFunction(false)->hasEqualParameterTypes(functionType)
			)
				return *function;
	solAssert(false, "Super function " + name + " not found.");
}

string IRGenerationContext::virtualFunctionName(FunctionDefinition const& _functionDeclaration)
{
	return functionName(virtualFunction(_functionDeclaration));
}

string IRGenerationContext::newYulVariable()
{
	return "_" + to_string(++m_varCounter);
}

string IRGenerationContext::variable(Expression const& _expression)
{
	unsigned size = _expression.annotation().type->sizeOnStack();
	string var = "expr_" + to_string(_expression.id());
	if (size == 1)
		return var;
	else
		return suffixedVariableNameList(move(var) + "_", 1, 1 + size);
}

string IRGenerationContext::variablePart(Expression const& _expression, size_t _part)
{
	size_t numVars = _expression.annotation().type->sizeOnStack();
	solAssert(numVars > 1, "");
	solAssert(1 <= _part && _part <= numVars, "");
	return "expr_" + to_string(_expression.id()) + "_" + to_string(_part);
}

string IRGenerationContext::internalDispatch(size_t _in, size_t _out)
{
	string funName = "dispatch_internal_in_" + to_string(_in) + "_out_" + to_string(_out);
	return m_functions->createFunction(funName, [&]() {
		Whiskers templ(R"(
			function <functionName>(fun <comma> <in>) <arrow> <out> {
				switch fun
				<#cases>
				case <funID>
				{
					<out> := <name>(<in>)
				}
				</cases>
				default { invalid() }
			}
		)");
		templ("functionName", funName);
		templ("comma", _in > 0 ? "," : "");
		YulUtilFunctions utils(m_evmVersion, m_functions);
		templ("in", suffixedVariableNameList("in_", 0, _in));
		templ("arrow", _out > 0 ? "->" : "");
		templ("out", suffixedVariableNameList("out_", 0, _out));
		vector<map<string, string>> functions;
		for (auto const& contract: m_inheritanceHierarchy)
			for (FunctionDefinition const* function: contract->definedFunctions())
				if (
					!function->isConstructor() &&
					function->parameters().size() == _in &&
					function->returnParameters().size() == _out
				)
				{
					// 0 is reserved for uninitialized function pointers
					solAssert(function->id() != 0, "Unexpected function ID: 0");

					functions.emplace_back(map<string, string> {
						{ "funID", to_string(function->id()) },
						{ "name", functionName(*function)}
					});
				}
		templ("cases", move(functions));
		return templ.render();
	});
}

YulUtilFunctions IRGenerationContext::utils()
{
	return YulUtilFunctions(m_evmVersion, m_functions);
}
