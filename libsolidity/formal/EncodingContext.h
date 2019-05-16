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

#pragma once

#include <libsolidity/formal/SolverInterface.h>
#include <libsolidity/formal/SymbolicVariables.h>

#include <unordered_map>
#include <set>

namespace dev
{
namespace solidity
{
namespace smt
{

/**
 * Stores the context of the SMT encoding.
 */
class EncodingContext
{
public:
	EncodingContext(SolverInterface& _solver);

	/// Resets the entire context.
	void reset();

	/// Methods related to variables.
	//@{
	/// @returns the symbolic representation of a program variable.
	std::shared_ptr<SymbolicVariable> variable(solidity::VariableDeclaration const& _varDecl);
	/// @returns all symbolic variables.
	std::unordered_map<solidity::VariableDeclaration const*, std::shared_ptr<SymbolicVariable>> const& variables() const { return m_variables; }

	/// Creates a symbolic variable and
	/// @returns true if a variable's type is not supported and is therefore abstract.
	bool createVariable(solidity::VariableDeclaration const& _varDecl);
	/// @returns true if variable was created.
	bool knownVariable(solidity::VariableDeclaration const& _varDecl);

	/// Resets a specific variable.
	void resetVariable(solidity::VariableDeclaration const& _variable);
	/// Resets a set of variables.
	void resetVariables(std::set<solidity::VariableDeclaration const*> const& _variables);
	/// Resets variables according to a predicate.
	void resetVariables(std::function<bool(solidity::VariableDeclaration const&)> const& _filter);
	///Resets all variables.
	void resetAllVariables();

	/// Allocates a new index for the declaration, updates the current
	/// index to this value and returns the expression.
	Expression newValue(solidity::VariableDeclaration const& _decl);
	/// Sets the value of the declaration to zero.
	void setZeroValue(solidity::VariableDeclaration const& _decl);
	void setZeroValue(SymbolicVariable& _variable);
	/// Resets the variable to an unknown value (in its range).
	void setUnknownValue(solidity::VariableDeclaration const& decl);
	void setUnknownValue(SymbolicVariable& _variable);
	//@}

	/// Methods related to expressions.
	////@{
	/// @returns the symbolic representation of an AST node expression.
	std::shared_ptr<SymbolicVariable> expression(solidity::Expression const& _e);
	/// @returns all symbolic expressions.
	std::unordered_map<solidity::Expression const*, std::shared_ptr<SymbolicVariable>> const& expressions() const { return m_expressions; }

	/// Creates the expression (value can be arbitrary).
	/// @returns true if type is not supported.
	bool createExpression(solidity::Expression const& _e, std::shared_ptr<SymbolicVariable> _symbExpr = nullptr);
	/// Checks if expression was created.
	bool knownExpression(solidity::Expression const& _e) const;
	//@}

	/// Blockchain related methods.
	//@{
	/// Value of `this` address.
	Expression thisAddress();
	/// @returns the symbolic balance of address `this`.
	Expression balance();
	/// @returns the symbolic balance of an address.
	Expression balance(Expression _address);
	/// Transfer _value from _from to _to.
	void transfer(Expression _from, Expression _to, Expression _value);
	//@}

private:
	/// Adds _value to _account's balance.
	void addBalance(Expression _account, Expression _value);

	SolverInterface& m_solver;

	/// Symbolic variables.
	std::unordered_map<solidity::VariableDeclaration const*, std::shared_ptr<SymbolicVariable>> m_variables;

	/// Symbolic expressions.
	std::unordered_map<solidity::Expression const*, std::shared_ptr<SymbolicVariable>> m_expressions;

	/// Symbolic `this` address.
	std::unique_ptr<SymbolicAddressVariable> m_thisAddress;

	/// Symbolic balances.
	std::unique_ptr<SymbolicVariable> m_balances;
};

}
}
}
