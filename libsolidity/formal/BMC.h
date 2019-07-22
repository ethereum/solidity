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
 * Class that implements an SMT-based Bounded Model Checker (BMC).
 * Traverses the AST such that:
 * - Loops are unrolled
 * - Internal function calls are inlined
 * Creates verification targets for:
 * - Underflow/Overflow
 * - Constant conditions
 * - Assertions
 */

#pragma once


#include <libsolidity/formal/EncodingContext.h>
#include <libsolidity/formal/SMTEncoder.h>
#include <libsolidity/formal/SolverInterface.h>

#include <libsolidity/interface/ReadFile.h>
#include <liblangutil/ErrorReporter.h>
#include <liblangutil/Scanner.h>

#include <set>
#include <string>
#include <vector>

namespace langutil
{
class ErrorReporter;
struct SourceLocation;
}

namespace dev
{
namespace solidity
{

class BMC: public SMTEncoder
{
public:
	BMC(smt::EncodingContext& _context, langutil::ErrorReporter& _errorReporter, std::map<h256, std::string> const& _smtlib2Responses);

	void analyze(SourceUnit const& _sources, std::shared_ptr<langutil::Scanner> const& _scanner, std::set<Expression const*> _safeAssertions);

	/// This is used if the SMT solver is not directly linked into this binary.
	/// @returns a list of inputs to the SMT solver that were not part of the argument to
	/// the constructor.
	std::vector<std::string> unhandledQueries() { return m_interface->unhandledQueries(); }

	/// @returns true if _funCall should be inlined, otherwise false.
	static bool shouldInlineFunctionCall(FunctionCall const& _funCall);

	std::shared_ptr<smt::SolverInterface> solver() { return m_interface; }

private:
	/// AST visitors.
	/// Only nodes that lead to verification targets being built
	/// or checked are visited.
	//@{
	bool visit(ContractDefinition const& _node) override;
	bool visit(FunctionDefinition const& _node) override;
	void endVisit(FunctionDefinition const& _node) override;
	bool visit(IfStatement const& _node) override;
	bool visit(WhileStatement const& _node) override;
	bool visit(ForStatement const& _node) override;
	void endVisit(UnaryOperation const& _node) override;
	void endVisit(FunctionCall const& _node) override;
	//@}

	/// Visitor helpers.
	//@{
	void visitAssert(FunctionCall const& _funCall);
	void visitRequire(FunctionCall const& _funCall);
	/// Visits the FunctionDefinition of the called function
	/// if available and inlines the return value.
	void inlineFunctionCall(FunctionCall const& _funCall);
	/// Creates an uninterpreted function call.
	void abstractFunctionCall(FunctionCall const& _funCall);
	/// Inlines if the function call is internal or external to `this`.
	/// Erases knowledge about state variables if external.
	void internalOrExternalFunctionCall(FunctionCall const& _funCall);

	/// Creates underflow/overflow verification targets.
	std::pair<smt::Expression, smt::Expression> arithmeticOperation(
		Token _op,
		smt::Expression const& _left,
		smt::Expression const& _right,
		TypePointer const& _commonType,
		Expression const& _expression
	) override;

	void resetStorageReferences();
	void reset();

	std::pair<std::vector<smt::Expression>, std::vector<std::string>> modelExpressions();
	//@}

	/// Verification targets.
	//@{
	struct VerificationTarget
	{
		enum class Type { ConstantCondition, Underflow, Overflow, UnderOverflow, DivByZero, Balance, Assert } type;
		smt::Expression value;
		smt::Expression constraints;
		Expression const* expression;
		std::vector<CallStackEntry> callStack;
		std::pair<std::vector<smt::Expression>, std::vector<std::string>> modelExpressions;
	};

	void checkVerificationTargets(smt::Expression const& _constraints);
	void checkVerificationTarget(VerificationTarget& _target, smt::Expression const& _constraints = smt::Expression(true));
	void checkConstantCondition(VerificationTarget& _target);
	void checkUnderflow(VerificationTarget& _target, smt::Expression const& _constraints);
	void checkOverflow(VerificationTarget& _target, smt::Expression const& _constraints);
	void checkDivByZero(VerificationTarget& _target);
	void checkBalance(VerificationTarget& _target);
	void checkAssert(VerificationTarget& _target);
	void addVerificationTarget(
		VerificationTarget::Type _type,
		smt::Expression const& _value,
		Expression const* _expression
	);
	//@}

	/// Solver related.
	//@{
	/// Check that a condition can be satisfied.
	void checkCondition(
		smt::Expression _condition,
		std::vector<CallStackEntry> const& callStack,
		std::pair<std::vector<smt::Expression>, std::vector<std::string>> const& _modelExpressions,
		langutil::SourceLocation const& _location,
		std::string const& _description,
		std::string const& _additionalValueName = "",
		smt::Expression const* _additionalValue = nullptr
	);
	/// Checks that a boolean condition is not constant. Do not warn if the expression
	/// is a literal constant.
	/// @param _description the warning string, $VALUE will be replaced by the constant value.
	void checkBooleanNotConstant(
		Expression const& _condition,
		smt::Expression const& _constraints,
		smt::Expression const& _value,
		std::vector<CallStackEntry> const& _callStack,
		std::string const& _description
	);
	std::pair<smt::CheckResult, std::vector<std::string>>
	checkSatisfiableAndGenerateModel(std::vector<smt::Expression> const& _expressionsToEvaluate);

	smt::CheckResult checkSatisfiable();
	//@}

	/// Flags used for better warning messages.
	bool m_loopExecutionHappened = false;
	bool m_externalFunctionCallHappened = false;

	/// ErrorReporter that comes from CompilerStack.
	langutil::ErrorReporter& m_outerErrorReporter;

	std::vector<VerificationTarget> m_verificationTargets;

	/// Assertions that are known to be safe.
	std::set<Expression const*> m_safeAssertions;

	std::shared_ptr<smt::SolverInterface> m_interface;
};

}
}
