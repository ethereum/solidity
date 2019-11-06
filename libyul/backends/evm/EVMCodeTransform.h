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
 * Common code generator for translating Yul / inline assembly to EVM and EVM1.5.
 */

#pragma once

#include <libyul/backends/evm/EVMAssembly.h>

#include <libyul/backends/evm/EVMDialect.h>
#include <libyul/optimiser/ASTWalker.h>
#include <libyul/AsmDataForward.h>
#include <libyul/AsmScope.h>

#include <boost/variant.hpp>

#include <optional>
#include <stack>

namespace langutil
{
class ErrorReporter;
}

namespace yul
{
struct AsmAnalysisInfo;
class EVMAssembly;

struct StackTooDeepError: virtual YulException
{
	StackTooDeepError(YulString _variable, int _depth): variable(_variable), depth(_depth) {}
	StackTooDeepError(YulString _functionName, YulString _variable, int _depth):
		functionName(_functionName), variable(_variable), depth(_depth)
	{}
	YulString functionName;
	YulString variable;
	int depth;
};

struct CodeTransformContext
{
	std::map<Scope::Label const*, AbstractAssembly::LabelID> labelIDs;
	std::map<Scope::Function const*, AbstractAssembly::LabelID> functionEntryIDs;
	std::map<Scope::Variable const*, int> variableStackHeights;
	std::map<Scope::Variable const*, unsigned> variableReferences;

	struct JumpInfo
	{
		AbstractAssembly::LabelID label;  ///< Jump's LabelID to jump to.
		int targetStackHeight;            ///< Stack height after the jump.
	};

	struct ForLoopLabels
	{
		JumpInfo post; ///< Jump info for jumping to post branch.
		JumpInfo done; ///< Jump info for jumping to done branch.
	};

	std::stack<ForLoopLabels> forLoopStack;
	std::stack<JumpInfo> functionExitPoints;
};

/**
 * Counts the number of references to a variable. This includes actual (read) references
 * but also assignments to the variable. It does not include the declaration itself or
 * function parameters, but it does include function return parameters.
 *
 * This component can handle multiple variables of the same name.
 *
 * Can only be applied to strict assembly.
 */
class VariableReferenceCounter: public yul::ASTWalker
{
public:
	explicit VariableReferenceCounter(
		CodeTransformContext& _context,
		AsmAnalysisInfo const& _assemblyInfo
	): m_context(_context), m_info(_assemblyInfo)
	{}

public:
	void operator()(Identifier const& _identifier);
	void operator()(FunctionDefinition const&);
	void operator()(ForLoop const&);
	void operator()(Block const& _block);

private:
	void increaseRefIfFound(YulString _variableName);

	CodeTransformContext& m_context;
	AsmAnalysisInfo const& m_info;
	Scope* m_scope = nullptr;
};

class CodeTransform: public boost::static_visitor<>
{
public:
	/// Create the code transformer.
	/// @param _identifierAccess used to resolve identifiers external to the inline assembly
	/// As a side-effect of its construction, translates the Yul code and appends it to the
	/// given assembly.
	/// Throws StackTooDeepError if a variable is not accessible or if a function has too
	/// many parameters.
	CodeTransform(
		AbstractAssembly& _assembly,
		AsmAnalysisInfo& _analysisInfo,
		Block const& _block,
		EVMDialect const& _dialect,
		BuiltinContext& _builtinContext,
		bool _allowStackOpt = false,
		bool _evm15 = false,
		ExternalIdentifierAccess const& _identifierAccess = ExternalIdentifierAccess(),
		bool _useNamedLabelsForFunctions = false
	): CodeTransform(
		_assembly,
		_analysisInfo,
		_block,
		_allowStackOpt,
		_dialect,
		_builtinContext,
		_evm15,
		_identifierAccess,
		_useNamedLabelsForFunctions,
		_assembly.stackHeight(),
		nullptr
	)
	{
	}

	std::vector<StackTooDeepError> const& stackErrors() const { return m_stackErrors; }

protected:
	using Context = CodeTransformContext;

