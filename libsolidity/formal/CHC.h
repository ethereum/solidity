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

#include <libsolidity/interface/ReadFile.h>

#include <libsmtutil/CHCSolverInterface.h>

#include <set>

namespace solidity::frontend
{

class CHC: public SMTEncoder
{
public:
	CHC(
		smt::EncodingContext& _context,
		langutil::ErrorReporter& _errorReporter,
		std::map<util::h256, std::string> const& _smtlib2Responses,
		ReadCallback::Callback const& _smtCallback,
		smtutil::SMTSolverChoice _enabledSolvers
	);

	void analyze(SourceUnit const& _sources);

	std::set<Expression const*> const& safeAssertions() const { return m_safeAssertions; }

	/// This is used if the Horn solver is not directly linked into this binary.
	/// @returns a list of inputs to the Horn solver that were not part of the argument to
	/// the constructor.
	std::vector<std::string> unhandledQueries() const;

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
	void internalFunctionCall(FunctionCall const& _funCall);
	void unknownFunctionCall(FunctionCall const& _funCall);
	void makeArrayPopVerificationTarget(FunctionCall const& _arrayPop) override;
	//@}

	struct IdCompare
	{
		bool operator()(ASTNode const* lhs, ASTNode const* rhs) const
		{
			return lhs->id() < rhs->id();
		}
	};

	/// Helpers.
	//@{
	void resetSourceAnalysis();
	void resetContractAnalysis();
	void eraseKnowledge();
	void clearIndices(ContractDefinition const* _contract, FunctionDefinition const* _function = nullptr) override;
	bool shouldVisit(FunctionDefinition const& _function) const;
	void setCurrentBlock(smt::SymbolicFunctionVariable const& _block, std::vector<smtutil::Expression> const* _arguments = nullptr);
	std::set<Expression const*, IdCompare> transactionAssertions(ASTNode const* _txRoot);
	static std::vector<VariableDeclaration const*> stateVariablesIncludingInheritedAndPrivate(ContractDefinition const& _contract);
	//@}

	/// Sort helpers.
	//@{
	static std::vector<smtutil::SortPointer> stateSorts(ContractDefinition const& _contract);
	smtutil::SortPointer constructorSort();
	smtutil::SortPointer interfaceSort();
	static smtutil::SortPointer interfaceSort(ContractDefinition const& _const);
	smtutil::SortPointer arity0FunctionSort();
	smtutil::SortPointer sort(FunctionDefinition const& _function);
	smtutil::SortPointer sort(ASTNode const* _block);
	/// @returns the sort of a predicate that represents the summary of _function in the scope of _contract.
	/// The _contract is also needed because the same function might be in many contracts due to inheritance,
	/// where the sort changes because the set of state variables might change.
	static smtutil::SortPointer summarySort(FunctionDefinition const& _function, ContractDefinition const& _contract);
	//@}

	/// Predicate helpers.
	//@{
	/// @returns a new block of given _sort and _name.
	std::unique_ptr<smt::SymbolicFunctionVariable> createSymbolicBlock(smtutil::SortPointer _sort, std::string const& _name);

	/// Creates summary predicates for all functions of all contracts
	/// in a given _source.
	void defineInterfacesAndSummaries(SourceUnit const& _source);

	/// Genesis predicate.
	smtutil::Expression genesis() { return (*m_genesisPredicate)({}); }
	/// Interface predicate over current variables.
	smtutil::Expression interface();
	smtutil::Expression interface(ContractDefinition const& _contract);
	/// Error predicate over current variables.
	smtutil::Expression error();
	smtutil::Expression error(unsigned _idx);

	/// Creates a block for the given _node.
	std::unique_ptr<smt::SymbolicFunctionVariable> createBlock(ASTNode const* _node, std::string const& _prefix = "");
	/// Creates a call block for the given function _function from contract _contract.
	/// The contract is needed here because of inheritance.
	std::unique_ptr<smt::SymbolicFunctionVariable> createSummaryBlock(FunctionDefinition const& _function, ContractDefinition const& _contract);

	/// Creates a new error block to be used by an assertion.
	/// Also registers the predicate.
	void createErrorBlock();

	void connectBlocks(smtutil::Expression const& _from, smtutil::Expression const& _to, smtutil::Expression const& _constraints = smtutil::Expression(true));

	/// @returns the symbolic values of the state variables at the beginning
	/// of the current transaction.
	std::vector<smtutil::Expression> initialStateVariables();
	std::vector<smtutil::Expression> stateVariablesAtIndex(unsigned _index);
	std::vector<smtutil::Expression> stateVariablesAtIndex(unsigned _index, ContractDefinition const& _contract);
	/// @returns the current symbolic values of the current state variables.
	std::vector<smtutil::Expression> currentStateVariables();

	/// @returns the current symbolic values of the current function's
	/// input and output parameters.
	std::vector<smtutil::Expression> currentFunctionVariables();
	/// @returns the same as currentFunctionVariables plus
	/// local variables.
	std::vector<smtutil::Expression> currentBlockVariables();

