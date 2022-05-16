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

#include <libsolidity/formal/ModelCheckerSettings.h>
#include <libsolidity/formal/Predicate.h>
#include <libsolidity/formal/SMTEncoder.h>

#include <libsolidity/interface/ReadFile.h>

#include <libsmtutil/CHCSolverInterface.h>

#include <liblangutil/SourceLocation.h>
#include <liblangutil/UniqueErrorReporter.h>

#include <boost/algorithm/string/join.hpp>

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
		langutil::UniqueErrorReporter& _errorReporter,
		std::map<util::h256, std::string> const& _smtlib2Responses,
		ReadCallback::Callback const& _smtCallback,
		ModelCheckerSettings const& _settings,
		langutil::CharStreamProvider const& _charStreamProvider
	);

	void analyze(SourceUnit const& _sources);

	struct ReportTargetInfo
	{
		langutil::ErrorId error;
		langutil::SourceLocation location;
		std::string message;
	};
	std::map<ASTNode const*, std::set<VerificationTargetType>, smt::EncodingContext::IdCompare> const& safeTargets() const { return m_safeTargets; }
	std::map<ASTNode const*, std::map<VerificationTargetType, ReportTargetInfo>, smt::EncodingContext::IdCompare> const& unsafeTargets() const { return m_unsafeTargets; }

	/// This is used if the Horn solver is not directly linked into this binary.
	/// @returns a list of inputs to the Horn solver that were not part of the argument to
	/// the constructor.
	std::vector<std::string> unhandledQueries() const;

	enum class CHCNatspecOption
	{
		AbstractFunctionNondet
	};

