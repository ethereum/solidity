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

#include <libsolidity/formal/EncodingContext.h>

#include <libsolidity/formal/SymbolicTypes.h>

using namespace std;
using namespace dev;
using namespace dev::solidity::smt;

EncodingContext::EncodingContext():
	m_thisAddress(make_unique<SymbolicAddressVariable>("this", *this))
{
	auto sort = make_shared<ArraySort>(
		make_shared<Sort>(Kind::Int),
		make_shared<Sort>(Kind::Int)
	);
	m_balances = make_unique<SymbolicVariable>(sort, "balances", *this);
}

void EncodingContext::reset()
{
	resetAllVariables();
	m_expressions.clear();
	m_globalContext.clear();
	m_thisAddress->increaseIndex();
	m_balances->increaseIndex();
	m_assertions.clear();
}

void EncodingContext::clear()
{
	m_variables.clear();
	reset();
}

/// Variables.

shared_ptr<SymbolicVariable> EncodingContext::variable(solidity::VariableDeclaration const& _varDecl)
{
	solAssert(knownVariable(_varDecl), "");
	return m_variables[&_varDecl];
}

bool EncodingContext::createVariable(solidity::VariableDeclaration const& _varDecl)
{
	solAssert(!knownVariable(_varDecl), "");
	auto const& type = _varDecl.type();
	auto result = newSymbolicVariable(*type, _varDecl.name() + "_" + to_string(_varDecl.id()), *this);
	m_variables.emplace(&_varDecl, result.second);
	return result.first;
}

bool EncodingContext::knownVariable(solidity::VariableDeclaration const& _varDecl)
{
	return m_variables.count(&_varDecl);
}

void EncodingContext::resetVariable(solidity::VariableDeclaration const& _variable)
{
	newValue(_variable);
	setUnknownValue(_variable);
}

void EncodingContext::resetVariables(set<solidity::VariableDeclaration const*> const& _variables)
{
	for (auto const* decl: _variables)
		resetVariable(*decl);
}

void EncodingContext::resetVariables(function<bool(solidity::VariableDeclaration const&)> const& _filter)
{
	for_each(begin(m_variables), end(m_variables), [&](auto _variable)
	{
		if (_filter(*_variable.first))
			this->resetVariable(*_variable.first);
	});
}

void EncodingContext::resetAllVariables()
{
	resetVariables([&](solidity::VariableDeclaration const&) { return true; });
}

Expression EncodingContext::newValue(solidity::VariableDeclaration const& _decl)
{
	solAssert(knownVariable(_decl), "");
	return m_variables.at(&_decl)->increaseIndex();
}

void EncodingContext::setZeroValue(solidity::VariableDeclaration const& _decl)
{
	solAssert(knownVariable(_decl), "");
	setZeroValue(*m_variables.at(&_decl));
}

void EncodingContext::setZeroValue(SymbolicVariable& _variable)
{
	setSymbolicZeroValue(_variable, *this);
}

void EncodingContext::setUnknownValue(solidity::VariableDeclaration const& _decl)
{
	solAssert(knownVariable(_decl), "");
	setUnknownValue(*m_variables.at(&_decl));
}

void EncodingContext::setUnknownValue(SymbolicVariable& _variable)
{
	setSymbolicUnknownValue(_variable, *this);
}

/// Expressions

shared_ptr<SymbolicVariable> EncodingContext::expression(solidity::Expression const& _e)
{
	if (!knownExpression(_e))
		createExpression(_e);
	return m_expressions.at(&_e);
}

bool EncodingContext::createExpression(solidity::Expression const& _e, shared_ptr<SymbolicVariable> _symbVar)
{
	solAssert(_e.annotation().type, "");
	if (knownExpression(_e))
	{
		expression(_e)->increaseIndex();
		return false;
	}
	else if (_symbVar)
	{
		m_expressions.emplace(&_e, _symbVar);
		return false;
	}
	else
	{
		auto result = newSymbolicVariable(*_e.annotation().type, "expr_" + to_string(_e.id()), *this);
		m_expressions.emplace(&_e, result.second);
		return result.first;
	}
}

bool EncodingContext::knownExpression(solidity::Expression const& _e) const
{
	return m_expressions.count(&_e);
}

/// Global variables and functions.

shared_ptr<SymbolicVariable> EncodingContext::globalSymbol(string const& _name)
{
	solAssert(knownGlobalSymbol(_name), "");
	return m_globalContext.at(_name);
}

bool EncodingContext::createGlobalSymbol(string const& _name, solidity::Expression const& _expr)
{
	solAssert(!knownGlobalSymbol(_name), "");
	auto result = newSymbolicVariable(*_expr.annotation().type, _name, *this);
	m_globalContext.emplace(_name, result.second);
	setUnknownValue(*result.second);
	return result.first;
}

bool EncodingContext::knownGlobalSymbol(string const& _var) const
{
	return m_globalContext.count(_var);
}

// Blockchain

Expression EncodingContext::thisAddress()
{
	return m_thisAddress->currentValue();
}

Expression EncodingContext::balance()
{
	return balance(m_thisAddress->currentValue());
}

Expression EncodingContext::balance(Expression _address)
{
	return Expression::select(m_balances->currentValue(), move(_address));
}

void EncodingContext::transfer(Expression _from, Expression _to, Expression _value)
{
	unsigned indexBefore = m_balances->index();
	addBalance(_from, 0 - _value);
	addBalance(_to, move(_value));
	unsigned indexAfter = m_balances->index();
	solAssert(indexAfter > indexBefore, "");
	m_balances->increaseIndex();
	/// Do not apply the transfer operation if _from == _to.
	auto newBalances = Expression::ite(
		move(_from) == move(_to),
		m_balances->valueAtIndex(indexBefore),
		m_balances->valueAtIndex(indexAfter)
	);
	addAssertion(m_balances->currentValue() == newBalances);
}

/// Solver.

Expression EncodingContext::assertions()
{
	if (m_assertions.empty())
		return Expression(true);

	return m_assertions.back();
}

void EncodingContext::pushSolver()
{
	if (m_accumulateAssertions)
		m_assertions.push_back(assertions());
	else
		m_assertions.push_back(smt::Expression(true));
}

void EncodingContext::popSolver()
{
	solAssert(!m_assertions.empty(), "");
	m_assertions.pop_back();
}

void EncodingContext::addAssertion(Expression const& _expr)
{
	if (m_assertions.empty())
		m_assertions.push_back(_expr);
	else
		m_assertions.back() = _expr && move(m_assertions.back());
}

/// Private helpers.

void EncodingContext::addBalance(Expression _address, Expression _value)
{
	auto newBalances = Expression::store(
		m_balances->currentValue(),
		_address,
		balance(_address) + move(_value)
	);
	m_balances->increaseIndex();
	addAssertion(newBalances == m_balances->currentValue());
}