	/// @returns the predicate name for a given node.
	std::string predicateName(ASTNode const* _node, ContractDefinition const* _contract = nullptr);
	/// @returns a predicate application over the current scoped variables.
	smtutil::Expression predicate(smt::SymbolicFunctionVariable const& _block);
	/// @returns a predicate application over @param _arguments.
	smtutil::Expression predicate(smt::SymbolicFunctionVariable const& _block, std::vector<smtutil::Expression> const& _arguments);
	/// @returns the summary predicate for the called function.
	smtutil::Expression predicate(FunctionCall const& _funCall);
	/// @returns a predicate that defines a constructor summary.
	smtutil::Expression summary(ContractDefinition const& _contract);
	/// @returns a predicate that defines a function summary.
	smtutil::Expression summary(FunctionDefinition const& _function);
	//@}

	/// Solver related.
	//@{
	/// Adds Horn rule to the solver.
	void addRule(smtutil::Expression const& _rule, std::string const& _ruleName);
	/// @returns <true, empty> if query is unsatisfiable (safe).
	/// @returns <false, model> otherwise.
	std::pair<smtutil::CheckResult, std::vector<std::string>> query(smtutil::Expression const& _query, langutil::SourceLocation const& _location);

	void addVerificationTarget(ASTNode const* _scope, VerificationTarget::Type _type, smtutil::Expression _from, smtutil::Expression _constraints, smtutil::Expression _errorId);
	void addAssertVerificationTarget(ASTNode const* _scope, smtutil::Expression _from, smtutil::Expression _constraints, smtutil::Expression _errorId);
	void addArrayPopVerificationTarget(ASTNode const* _scope, smtutil::Expression _errorId);
	//@}

	/// Misc.
	//@{
	/// Returns a prefix to be used in a new unique block name
	/// and increases the block counter.
	std::string uniquePrefix();
	//@}

	/// Predicates.
	//@{
	/// Genesis predicate.
	std::unique_ptr<smt::SymbolicFunctionVariable> m_genesisPredicate;

	/// Implicit constructor predicate.
	/// Explicit constructors are handled as functions.
	std::unique_ptr<smt::SymbolicFunctionVariable> m_implicitConstructorPredicate;

	/// Constructor summary predicate, exists after the constructor
	/// (implicit or explicit) and before the interface.
	std::unique_ptr<smt::SymbolicFunctionVariable> m_constructorSummaryPredicate;

	/// Artificial Interface predicate.
	/// Single entry block for all functions.
	std::map<ContractDefinition const*, std::unique_ptr<smt::SymbolicFunctionVariable>> m_interfaces;

	/// Artificial Error predicate.
	/// Single error block for all assertions.
	std::unique_ptr<smt::SymbolicFunctionVariable> m_errorPredicate;

	/// Function predicates.
	std::map<ContractDefinition const*, std::map<FunctionDefinition const*, std::unique_ptr<smt::SymbolicFunctionVariable>>> m_summaries;

	smt::SymbolicIntVariable m_error{
		TypeProvider::uint256(),
		TypeProvider::uint256(),
		"error",
		m_context
	};
	//@}

	/// Variables.
	//@{
	/// State variables sorts.
	/// Used by all predicates.
	std::vector<smtutil::SortPointer> m_stateSorts;
	/// State variables.
	/// Used to create all predicates.
	std::vector<VariableDeclaration const*> m_stateVariables;
	//@}

	/// Verification targets.
	//@{
	struct CHCVerificationTarget: VerificationTarget
	{
		smtutil::Expression errorId;
	};

	std::map<ASTNode const*, CHCVerificationTarget, IdCompare> m_verificationTargets;

	/// Assertions proven safe.
	std::set<Expression const*> m_safeAssertions;
	/// Targets proven unsafe.
	std::set<ASTNode const*> m_unsafeTargets;
	//@}

	/// Control-flow.
	//@{
	FunctionDefinition const* m_currentFunction = nullptr;

	std::map<ASTNode const*, std::set<ASTNode const*, IdCompare>, IdCompare> m_callGraph;

	std::map<ASTNode const*, std::set<Expression const*>, IdCompare> m_functionAssertions;

	/// The current block.
	smtutil::Expression m_currentBlock = smtutil::Expression(true);

	/// Counter to generate unique block names.
	unsigned m_blockCounter = 0;

	/// Whether a function call was seen in the current scope.
	bool m_unknownFunctionCallSeen = false;

	/// Block where a loop break should go to.
	smt::SymbolicFunctionVariable const* m_breakDest = nullptr;
	/// Block where a loop continue should go to.
	smt::SymbolicFunctionVariable const* m_continueDest = nullptr;
	//@}

	/// CHC solver.
	std::unique_ptr<smtutil::CHCSolverInterface> m_interface;

	/// ErrorReporter that comes from CompilerStack.
	langutil::ErrorReporter& m_outerErrorReporter;

	/// SMT solvers that are chosen at runtime.
	smtutil::SMTSolverChoice m_enabledSolvers;
};

}
