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

#include <liblangutil/ErrorReporter.h>
#include <liblangutil/Exceptions.h>

#include <libdevcore/CommonData.h>

#include <boost/range/adaptor/reversed.hpp>

#include <memory>
#include <functional>

using namespace std;
using namespace dev;
using namespace langutil;
using namespace yul;
using namespace dev::solidity;

ScopeFiller::ScopeFiller(AsmAnalysisInfo& _info, ErrorReporter& _errorReporter):
	m_info(_info), m_errorReporter(_errorReporter)
{
	m_currentScope = &scope(nullptr);
}

bool ScopeFiller::operator()(ExpressionStatement const& _expr)
{
	return boost::apply_visitor(*this, _expr.expression);
}

bool ScopeFiller::operator()(Label const& _item)
{
	if (!m_currentScope->registerLabel(_item.name))
	{
		//@TODO secondary location
		m_errorReporter.declarationError(
			_item.location,
			"Label name " + _item.name.str() + " already taken in this scope."
		);
		return false;
	}
	return true;
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
	bool success = true;
	vector<Scope::YulType> arguments;
	for (auto const& _argument: _funDef.parameters)
		arguments.emplace_back(_argument.type.str());
	vector<Scope::YulType> returns;
	for (auto const& _return: _funDef.returnVariables)
		returns.emplace_back(_return.type.str());
	if (!m_currentScope->registerFunction(_funDef.name, arguments, returns))
	{
		//@TODO secondary location
		m_errorReporter.declarationError(
			_funDef.location,
			"Function name " + _funDef.name.str() + " already taken in this scope."
		);
		success = false;
	}

	auto virtualBlock = m_info.virtualBlocks[&_funDef] = make_shared<Block>();
	Scope& varScope = scope(virtualBlock.get());
	varScope.superScope = m_currentScope;
	m_currentScope = &varScope;
	varScope.functionScope = true;
	for (auto const& var: _funDef.parameters + _funDef.returnVariables)
		if (!registerVariable(var, _funDef.location, varScope))
			success = false;

	if (!(*this)(_funDef.body))
		success = false;

	solAssert(m_currentScope == &varScope, "");
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
	if (!boost::apply_visitor(*this, *_forLoop.condition))
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

	for (auto const& s: _block.statements)
		if (!boost::apply_visitor(*this, s))
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
			_location,
			"Variable name " + _name.name.str() + " already taken in this scope."
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
