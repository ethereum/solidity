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
 * Model checker based on Constrained Horn Clauses.
 *
 * A Solidity contract's CFG is encoded into a system of Horn clauses where
 * each block has a predicate and edges are rules.
 *
 * The entry block is the constructor which has no in-edges.
 * The constructor has one out-edge to an artificial block named _Interface_
 * which has in/out-edges from/to all public functions.
 *
 * Loop invariants for Interface -> Interface' are state invariants.
 */

#pragma once

#include <libsolidity/formal/SMTEncoder.h>

#include <libsolidity/formal/CHCSolverInterface.h>

#include <set>

namespace dev
{
namespace solidity
{

class CHC: public SMTEncoder
{
public:
	CHC(smt::EncodingContext& _context, langutil::ErrorReporter& _errorReporter);

	void analyze(SourceUnit const& _sources);

	std::set<Expression const*> const& safeAssertions() const { return m_safeAssertions; }

private:
	/// Visitor functions.
	//@{
	bool visit(ContractDefinition const& _node) override;
	void endVisit(ContractDefinition const& _node) override;
	bool visit(FunctionDefinition const& _node) override;
	void endVisit(FunctionDefinition const& _node) override;
	bool visit(IfStatement const& _node) override;
	void endVisit(FunctionCall const& _node) override;

	void visitAssert(FunctionCall const& _funCall);
	//@}

	/// Helpers.
	//@{
	void reset();
	bool shouldVisit(ContractDefinition const& _contract) const;
	bool shouldVisit(FunctionDefinition const& _function) const;
	//@}

	/// Sort helpers.
	//@{
	smt::SortPointer constructorSort();
	smt::SortPointer interfaceSort();
	smt::SortPointer functionSort(FunctionDefinition const& _function);
	//@}

	/// Predicate helpers.
	//@{
	/// @returns a new block of given _sort and _name.
	std::unique_ptr<smt::SymbolicFunctionVariable> createBlock(smt::SortPointer _sort, std::string const& _name);

	/// Constructor predicate over current variables.
	smt::Expression constructor();
	/// Interface predicate over current variables.
	smt::Expression interface();
	/// Error predicate over current variables.
	smt::Expression error();
	//@}

	/// Solver related.
	//@{
	/// @returns true if query is unsatisfiable (safe).
	bool query(smt::Expression const& _query, langutil::SourceLocation const& _location);
	//@}

	/// Predicates.
	//@{
	/// Constructor predicate.
	/// Default constructor sets state vars to 0.
	std::unique_ptr<smt::SymbolicVariable> m_constructorPredicate;

	/// Artificial Interface predicate.
	/// Single entry block for all functions.
	std::unique_ptr<smt::SymbolicVariable> m_interfacePredicate;

	/// Artificial Error predicate.
	/// Single error block for all assertions.
	std::unique_ptr<smt::SymbolicVariable> m_errorPredicate;
	//@}

	/// Variables.
	//@{
	/// State variables sorts.
	/// Used by all predicates.
	std::vector<smt::SortPointer> m_stateSorts;
	/// State variables.
	/// Used to create all predicates.
	std::vector<VariableDeclaration const*> m_stateVariables;
	//@}

	/// Verification targets.
	//@{
	std::vector<Expression const*> m_verificationTargets;

	/// Assertions proven safe.
	std::set<Expression const*> m_safeAssertions;
	//@}

	/// Control-flow.
	//@{
	FunctionDefinition const* m_currentFunction = nullptr;
	//@}

	/// CHC solver.
	std::shared_ptr<smt::CHCSolverInterface> m_interface;

	/// ErrorReporter that comes from CompilerStack.
	langutil::ErrorReporter& m_outerErrorReporter;
};

}
}
