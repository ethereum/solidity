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

namespace solidity::frontend::smt
{

/**
 * Stores the context of the SMT encoding.
 */
class EncodingContext
{
public:
	EncodingContext();

	/// Resets the entire context except for symbolic variables which stay
	/// alive because of state variables and inlined function calls.
	/// To be used in the beginning of a root function visit.
	void reset();
	/// Clears the entire context, erasing everything.
	/// To be used before a model checking engine starts.
	void clear();

	/// Sets the current solver used by the current engine for
	/// SMT variable declaration.
	void setSolver(SolverInterface* _solver)
	{
		solAssert(_solver, "");
		m_solver = _solver;
	}

	/// Sets whether the context should conjoin assertions in the assertion stack.
	void setAssertionAccumulation(bool _acc) { m_accumulateAssertions = _acc; }

	/// Forwards variable creation to the solver.
	Expression newVariable(std::string _name, SortPointer _sort)
	{
		solAssert(m_solver, "");
		return m_solver->newVariable(move(_name), move(_sort));
	}

	/// Variables.
	//@{
	/// @returns the symbolic representation of a program variable.
	std::shared_ptr<SymbolicVariable> variable(frontend::VariableDeclaration const& _varDecl);
	/// @returns all symbolic variables.
	std::unordered_map<frontend::VariableDeclaration const*, std::shared_ptr<SymbolicVariable>> const& variables() const { return m_variables; }

	/// Creates a symbolic variable and
	/// @returns true if a variable's type is not supported and is therefore abstract.
	bool createVariable(frontend::VariableDeclaration const& _varDecl);
	/// @returns true if variable was created.
	bool knownVariable(frontend::VariableDeclaration const& _varDecl);

	/// Resets a specific variable.
	void resetVariable(frontend::VariableDeclaration const& _variable);
	/// Resets a set of variables.
	void resetVariables(std::set<frontend::VariableDeclaration const*> const& _variables);
	/// Resets variables according to a predicate.
	void resetVariables(std::function<bool(frontend::VariableDeclaration const&)> const& _filter);
	///Resets all variables.
	void resetAllVariables();

	/// Allocates a new index for the declaration, updates the current
	/// index to this value and returns the expression.
	Expression newValue(frontend::VariableDeclaration const& _decl);
	/// Sets the value of the declaration to zero.
	void setZeroValue(frontend::VariableDeclaration const& _decl);
	void setZeroValue(SymbolicVariable& _variable);
	/// Resets the variable to an unknown value (in its range).
	void setUnknownValue(frontend::VariableDeclaration const& decl);
	void setUnknownValue(SymbolicVariable& _variable);
	//@}

	/// Expressions.
	////@{
	/// @returns the symbolic representation of an AST node expression.
	std::shared_ptr<SymbolicVariable> expression(frontend::Expression const& _e);
	/// @returns all symbolic expressions.
	std::unordered_map<frontend::Expression const*, std::shared_ptr<SymbolicVariable>> const& expressions() const { return m_expressions; }

	/// Creates the expression (value can be arbitrary).
	/// @returns true if type is not supported.
	bool createExpression(frontend::Expression const& _e, std::shared_ptr<SymbolicVariable> _symbExpr = nullptr);
	/// Checks if expression was created.
	bool knownExpression(frontend::Expression const& _e) const;
	//@}

	/// Global variables and functions.
	//@{
	/// Global variables and functions.
	std::shared_ptr<SymbolicVariable> globalSymbol(std::string const& _name);
	/// @returns all symbolic globals.
	std::unordered_map<std::string, std::shared_ptr<SymbolicVariable>> const& globalSymbols() const { return m_globalContext; }

	/// Defines a new global variable or function
	/// and @returns true if type was abstracted.
	bool createGlobalSymbol(std::string const& _name, frontend::Expression const& _expr);
	/// Checks if special variable or function was seen.
	bool knownGlobalSymbol(std::string const& _var) const;
	//@}

	/// Blockchain.
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

	/// Solver.
	//@{
	/// @returns conjunction of all added assertions.
	Expression assertions();
	void pushSolver();
	void popSolver();
	void addAssertion(Expression const& _e);
	unsigned solverStackHeigh() { return m_assertions.size(); } const
	SolverInterface* solver()
	{
		solAssert(m_solver, "");
		return m_solver;
	}
	//@}

private:
	/// Adds _value to _account's balance.
	void addBalance(Expression _account, Expression _value);

	/// Symbolic expressions.
	//{@
	/// Symbolic variables.
	std::unordered_map<frontend::VariableDeclaration const*, std::shared_ptr<SymbolicVariable>> m_variables;

	/// Symbolic expressions.
	std::unordered_map<frontend::Expression const*, std::shared_ptr<SymbolicVariable>> m_expressions;

	/// Symbolic representation of global symbols including
	/// variables and functions.
	std::unordered_map<std::string, std::shared_ptr<smt::SymbolicVariable>> m_globalContext;

	/// Symbolic `this` address.
	std::unique_ptr<SymbolicAddressVariable> m_thisAddress;

	/// Symbolic balances.
	std::unique_ptr<SymbolicVariable> m_balances;
	//@}

	/// Solver related.
	//@{
	/// Solver can be SMT solver or Horn solver in the future.
	SolverInterface* m_solver = nullptr;

	/// Assertion stack.
	std::vector<Expression> m_assertions;

	/// Whether to conjoin assertions in the assertion stack.
	bool m_accumulateAssertions = true;
	//@}
};

}
