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
	bool visit(WhileStatement const&) override;
	bool visit(ForStatement const&) override;
	void endVisit(FunctionCall const& _node) override;
	void endVisit(Break const& _node) override;
	void endVisit(Continue const& _node) override;

	void visitAssert(FunctionCall const& _funCall);
	void unknownFunctionCall(FunctionCall const& _funCall);
	void visitLoop(
		BreakableStatement const& _loop,
		Expression const* _condition,
		Statement const& _body,
		ASTNode const* _postLoop
	);
	//@}

	/// Helpers.
	//@{
	void reset();
	void eraseKnowledge();
	bool shouldVisit(ContractDefinition const& _contract) const;
	bool shouldVisit(FunctionDefinition const& _function) const;
	void pushBlock(ASTNode const* _node);
	void popBlock();
	//@}

	/// Sort helpers.
	//@{
	smt::SortPointer constructorSort();
	smt::SortPointer interfaceSort();
	smt::SortPointer sort(FunctionDefinition const& _function);
	smt::SortPointer sort(ASTNode const* _block);
	//@}

	/// Predicate helpers.
	//@{
	/// @returns a new block of given _sort and _name.
	std::unique_ptr<smt::SymbolicFunctionVariable> createSymbolicBlock(smt::SortPointer _sort, std::string const& _name);

	/// Constructor predicate over current variables.
	smt::Expression constructor();
	/// Interface predicate over current variables.
	smt::Expression interface();
	/// Error predicate over current variables.
	smt::Expression error();
	smt::Expression error(unsigned _idx);

	/// Creates a block for the given _node or increases its SSA index
	/// if the block already exists which in practice creates a new function.
	void createBlock(ASTNode const* _node, std::string const& _prefix = "");

	/// Creates a new error block to be used by an assertion.
	/// Also registers the predicate.
	void createErrorBlock();

	void connectBlocks(smt::Expression const& _from, smt::Expression const& _to, smt::Expression const& _constraints = smt::Expression(true));

	/// @returns the current symbolic values of the current function's
	/// input and output parameters.
	std::vector<smt::Expression> currentFunctionVariables();
	/// @returns the samve as currentFunctionVariables plus
	/// local variables.
	std::vector<smt::Expression> currentBlockVariables();

	/// Sets the SSA indices of the variables in scope to 0.
	/// Used when starting a new block.
	void clearIndices();

	/// @returns the predicate name for a given node.
	std::string predicateName(ASTNode const* _node);
	/// @returns a predicate application over the current scoped variables.
	smt::Expression predicate(ASTNode const* _node);
	//@}

	/// Solver related.
	//@{
	/// Adds Horn rule to the solver.
	void addRule(smt::Expression const& _rule, std::string const& _ruleName);
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

	/// Maps AST nodes to their predicates.
	std::unordered_map<ASTNode const*, std::shared_ptr<smt::SymbolicVariable>> m_predicates;
	//@}

	/// Variables.
	//@{
	/// State variables sorts.
	/// Used by all predicates.
	std::vector<smt::SortPointer> m_stateSorts;
	/// State variables.
	/// Used to create all predicates.
	std::vector<VariableDeclaration const*> m_stateVariables;

	/// Input sorts for AST nodes.
	std::map<ASTNode const*, smt::SortPointer> m_nodeSorts;
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

	/// Number of basic blocks created for the body of the current function.
	unsigned m_functionBlocks = 0;
	/// The current control flow path.
	std::vector<smt::Expression> m_path;
	/// Whether a function call was seen in the current scope.
	bool m_unknownFunctionCallSeen = false;
	/// Whether a break statement was seen in the current scope.
	bool m_breakSeen = false;
	/// Whether a continue statement was seen in the current scope.
	bool m_continueSeen = false;

	/// Block where a loop break should go to.
	ASTNode const* m_breakDest;
	/// Block where a loop continue should go to.
	ASTNode const* m_continueDest;
	//@}

	/// CHC solver.
	std::shared_ptr<smt::CHCSolverInterface> m_interface;

	/// ErrorReporter that comes from CompilerStack.
	langutil::ErrorReporter& m_outerErrorReporter;
};

}
}