private:
	/// Visitor functions.
	//@{
	bool visit(ContractDefinition const& _node) override;
	void endVisit(ContractDefinition const& _node) override;
	bool visit(FunctionDefinition const& _node) override;
	void endVisit(FunctionDefinition const& _node) override;
	bool visit(Block const& _block) override;
	void endVisit(Block const& _block) override;
	bool visit(IfStatement const& _node) override;
	bool visit(WhileStatement const&) override;
	bool visit(ForStatement const&) override;
	void endVisit(ForStatement const&) override;
	void endVisit(FunctionCall const& _node) override;
	void endVisit(Break const& _node) override;
	void endVisit(Continue const& _node) override;
	void endVisit(IndexRangeAccess const& _node) override;
	void endVisit(Return const& _node) override;
	bool visit(TryCatchClause const&) override;
	void endVisit(TryCatchClause const&) override;
	bool visit(TryStatement const& _node) override;

	void pushInlineFrame(CallableDeclaration const& _callable) override;
	void popInlineFrame(CallableDeclaration const& _callable) override;

	void visitAssert(FunctionCall const& _funCall);
	void visitAddMulMod(FunctionCall const& _funCall) override;
	void internalFunctionCall(FunctionCall const& _funCall);
	void externalFunctionCall(FunctionCall const& _funCall);
	void externalFunctionCallToTrustedCode(FunctionCall const& _funCall);
	void unknownFunctionCall(FunctionCall const& _funCall);
	void makeArrayPopVerificationTarget(FunctionCall const& _arrayPop) override;
	void makeOutOfBoundsVerificationTarget(IndexAccess const& _access) override;
	/// Creates underflow/overflow verification targets.
	std::pair<smtutil::Expression, smtutil::Expression> arithmeticOperation(
		Token _op,
		smtutil::Expression const& _left,
		smtutil::Expression const& _right,
		Type const* _commonType,
		Expression const& _expression
	) override;
	//@}

	/// Helpers.
	//@{
	void resetSourceAnalysis();
	void resetContractAnalysis();
	void eraseKnowledge();
	void clearIndices(ContractDefinition const* _contract, FunctionDefinition const* _function = nullptr) override;
	void setCurrentBlock(Predicate const& _block);
	std::set<unsigned> transactionVerificationTargetsIds(ASTNode const* _txRoot);
	//@}

	/// SMT Natspec and abstraction helpers.
	//@{
	/// @returns a CHCNatspecOption enum if _option is a valid SMTChecker Natspec value
	/// or nullopt otherwise.
	static std::optional<CHCNatspecOption> natspecOptionFromString(std::string const& _option);
	/// @returns which SMTChecker options are enabled by @a _function's Natspec via
	/// `@custom:smtchecker <option>` or nullopt if none is used.
	std::set<CHCNatspecOption> smtNatspecTags(FunctionDefinition const& _function);
	/// @returns true if _function is Natspec annotated to be abstracted by
	/// nondeterministic values.
	bool abstractAsNondet(FunctionDefinition const& _function);
	//@}

	/// Sort helpers.
	//@{
	smtutil::SortPointer sort(FunctionDefinition const& _function);
	smtutil::SortPointer sort(ASTNode const* _block);
	//@}

	/// Predicate helpers.
	//@{
	/// @returns a new block of given _sort and _name.
	Predicate const* createSymbolicBlock(smtutil::SortPointer _sort, std::string const& _name, PredicateType _predType, ASTNode const* _node = nullptr, ContractDefinition const* _contractContext = nullptr);

	/// Creates summary predicates for all functions of all contracts
	/// in a given _source.
	void defineInterfacesAndSummaries(SourceUnit const& _source);

	/// Creates the rule
	/// summary_function \land transaction_entry_constraints => external_summary_function
	/// This is needed to add these transaction entry constraints which include
	/// potential balance increase by external means, for example.
	void defineExternalFunctionInterface(FunctionDefinition const& _function, ContractDefinition const& _contract);

	/// Creates a CHC system that, for a given contract,
	/// - initializes its state variables (as 0 or given value, if any).
	/// - "calls" the explicit constructor function of the contract, if any.
	void defineContractInitializer(ContractDefinition const& _contract, ContractDefinition const& _contractContext);

	/// Interface predicate over current variables.
	smtutil::Expression interface();
	smtutil::Expression interface(ContractDefinition const& _contract);
	/// Error predicate over current variables.
	smtutil::Expression error();
	smtutil::Expression error(unsigned _idx);

	/// Creates a block for the given _node.
	Predicate const* createBlock(ASTNode const* _node, PredicateType _predType, std::string const& _prefix = "");
	/// Creates a call block for the given function _function from contract _contract.
	/// The contract is needed here because of inheritance.
	/// There are different types of summaries, where the most common is FunctionSummary,
	/// but other summaries are also used for internal and external function calls.
	Predicate const* createSummaryBlock(
		FunctionDefinition const& _function,
		ContractDefinition const& _contract,
		PredicateType _type = PredicateType::FunctionSummary
	);

	/// @returns a block related to @a _contract's constructor.
	Predicate const* createConstructorBlock(ContractDefinition const& _contract, std::string const& _prefix);

	/// Creates a new error block to be used by an assertion.
	/// Also registers the predicate.
	void createErrorBlock();

	void connectBlocks(smtutil::Expression const& _from, smtutil::Expression const& _to, smtutil::Expression const& _constraints = smtutil::Expression(true));

	/// @returns The initial constraints that set up the beginning of a function.
	smtutil::Expression initialConstraints(ContractDefinition const& _contract, FunctionDefinition const* _function = nullptr);

	/// @returns the symbolic values of the state variables at the beginning
	/// of the current transaction.
	std::vector<smtutil::Expression> initialStateVariables();
	std::vector<smtutil::Expression> stateVariablesAtIndex(unsigned _index);
	std::vector<smtutil::Expression> stateVariablesAtIndex(unsigned _index, ContractDefinition const& _contract);
	/// @returns the current symbolic values of the current state variables.
	std::vector<smtutil::Expression> currentStateVariables();
	std::vector<smtutil::Expression> currentStateVariables(ContractDefinition const& _contract);

	/// @returns \bigwedge currentValue(_vars[i]) == initialState(_var[i])
	smtutil::Expression currentEqualInitialVarsConstraints(std::vector<VariableDeclaration const*> const& _vars) const;

	/// @returns the predicate name for a given node.
	std::string predicateName(ASTNode const* _node, ContractDefinition const* _contract = nullptr);
	/// @returns a predicate application after checking the predicate's type.
	smtutil::Expression predicate(Predicate const& _block);
	/// @returns the summary predicate for the called function.
	smtutil::Expression predicate(FunctionCall const& _funCall);
	/// @returns a predicate that defines a contract initializer for _contract in the context of _contractContext.
	smtutil::Expression initializer(ContractDefinition const& _contract, ContractDefinition const& _contractContext);
	/// @returns a predicate that defines a constructor summary.
	smtutil::Expression summary(ContractDefinition const& _contract);
	/// @returns a predicate that defines a function summary.
	smtutil::Expression summary(FunctionDefinition const& _function);
	smtutil::Expression summary(FunctionDefinition const& _function, ContractDefinition const& _contract);
	/// @returns a predicate that applies a function summary
	/// over the constrained variables.
	smtutil::Expression summaryCall(FunctionDefinition const& _function);
	smtutil::Expression summaryCall(FunctionDefinition const& _function, ContractDefinition const& _contract);
	/// @returns a predicate that defines an external function summary.
	smtutil::Expression externalSummary(FunctionDefinition const& _function);
	smtutil::Expression externalSummary(FunctionDefinition const& _function, ContractDefinition const& _contract);
	//@}

	/// Solver related.
	//@{
	/// Adds Horn rule to the solver.
	void addRule(smtutil::Expression const& _rule, std::string const& _ruleName);
	/// @returns <true, invariant, empty> if query is unsatisfiable (safe).
	/// @returns <false, Expression(true), model> otherwise.
	std::tuple<smtutil::CheckResult, smtutil::Expression, smtutil::CHCSolverInterface::CexGraph> query(smtutil::Expression const& _query, langutil::SourceLocation const& _location);

	void verificationTargetEncountered(ASTNode const* const _errorNode, VerificationTargetType _type, smtutil::Expression const& _errorCondition);

	void checkVerificationTargets();
	// Forward declarations. Definitions are below.
	struct CHCVerificationTarget;
	struct CHCQueryPlaceholder;
	void checkAssertTarget(ASTNode const* _scope, CHCVerificationTarget const& _target);
	void checkAndReportTarget(
		CHCVerificationTarget const& _target,
		std::vector<CHCQueryPlaceholder> const& _placeholders,
		langutil::ErrorId _errorReporterId,
		std::string _satMsg,
		std::string _unknownMsg = ""
	);

	std::optional<std::string> generateCounterexample(smtutil::CHCSolverInterface::CexGraph const& _graph, std::string const& _root);

	/// @returns a call graph for function summaries in the counterexample graph.
	/// The returned map also contains a key _root, whose value are the
	/// summaries called by _root.
	std::map<unsigned, std::vector<unsigned>> summaryCalls(
		smtutil::CHCSolverInterface::CexGraph const& _graph,
		unsigned _root
	);

	/// @returns a set of pairs _var = _value separated by _separator.
	template <typename T>
	std::string formatVariableModel(std::vector<T> const& _variables, std::vector<std::optional<std::string>> const& _values, std::string const& _separator) const
	{
		solAssert(_variables.size() == _values.size(), "");

		std::vector<std::string> assignments;
		for (unsigned i = 0; i < _values.size(); ++i)
		{
			auto var = _variables.at(i);
			if (var && _values.at(i))
				assignments.emplace_back(var->name() + " = " + *_values.at(i));
		}

		return boost::algorithm::join(assignments, _separator);
	}

	/// @returns a DAG in the dot format.
	/// Used for debugging purposes.
	std::string cex2dot(smtutil::CHCSolverInterface::CexGraph const& _graph);
	//@}

	/// Misc.
	//@{
	/// @returns a prefix to be used in a new unique block name
	/// and increases the block counter.
	std::string uniquePrefix();

	/// @returns a suffix to be used by contract related predicates.
	std::string contractSuffix(ContractDefinition const& _contract);

	/// @returns a new unique error id associated with _expr and stores
	/// it into m_errorIds.
	unsigned newErrorId();

	smt::SymbolicIntVariable& errorFlag();
	//@}

	/// Predicates.
	//@{
	/// Artificial Interface predicate.
	/// Single entry block for all functions.
	std::map<ContractDefinition const*, Predicate const*> m_interfaces;

	/// Nondeterministic interfaces.
	/// These are used when the analyzed contract makes external calls to unknown code,
	/// which means that the analyzed contract can potentially be called
	/// nondeterministically.
	std::map<ContractDefinition const*, Predicate const*> m_nondetInterfaces;

	std::map<ContractDefinition const*, Predicate const*> m_constructorSummaries;
	std::map<ContractDefinition const*, std::map<ContractDefinition const*, Predicate const*>> m_contractInitializers;

	/// Artificial Error predicate.
	/// Single error block for all assertions.
	Predicate const* m_errorPredicate = nullptr;

	/// Function predicates.
	std::map<ContractDefinition const*, std::map<FunctionDefinition const*, Predicate const*>> m_summaries;

	/// External function predicates.
	std::map<ContractDefinition const*, std::map<FunctionDefinition const*, Predicate const*>> m_externalSummaries;
	//@}

	/// Variables.
	//@{
	/// State variables.
	/// Used to create all predicates.
	std::vector<VariableDeclaration const*> m_stateVariables;
	//@}

	/// Verification targets.
	//@{
	struct CHCVerificationTarget: VerificationTarget
	{
		unsigned const errorId;
		ASTNode const* const errorNode;
	};

	/// Query placeholder stores information necessary to create the final query edge in the CHC system.
	/// It is combined with the unique error id (and error type) to create a complete Verification Target.
	struct CHCQueryPlaceholder
	{
		smtutil::Expression const constraints;
		smtutil::Expression const errorExpression;
		smtutil::Expression const fromPredicate;
	};

	/// Query placeholders for constructors, if the key has type ContractDefinition*,
	/// or external functions, if the key has type FunctionDefinition*.
	/// A placeholder is created for each possible context of a function (e.g. multiple contracts in contract inheritance hierarchy).
	std::map<ASTNode const*, std::vector<CHCQueryPlaceholder>, smt::EncodingContext::IdCompare> m_queryPlaceholders;

	/// Records verification conditions IDs per function encountered during an analysis of that function.
	/// The key is the ASTNode of the function where the verification condition has been encountered,
	/// or the ASTNode of the contract if the verification condition happens inside an implicit constructor.
	std::map<ASTNode const*, std::vector<unsigned>, smt::EncodingContext::IdCompare> m_functionTargetIds;
	/// Helper mapping unique IDs to actual verification targets.
	std::map<unsigned, CHCVerificationTarget> m_verificationTargets;

	/// Targets proved safe.
	std::map<ASTNode const*, std::set<VerificationTargetType>, smt::EncodingContext::IdCompare> m_safeTargets;
	/// Targets proved unsafe.
	std::map<ASTNode const*, std::map<VerificationTargetType, ReportTargetInfo>, smt::EncodingContext::IdCompare> m_unsafeTargets;
	/// Targets not proved.
	std::map<ASTNode const*, std::map<VerificationTargetType, ReportTargetInfo>, smt::EncodingContext::IdCompare> m_unprovedTargets;

	/// Inferred invariants.
	std::map<Predicate const*, std::set<std::string>, PredicateCompare> m_invariants;
	//@}

	/// Control-flow.
	//@{
	FunctionDefinition const* m_currentFunction = nullptr;

	std::map<ASTNode const*, std::set<ASTNode const*, smt::EncodingContext::IdCompare>, smt::EncodingContext::IdCompare> m_callGraph;

	/// The current block.
	smtutil::Expression m_currentBlock = smtutil::Expression(true);

	/// Counter to generate unique block names.
	unsigned m_blockCounter = 0;

	/// Whether a function call was seen in the current scope.
	bool m_unknownFunctionCallSeen = false;

	/// Block where a loop break should go to.
	Predicate const* m_breakDest = nullptr;
	/// Block where a loop continue should go to.
	Predicate const* m_continueDest = nullptr;

	/// Block where an error condition should go to.
	/// This can be:
	/// 1) Constructor initializer summary, if error happens while evaluating initial values of state variables.
	/// 2) Constructor summary, if error happens while evaluating base constructor arguments.
	/// 3) Function summary, if error happens inside a function.
	Predicate const* m_errorDest = nullptr;

	/// Represents the stack of destinations where a `return` should go.
	/// This is different from `m_errorDest` above:
	/// - Constructor initializers and constructor summaries will never be `return` targets because they are artificial.
	/// - Modifiers also have their own `return` target blocks, whereas they do not have their own error destination.
	std::vector<Predicate const*> m_returnDests;
	//@}

	/// CHC solver.
	std::unique_ptr<smtutil::CHCSolverInterface> m_interface;

	std::map<util::h256, std::string> const& m_smtlib2Responses;
	ReadCallback::Callback const& m_smtCallback;
};

}