	CodeTransform(
		AbstractAssembly& _assembly,
		AsmAnalysisInfo& _analysisInfo,
		Block const& _block,
		bool _allowStackOpt,
		EVMDialect const& _dialect,
		BuiltinContext& _builtinContext,
		bool _evm15,
		ExternalIdentifierAccess const& _identifierAccess,
		bool _useNamedLabelsForFunctions,
		int _stackAdjustment,
		std::shared_ptr<Context> _context
	);

	void decreaseReference(YulString _name, Scope::Variable const& _var);
	bool unreferenced(Scope::Variable const& _var) const;
	/// Marks slots of variables that are not used anymore
	/// and were defined in the current scope for reuse.
	/// Also POPs unused topmost stack slots.
	void freeUnusedVariables();
	/// Marks the stack slot of @a _var to be reused.
	void deleteVariable(Scope::Variable const& _var);

public:
	void operator()(Instruction const& _instruction);
	void operator()(Literal const& _literal);
	void operator()(Identifier const& _identifier);
	void operator()(FunctionCall const&);
	void operator()(ExpressionStatement const& _statement);
	void operator()(Assignment const& _assignment);
	void operator()(VariableDeclaration const& _varDecl);
	void operator()(If const& _if);
	void operator()(Switch const& _switch);
	void operator()(FunctionDefinition const&);
	void operator()(ForLoop const&);
	void operator()(Break const&);
	void operator()(Continue const&);
	void operator()(Leave const&);
	void operator()(Block const& _block);

private:
	AbstractAssembly::LabelID labelFromIdentifier(Identifier const& _identifier);
	/// @returns the label ID corresponding to the given label, allocating a new one if
	/// necessary.
	AbstractAssembly::LabelID labelID(Scope::Label const& _label);
	AbstractAssembly::LabelID functionEntryID(YulString _name, Scope::Function const& _function);
	/// Generates code for an expression that is supposed to return a single value.
	void visitExpression(Expression const& _expression);

	void visitStatements(std::vector<Statement> const& _statements);

	/// Pops all variables declared in the block and checks that the stack height is equal
	/// to @a _blackStartStackHeight.
	void finalizeBlock(Block const& _block, int _blockStartStackHeight);

	void generateMultiAssignment(std::vector<Identifier> const& _variableNames);
	void generateAssignment(Identifier const& _variableName);

	/// Determines the stack height difference to the given variables. Throws
	/// if it is not yet in scope or the height difference is too large. Returns
	/// the (positive) stack height difference otherwise.
	int variableHeightDiff(Scope::Variable const& _var, YulString _name, bool _forSwap);

	void expectDeposit(int _deposit, int _oldHeight) const;

	void checkStackHeight(void const* _astElement) const;

	/// Stores the stack error in the list of errors, appends an invalid opcode
	/// and corrects the stack height to the target stack height.
	void stackError(StackTooDeepError _error, int _targetStackSize);

	/// Ensures stack height is down to @p _targetDepth by appending POP instructions to the output assembly.
	/// Returns the number of POP statements that have been appended.
	int appendPopUntil(int _targetDepth);

	AbstractAssembly& m_assembly;
	AsmAnalysisInfo& m_info;
	Scope* m_scope = nullptr;
	EVMDialect const& m_dialect;
	BuiltinContext& m_builtinContext;
	bool const m_allowStackOpt = true;
	bool const m_evm15 = false;
	bool const m_useNamedLabelsForFunctions = false;
	ExternalIdentifierAccess m_identifierAccess;
	/// Adjustment between the stack height as determined during the analysis phase
	/// and the stack height in the assembly. This is caused by an initial stack being present
	/// for inline assembly and different stack heights depending on the EVM backend used
	/// (EVM 1.0 or 1.5).
	int m_stackAdjustment = 0;
	std::shared_ptr<Context> m_context;

	/// Set of variables whose reference counter has reached zero,
	/// and whose stack slot will be marked as unused once we reach
	/// statement level in the scope where the variable was defined.
	std::set<Scope::Variable const*> m_variablesScheduledForDeletion;
	std::set<int> m_unusedStackSlots;

	std::vector<StackTooDeepError> m_stackErrors;
};

}
