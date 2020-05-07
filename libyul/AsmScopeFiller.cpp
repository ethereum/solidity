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
 * Module responsible for registering identifiers inside their scopes.
 */

#include <libyul/AsmScopeFiller.h>

#include <libyul/AsmData.h>
#include <libyul/AsmScope.h>
#include <libyul/AsmAnalysisInfo.h>
#include <libyul/Exceptions.h>

#include <liblangutil/ErrorReporter.h>

#include <libsolutil/CommonData.h>

#include <memory>
#include <functional>

using namespace std;
using namespace solidity;
using namespace solidity::yul;
using namespace solidity::util;
using namespace solidity::langutil;

ScopeFiller::ScopeFiller(AsmAnalysisInfo& _info, ErrorReporter& _errorReporter):
	m_info(_info), m_errorReporter(_errorReporter)
{
	m_currentScope = &scope(nullptr);
}

bool ScopeFiller::operator()(ExpressionStatement const& _expr)
{
	return std::visit(*this, _expr.expression);
}

bool ScopeFiller::operator()(VariableDeclaration const& _varDecl)
{
	for (auto const& variable: _varDecl.variables)
		if (!registerVariable(variable, _varDecl.location, *m_currentScope))
			return false;
	return true;
}

bool ScopeFiller::operator()(FunctionDefinition const& _funDef)
{
	auto virtualBlock = m_info.virtualBlocks[&_funDef] = make_shared<Block>();
	Scope& varScope = scope(virtualBlock.get());
	varScope.superScope = m_currentScope;
	m_currentScope = &varScope;
	varScope.functionScope = true;

	bool success = true;
	for (auto const& var: _funDef.parameters + _funDef.returnVariables)
		if (!registerVariable(var, _funDef.location, varScope))
			success = false;

	if (!(*this)(_funDef.body))
		success = false;

	yulAssert(m_currentScope == &varScope, "");
	m_currentScope = m_currentScope->superScope;

	return success;
}

bool ScopeFiller::operator()(If const& _if)
{
	return (*this)(_if.body);
}

bool ScopeFiller::operator()(Switch const& _switch)
{
	bool success = true;
	for (auto const& _case: _switch.cases)
		if (!(*this)(_case.body))
			success = false;
	return success;
}

bool ScopeFiller::operator()(ForLoop const& _forLoop)
{
	Scope* originalScope = m_currentScope;

	bool success = true;
	if (!(*this)(_forLoop.pre))
		success = false;
	m_currentScope = &scope(&_forLoop.pre);
	if (!std::visit(*this, *_forLoop.condition))
		success = false;
	if (!(*this)(_forLoop.body))
		success = false;
	if (!(*this)(_forLoop.post))
		success = false;

	m_currentScope = originalScope;

	return success;
}

bool ScopeFiller::operator()(Block const& _block)
{
	bool success = true;
	scope(&_block).superScope = m_currentScope;
	m_currentScope = &scope(&_block);

	// First visit all functions to make them create
	// an entry in the scope according to their visibility.
	for (auto const& s: _block.statements)
		if (holds_alternative<FunctionDefinition>(s))
			if (!registerFunction(std::get<FunctionDefinition>(s)))
				success = false;
	for (auto const& s: _block.statements)
		if (!std::visit(*this, s))
			success = false;

	m_currentScope = m_currentScope->superScope;
	return success;
}

bool ScopeFiller::registerVariable(TypedName const& _name, SourceLocation const& _location, Scope& _scope)
{
	if (!_scope.registerVariable(_name.name, _name.type))
	{
		//@TODO secondary location
		m_errorReporter.declarationError(
			1395_error,
			_location,
			"Variable name " + _name.name.str() + " already taken in this scope."
		);
		return false;
	}
	return true;
}

bool ScopeFiller::registerFunction(FunctionDefinition const& _funDef)
{
	vector<Scope::YulType> parameters;
	for (auto const& parameter: _funDef.parameters)
		parameters.emplace_back(parameter.type);
	vector<Scope::YulType> returns;
	for (auto const& returnVariable: _funDef.returnVariables)
		returns.emplace_back(returnVariable.type);
	if (!m_currentScope->registerFunction(_funDef.name, std::move(parameters), std::move(returns)))
	{
		//@TODO secondary location
		m_errorReporter.declarationError(
			6052_error,
			_funDef.location,
			"Function name " + _funDef.name.str() + " already taken in this scope."
		);
		return false;
	}
	return true;
}

Scope& ScopeFiller::scope(Block const* _block)
{
	auto& scope = m_info.scopes[_block];
	if (!scope)
		scope = make_shared<Scope>();
	return *scope;
}
