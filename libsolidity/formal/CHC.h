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
// SPDX-License-Identifier: GPL-3.0

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

#include <libsolidity/formal/Predicate.h>
#include <libsolidity/formal/SMTEncoder.h>

#include <libsolidity/interface/ReadFile.h>

#include <libsmtutil/CHCSolverInterface.h>

#include <map>
#include <optional>
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

	std::map<ASTNode const*, std::set<VerificationTarget::Type>> const& safeTargets() const { return m_safeTargets; }
	std::map<ASTNode const*, std::set<VerificationTarget::Type>> const& unsafeTargets() const { return m_unsafeTargets; }

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
	void externalFunctionCall(FunctionCall const& _funCall);
	void unknownFunctionCall(FunctionCall const& _funCall);
	void makeArrayPopVerificationTarget(FunctionCall const& _arrayPop) override;
	/// Creates underflow/overflow verification targets.
	std::pair<smtutil::Expression, smtutil::Expression> arithmeticOperation(
		Token _op,
		smtutil::Expression const& _left,
		smtutil::Expression const& _right,
		TypePointer const& _commonType,
		Expression const& _expression
	) override;
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
	void setCurrentBlock(Predicate const& _block, std::vector<smtutil::Expression> const* _arguments = nullptr);
	std::set<Expression const*, IdCompare> transactionAssertions(ASTNode const* _txRoot);
	//@}

	/// Sort helpers.
	//@{
	static std::vector<smtutil::SortPointer> stateSorts(ContractDefinition const& _contract);
	smtutil::SortPointer constructorSort();
	smtutil::SortPointer interfaceSort();
	smtutil::SortPointer nondetInterfaceSort();
	static smtutil::SortPointer interfaceSort(ContractDefinition const& _const);
	static smtutil::SortPointer nondetInterfaceSort(ContractDefinition const& _const);
	smtutil::SortPointer arity0FunctionSort() const;
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
	Predicate const* createSymbolicBlock(smtutil::SortPointer _sort, std::string const& _name, ASTNode const* _node = nullptr);

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
	Predicate const* createBlock(ASTNode const* _node, std::string const& _prefix = "");
	/// Creates a call block for the given function _function from contract _contract.
	/// The contract is needed here because of inheritance.
	Predicate const* createSummaryBlock(FunctionDefinition const& _function, ContractDefinition const& _contract);

	/// Creates a new error block to be used by an assertion.
	/// Also registers the predicate.
	void createErrorBlock();

	void connectBlocks(smtutil::Expression const& _from, smtutil::Expression const& _to, smtutil::Expression const& _constraints = smtutil::Expression(true));

	/// @returns the symbolic values of the state variables at the beginning
	/// of the current transaction.
	std::vector<smtutil::Expression> initialStateVariables();
	std::vector<smtutil::Expression> initialStateVariables(ContractDefinition const& _contract);
	std::vector<smtutil::Expression> stateVariablesAtIndex(unsigned _index);
	std::vector<smtutil::Expression> stateVariablesAtIndex(unsigned _index, ContractDefinition const& _contract);
	/// @returns the current symbolic values of the current state variables.
	std::vector<smtutil::Expression> currentStateVariables();
	std::vector<smtutil::Expression> currentStateVariables(ContractDefinition const& _contract);

	/// @returns the current symbolic values of the current function's
	/// input and output parameters.
	std::vector<smtutil::Expression> currentFunctionVariables();
	std::vector<smtutil::Expression> currentFunctionVariables(FunctionDefinition const& _function);
	std::vector<smtutil::Expression> currentFunctionVariables(ContractDefinition const& _contract);

	/// @returns the same as currentFunctionVariables plus
	/// local variables.
	std::vector<smtutil::Expression> currentBlockVariables();

	/// @returns the predicate name for a given node.
	std::string predicateName(ASTNode const* _node, ContractDefinition const* _contract = nullptr);
	/// @returns a predicate application over the current scoped variables.
	smtutil::Expression predicate(Predicate const& _block);
	/// @returns a predicate application over @param _arguments.
	smtutil::Expression predicate(Predicate const& _block, std::vector<smtutil::Expression> const& _arguments);
	/// @returns the summary predicate for the called function.
	smtutil::Expression predicate(FunctionCall const& _funCall);
	/// @returns a predicate that defines a constructor summary.
	smtutil::Expression summary(ContractDefinition const& _contract);
	/// @returns a predicate that defines a function summary.
	smtutil::Expression summary(FunctionDefinition const& _function);
	smtutil::Expression summary(FunctionDefinition const& _function, ContractDefinition const& _contract);
	//@}

	/// Solver related.
	//@{
	/// Adds Horn rule to the solver.
	void addRule(smtutil::Expression const& _rule, std::string const& _ruleName);
	/// @returns <true, empty> if query is unsatisfiable (safe).
	/// @returns <false, model> otherwise.
	std::pair<smtutil::CheckResult, smtutil::CHCSolverInterface::CexGraph> query(smtutil::Expression const& _query, langutil::SourceLocation const& _location);

	void addVerificationTarget(ASTNode const* _scope, VerificationTarget::Type _type, smtutil::Expression _from, smtutil::Expression _constraints, smtutil::Expression _errorId);
	void addVerificationTarget(ASTNode const* _scope, VerificationTarget::Type _type, smtutil::Expression _errorId);
	void addAssertVerificationTarget(ASTNode const* _scope, smtutil::Expression _from, smtutil::Expression _constraints, smtutil::Expression _errorId);

	void checkVerificationTargets();
	// Forward declaration. Definition is below.
	struct CHCVerificationTarget;
	void checkAssertTarget(ASTNode const* _scope, CHCVerificationTarget const& _target);
	void checkAndReportTarget(
		ASTNode const* _scope,
		CHCVerificationTarget const& _target,
		unsigned _errorId,
		langutil::ErrorId _errorReporterId,
		std::string _satMsg,
		std::string _unknownMsg = ""
	);

	std::optional<std::string> generateCounterexample(smtutil::CHCSolverInterface::CexGraph const& _graph, std::string const& _root);
	/// @returns values for the _stateVariables after a transaction calling
	/// _function was executed.
	/// _function = nullptr means the transaction was the deployment of a
	/// contract without an explicit constructor.
	std::string formatStateCounterexample(std::vector<VariableDeclaration const*> const& _stateVariables, FunctionDefinition const* _function, std::vector<std::string> const& _summaryValues);
	/// @returns a formatted text representing a call to _function
	/// with the concrete values for value type parameters and
	/// the parameter name for reference types.
	std::string formatFunctionCallCounterexample(std::vector<VariableDeclaration const*> const& _stateVariables, FunctionDefinition const& _function, std::vector<std::string> const& _summaryValues);

	/// @returns a DAG in the dot format.
	/// Used for debugging purposes.
	std::string cex2dot(smtutil::CHCSolverInterface::CexGraph const& _graph);
	//@}

	/// Misc.
	//@{
	/// Returns a prefix to be used in a new unique block name
	/// and increases the block counter.
	std::string uniquePrefix();

	/// @returns a new unique error id associated with _expr and stores
	/// it into m_errorIds.
	unsigned newErrorId(Expression const& _expr);
	//@}

	/// Predicates.
	//@{
	/// Genesis predicate.
	Predicate const* m_genesisPredicate = nullptr;

	/// Implicit constructor predicate.
	/// Explicit constructors are handled as functions.
	Predicate const* m_implicitConstructorPredicate = nullptr;

	/// Constructor summary predicate, exists after the constructor
	/// (implicit or explicit) and before the interface.
	Predicate const* m_constructorSummaryPredicate = nullptr;

	/// Artificial Interface predicate.
	/// Single entry block for all functions.
	std::map<ContractDefinition const*, Predicate const*> m_interfaces;

	/// Nondeterministic interfaces.
	/// These are used when the analyzed contract makes external calls to unknown code,
	/// which means that the analyzed contract can potentially be called
	/// nondeterministically.
	std::map<ContractDefinition const*, Predicate const*> m_nondetInterfaces;

	/// Artificial Error predicate.
	/// Single error block for all assertions.
	Predicate const* m_errorPredicate = nullptr;

	/// Function predicates.
	std::map<ContractDefinition const*, std::map<FunctionDefinition const*, Predicate const*>> m_summaries;

	smt::SymbolicIntVariable m_error{
		TypeProvider::uint256(),
		TypeProvider::uint256(),
		"error",
		m_context
	};

	/// Maps predicate names to the ASTNodes they came from.
	std::map<std::string, ASTNode const*> m_symbolFunction;
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

	/// Targets proven safe.
	std::map<ASTNode const*, std::set<VerificationTarget::Type>> m_safeTargets;
	/// Targets proven unsafe.
	std::map<ASTNode const*, std::set<VerificationTarget::Type>> m_unsafeTargets;
	//@}

	/// Control-flow.
	//@{
	FunctionDefinition const* m_currentFunction = nullptr;

	std::map<ASTNode const*, std::set<ASTNode const*, IdCompare>, IdCompare> m_callGraph;

	std::map<ASTNode const*, std::set<Expression const*>, IdCompare> m_functionAssertions;

	/// Maps ASTNode ids to error ids.
	/// A multimap is used instead of map anticipating the UnderOverflow
	/// target which has 2 error ids.
	std::multimap<unsigned, unsigned> m_errorIds;

	/// The current block.
	smtutil::Expression m_currentBlock = smtutil::Expression(true);

	/// Counter to generate unique block names.
	unsigned m_blockCounter = 0;

	/// Whether a function call was seen in the current scope.
	bool m_unknownFunctionCallSeen = false;

	/// Block where a loop break should go to.
	Predicate const* m_breakDest;
	/// Block where a loop continue should go to.
	Predicate const* m_continueDest;
	//@}

	/// CHC solver.
	std::unique_ptr<smtutil::CHCSolverInterface> m_interface;

	/// ErrorReporter that comes from CompilerStack.
	langutil::ErrorReporter& m_outerErrorReporter;

	/// SMT solvers that are chosen at runtime.
	smtutil::SMTSolverChoice m_enabledSolvers;
};

}
