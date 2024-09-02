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
 * Code generator for translating Yul / inline assembly to EVM.
 */

#include <libyul/backends/evm/EVMCodeTransform.h>

#include <libyul/optimiser/NameCollector.h>
#include <libyul/AsmAnalysisInfo.h>
#include <libyul/Utilities.h>

#include <libsolutil/Visitor.h>
#include <libsolutil/StackTooDeepString.h>

#include <liblangutil/Exceptions.h>

#include <libevmasm/Instruction.h>

#include <range/v3/view/reverse.hpp>

#include <range/v3/algorithm/max.hpp>
#include <range/v3/algorithm/none_of.hpp>
#include <range/v3/view/enumerate.hpp>
#include <range/v3/view/transform.hpp>

#include <utility>
#include <variant>

using namespace solidity;
using namespace solidity::yul;
using namespace solidity::util;

CodeTransform::CodeTransform(
	AbstractAssembly& _assembly,
	AsmAnalysisInfo& _analysisInfo,
	Block const& _block,
	bool _allowStackOpt,
	EVMDialect const& _dialect,
	BuiltinContext& _builtinContext,
	ExternalIdentifierAccess::CodeGenerator _identifierAccessCodeGen,
	UseNamedLabels _useNamedLabelsForFunctions,
	std::shared_ptr<Context> _context,
	std::vector<NameWithDebugData> _delayedReturnVariables,
	std::optional<AbstractAssembly::LabelID> _functionExitLabel
):
	m_assembly(_assembly),
	m_info(_analysisInfo),
	m_dialect(_dialect),
	m_builtinContext(_builtinContext),
	m_allowStackOpt(_allowStackOpt),
	m_useNamedLabelsForFunctions(_useNamedLabelsForFunctions),
	m_identifierAccessCodeGen(std::move(_identifierAccessCodeGen)),
	m_context(std::move(_context)),
	m_delayedReturnVariables(std::move(_delayedReturnVariables)),
	m_functionExitLabel(_functionExitLabel)
{
	if (!m_context)
	{
		// initialize
		m_context = std::make_shared<Context>();
		if (m_allowStackOpt)
			m_context->variableReferences = VariableReferenceCounter::run(m_info, _block);
	}
}

void CodeTransform::decreaseReference(YulName, Scope::Variable const& _var)
{
	if (!m_allowStackOpt)
		return;

	unsigned& ref = m_context->variableReferences.at(&_var);
	yulAssert(ref >= 1, "");
	--ref;
	if (ref == 0)
		m_variablesScheduledForDeletion.insert(&_var);
}

bool CodeTransform::unreferenced(Scope::Variable const& _var) const
{
	return !m_context->variableReferences.count(&_var) || m_context->variableReferences[&_var] == 0;
}

void CodeTransform::freeUnusedVariables(bool _popUnusedSlotsAtStackTop)
{
	if (!m_allowStackOpt)
		return;

	for (auto const& identifier: m_scope->identifiers)
		if (Scope::Variable const* var = std::get_if<Scope::Variable>(&identifier.second))
			if (m_variablesScheduledForDeletion.count(var))
				deleteVariable(*var);
	// Directly in a function body block, we can also delete the function arguments,
	// which live in the virtual function scope.
	// However, doing so after the return variables are already allocated, seems to have an adverse
	// effect, so we only do it before that.
	if (!returnVariablesAndFunctionExitAreSetup() && !m_scope->functionScope && m_scope->superScope && m_scope->superScope->functionScope)
		for (auto const& identifier: m_scope->superScope->identifiers)
			if (Scope::Variable const* var = std::get_if<Scope::Variable>(&identifier.second))
				if (m_variablesScheduledForDeletion.count(var))
					deleteVariable(*var);

	if (_popUnusedSlotsAtStackTop)
		while (m_unusedStackSlots.count(m_assembly.stackHeight() - 1))
		{
			yulAssert(m_unusedStackSlots.erase(m_assembly.stackHeight() - 1), "");
			m_assembly.appendInstruction(evmasm::Instruction::POP);
		}
}

void CodeTransform::deleteVariable(Scope::Variable const& _var)
{
	yulAssert(m_allowStackOpt, "");
	yulAssert(m_context->variableStackHeights.count(&_var) > 0, "");
	m_unusedStackSlots.insert(static_cast<int>(m_context->variableStackHeights[&_var]));
	m_context->variableStackHeights.erase(&_var);
	m_context->variableReferences.erase(&_var);
	m_variablesScheduledForDeletion.erase(&_var);
}

void CodeTransform::operator()(VariableDeclaration const& _varDecl)
{
	yulAssert(m_scope, "");

	size_t const numVariables = _varDecl.variables.size();
	auto heightAtStart = static_cast<size_t>(m_assembly.stackHeight());
	if (_varDecl.value)
	{
		std::visit(*this, *_varDecl.value);
		expectDeposit(static_cast<int>(numVariables), static_cast<int>(heightAtStart));
		freeUnusedVariables(false);
	}
	else
	{
		m_assembly.setSourceLocation(originLocationOf(_varDecl));
		size_t variablesLeft = numVariables;
		while (variablesLeft--)
			m_assembly.appendConstant(u256(0));
	}

	m_assembly.setSourceLocation(originLocationOf(_varDecl));
	bool atTopOfStack = true;
	for (size_t varIndex = 0; varIndex < numVariables; ++varIndex)
	{
		size_t varIndexReverse = numVariables - 1 - varIndex;
		YulName varName = _varDecl.variables[varIndexReverse].name;
		auto& var = std::get<Scope::Variable>(m_scope->identifiers.at(varName));
		m_context->variableStackHeights[&var] = heightAtStart + varIndexReverse;
		if (!m_allowStackOpt)
			continue;

		if (unreferenced(var))
		{
			if (atTopOfStack)
			{
				m_context->variableStackHeights.erase(&var);
				m_assembly.appendInstruction(evmasm::Instruction::POP);
			}
			else
				m_variablesScheduledForDeletion.insert(&var);
		}
		else
		{
			bool foundUnusedSlot = false;
			for (auto it = m_unusedStackSlots.begin(); it != m_unusedStackSlots.end(); ++it)
			{
				if (m_assembly.stackHeight() - *it > 17)
					continue;
				foundUnusedSlot = true;
				auto slot = static_cast<size_t>(*it);
				m_unusedStackSlots.erase(it);
				m_context->variableStackHeights[&var] = slot;
				if (size_t heightDiff = variableHeightDiff(var, varName, true))
					m_assembly.appendInstruction(evmasm::swapInstruction(static_cast<unsigned>(heightDiff - 1)));
				m_assembly.appendInstruction(evmasm::Instruction::POP);
				break;
			}
			if (!foundUnusedSlot)
				atTopOfStack = false;
		}
	}
}

void CodeTransform::stackError(StackTooDeepError _error, int _targetStackHeight)
{
	m_assembly.appendInstruction(evmasm::Instruction::INVALID);
	// Correct the stack.
	while (m_assembly.stackHeight() > _targetStackHeight)
		m_assembly.appendInstruction(evmasm::Instruction::POP);
	while (m_assembly.stackHeight() < _targetStackHeight)
		m_assembly.appendConstant(u256(0));
	// Store error.
	m_stackErrors.emplace_back(std::move(_error));
	m_assembly.markAsInvalid();
}

void CodeTransform::operator()(Assignment const& _assignment)
{
	int height = m_assembly.stackHeight();
	std::visit(*this, *_assignment.value);
	expectDeposit(static_cast<int>(_assignment.variableNames.size()), height);

	m_assembly.setSourceLocation(originLocationOf(_assignment));
	generateMultiAssignment(_assignment.variableNames);
}

void CodeTransform::operator()(ExpressionStatement const& _statement)
{
	m_assembly.setSourceLocation(originLocationOf(_statement));
	std::visit(*this, _statement.expression);
}

void CodeTransform::operator()(FunctionCall const& _call)
{
	yulAssert(m_scope, "");

	m_assembly.setSourceLocation(originLocationOf(_call));
	if (BuiltinFunctionForEVM const* builtin = m_dialect.builtin(_call.functionName.name))
	{
		for (auto&& [i, arg]: _call.arguments | ranges::views::enumerate | ranges::views::reverse)
			if (!builtin->literalArgument(i))
				visitExpression(arg);
		m_assembly.setSourceLocation(originLocationOf(_call));
		builtin->generateCode(_call, m_assembly, m_builtinContext);
	}
	else
	{
		AbstractAssembly::LabelID returnLabel = m_assembly.newLabelId();
		m_assembly.appendLabelReference(returnLabel);

		Scope::Function* function = nullptr;
		yulAssert(m_scope->lookup(_call.functionName.name, GenericVisitor{
			[](Scope::Variable&) { yulAssert(false, "Expected function name."); },
			[&](Scope::Function& _function) { function = &_function; }
		}), "Function name not found.");
		yulAssert(function, "");
		yulAssert(function->numArguments == _call.arguments.size(), "");
		for (auto const& arg: _call.arguments | ranges::views::reverse)
			visitExpression(arg);
		m_assembly.setSourceLocation(originLocationOf(_call));
		m_assembly.appendJumpTo(
			functionEntryID(*function),
			static_cast<int>(function->numReturns) - static_cast<int>(function->numArguments) - 1,
			AbstractAssembly::JumpType::IntoFunction
		);
		m_assembly.appendLabel(returnLabel);
	}
}

void CodeTransform::operator()(Identifier const& _identifier)
{
	m_assembly.setSourceLocation(originLocationOf(_identifier));
	// First search internals, then externals.
	yulAssert(m_scope, "");
	if (m_scope->lookup(_identifier.name, GenericVisitor{
		[&](Scope::Variable& _var)
		{
			// TODO: opportunity for optimization: Do not DUP if this is the last reference
			// to the top most element of the stack
			if (size_t heightDiff = variableHeightDiff(_var, _identifier.name, false))
				m_assembly.appendInstruction(evmasm::dupInstruction(static_cast<unsigned>(heightDiff)));
			else
				// Store something to balance the stack
				m_assembly.appendConstant(u256(0));
			decreaseReference(_identifier.name, _var);
		},
		[](Scope::Function&)
		{
			yulAssert(false, "Function not removed during desugaring.");
		}
	}))
	{
		return;
	}
	yulAssert(
		m_identifierAccessCodeGen,
		"Identifier not found and no external access available."
	);
	m_identifierAccessCodeGen(_identifier, IdentifierContext::RValue, m_assembly);
}

void CodeTransform::operator()(Literal const& _literal)
{
	m_assembly.setSourceLocation(originLocationOf(_literal));
	m_assembly.appendConstant(_literal.value.value());
}

void CodeTransform::operator()(If const& _if)
{
	visitExpression(*_if.condition);
	m_assembly.setSourceLocation(originLocationOf(_if));
	m_assembly.appendInstruction(evmasm::Instruction::ISZERO);
	AbstractAssembly::LabelID end = m_assembly.newLabelId();
	m_assembly.appendJumpToIf(end);
	(*this)(_if.body);
	m_assembly.setSourceLocation(originLocationOf(_if));
	m_assembly.appendLabel(end);
}

void CodeTransform::operator()(Switch const& _switch)
{
	visitExpression(*_switch.expression);
	int expressionHeight = m_assembly.stackHeight();
	std::map<Case const*, AbstractAssembly::LabelID> caseBodies;
	AbstractAssembly::LabelID end = m_assembly.newLabelId();
	for (Case const& c: _switch.cases)
	{
		if (c.value)
		{
			(*this)(*c.value);
			m_assembly.setSourceLocation(originLocationOf(c));
			AbstractAssembly::LabelID bodyLabel = m_assembly.newLabelId();
			caseBodies[&c] = bodyLabel;
			yulAssert(m_assembly.stackHeight() == expressionHeight + 1, "");
			m_assembly.appendInstruction(evmasm::dupInstruction(2));
			m_assembly.appendInstruction(evmasm::Instruction::EQ);
			m_assembly.appendJumpToIf(bodyLabel);
		}
		else
			// default case
			(*this)(c.body);
	}
	m_assembly.setSourceLocation(originLocationOf(_switch));
	m_assembly.appendJumpTo(end);

	size_t numCases = caseBodies.size();
	for (auto const& c: caseBodies)
	{
		m_assembly.setSourceLocation(originLocationOf(*c.first));
		m_assembly.appendLabel(c.second);
		(*this)(c.first->body);
		// Avoid useless "jump to next" for the last case.
		if (--numCases > 0)
		{
			m_assembly.setSourceLocation(originLocationOf(*c.first));
			m_assembly.appendJumpTo(end);
		}
	}

	m_assembly.setSourceLocation(originLocationOf(_switch));
	m_assembly.appendLabel(end);
	m_assembly.appendInstruction(evmasm::Instruction::POP);
}

void CodeTransform::operator()(FunctionDefinition const& _function)
{
	yulAssert(m_scope, "");
	yulAssert(m_scope->identifiers.count(_function.name), "");
	Scope::Function& function = std::get<Scope::Function>(m_scope->identifiers.at(_function.name));

	size_t height = 1;
	yulAssert(m_info.scopes.at(&_function.body), "");
	Scope* virtualFunctionScope = m_info.scopes.at(m_info.virtualBlocks.at(&_function).get()).get();
	yulAssert(virtualFunctionScope, "");
	for (auto const& v: _function.parameters | ranges::views::reverse)
	{
		auto& var = std::get<Scope::Variable>(virtualFunctionScope->identifiers.at(v.name));
		m_context->variableStackHeights[&var] = height++;
	}

	m_assembly.setSourceLocation(originLocationOf(_function));
	int const stackHeightBefore = m_assembly.stackHeight();

	m_assembly.appendLabel(functionEntryID(function));

	m_assembly.setStackHeight(static_cast<int>(height));

	CodeTransform subTransform(
		m_assembly,
		m_info,
		_function.body,
		m_allowStackOpt,
		m_dialect,
		m_builtinContext,
		m_identifierAccessCodeGen,
		m_useNamedLabelsForFunctions,
		m_context,
		_function.returnVariables,
		m_assembly.newLabelId()
	);
	subTransform.m_scope = virtualFunctionScope;

	if (m_allowStackOpt)
		// Immediately delete entirely unused parameters.
		for (auto const& v: _function.parameters | ranges::views::reverse)
		{
			auto& var = std::get<Scope::Variable>(virtualFunctionScope->identifiers.at(v.name));
			if (util::valueOrDefault(m_context->variableReferences, &var, 0u) == 0)
				subTransform.deleteVariable(var);
		}

	if (!m_allowStackOpt)
		subTransform.setupReturnVariablesAndFunctionExit();

	subTransform.m_assignedNamedLabels = std::move(m_assignedNamedLabels);

	subTransform(_function.body);

	m_assignedNamedLabels = std::move(subTransform.m_assignedNamedLabels);

	m_assembly.setSourceLocation(originLocationOf(_function));
	if (!subTransform.m_stackErrors.empty())
	{
		m_assembly.markAsInvalid();
		for (StackTooDeepError& stackError: subTransform.m_stackErrors)
		{
			if (stackError.functionName.empty())
				stackError.functionName = _function.name;
			m_stackErrors.emplace_back(std::move(stackError));
		}
	}

	if (!subTransform.returnVariablesAndFunctionExitAreSetup())
		subTransform.setupReturnVariablesAndFunctionExit();
	appendPopUntil(*subTransform.m_functionExitStackHeight);

	yulAssert(
		subTransform.m_functionExitStackHeight &&
		*subTransform.m_functionExitStackHeight == m_assembly.stackHeight(),
		""
	);

	m_assembly.appendLabel(*subTransform.m_functionExitLabel);

	{
		// The stack layout here is:
		// <return label>? <arguments...> <return values...>
		// But we would like it to be:
		// <return values...> <return label>?
		// So we have to append some SWAP and POP instructions.

		// This vector holds the desired target positions of all stack slots and is
		// modified parallel to the actual stack.
		std::vector<int> stackLayout(static_cast<size_t>(m_assembly.stackHeight()), -1);
		stackLayout[0] = static_cast<int>(_function.returnVariables.size()); // Move return label to the top
		for (auto&& [n, returnVariable]: ranges::views::enumerate(_function.returnVariables))
			stackLayout.at(m_context->variableStackHeights.at(
				&std::get<Scope::Variable>(virtualFunctionScope->identifiers.at(returnVariable.name))
			)) = static_cast<int>(n);

		if (stackLayout.size() > 17)
		{
			StackTooDeepError error(
				_function.name,
				YulName{},
				static_cast<int>(stackLayout.size()) - 17,
				"The function " +
				_function.name.str() +
				" has " +
				std::to_string(stackLayout.size() - 17) +
				" parameters or return variables too many to fit the stack size."
			);
			stackError(std::move(error), m_assembly.stackHeight() - static_cast<int>(_function.parameters.size()));
		}
		else
		{
			while (!stackLayout.empty() && stackLayout.back() != static_cast<int>(stackLayout.size() - 1))
				if (stackLayout.back() < 0)
				{
					m_assembly.appendInstruction(evmasm::Instruction::POP);
					stackLayout.pop_back();
				}
				else
				{
					m_assembly.appendInstruction(evmasm::swapInstruction(static_cast<unsigned>(stackLayout.size()) - static_cast<unsigned>(stackLayout.back()) - 1u));
					std::swap(stackLayout[static_cast<size_t>(stackLayout.back())], stackLayout.back());
				}
			for (size_t i = 0; i < stackLayout.size(); ++i)
				yulAssert(i == static_cast<size_t>(stackLayout[i]), "Error reshuffling stack.");
		}
	}
	m_assembly.appendJump(
		stackHeightBefore - static_cast<int>(_function.returnVariables.size()),
		AbstractAssembly::JumpType::OutOfFunction
	);
	m_assembly.setStackHeight(stackHeightBefore);
}

void CodeTransform::operator()(ForLoop const& _forLoop)
{
	Scope* originalScope = m_scope;
	// We start with visiting the block, but not finalizing it.
	m_scope = m_info.scopes.at(&_forLoop.pre).get();
	int stackStartHeight = m_assembly.stackHeight();

	visitStatements(_forLoop.pre.statements);

	AbstractAssembly::LabelID loopStart = m_assembly.newLabelId();
	AbstractAssembly::LabelID postPart = m_assembly.newLabelId();
	AbstractAssembly::LabelID loopEnd = m_assembly.newLabelId();

	m_assembly.setSourceLocation(originLocationOf(_forLoop));
	m_assembly.appendLabel(loopStart);

	visitExpression(*_forLoop.condition);
	m_assembly.setSourceLocation(originLocationOf(_forLoop));
	m_assembly.appendInstruction(evmasm::Instruction::ISZERO);
	m_assembly.appendJumpToIf(loopEnd);

	int const stackHeightBody = m_assembly.stackHeight();
	m_context->forLoopStack.emplace(Context::ForLoopLabels{ {postPart, stackHeightBody}, {loopEnd, stackHeightBody} });
	(*this)(_forLoop.body);

	m_assembly.setSourceLocation(originLocationOf(_forLoop));
	m_assembly.appendLabel(postPart);

	(*this)(_forLoop.post);

	m_assembly.setSourceLocation(originLocationOf(_forLoop));
	m_assembly.appendJumpTo(loopStart);
	m_assembly.appendLabel(loopEnd);

	finalizeBlock(_forLoop.pre, stackStartHeight);
	m_context->forLoopStack.pop();
	m_scope = originalScope;
}

int CodeTransform::appendPopUntil(int _targetDepth)
{
	int const stackDiffAfter = m_assembly.stackHeight() - _targetDepth;
	for (int i = 0; i < stackDiffAfter; ++i)
		m_assembly.appendInstruction(evmasm::Instruction::POP);
	return stackDiffAfter;
}

void CodeTransform::operator()(Break const& _break)
{
	yulAssert(!m_context->forLoopStack.empty(), "Invalid break-statement. Requires surrounding for-loop in code generation.");
	m_assembly.setSourceLocation(originLocationOf(_break));

	Context::JumpInfo const& jump = m_context->forLoopStack.top().done;
	m_assembly.appendJumpTo(jump.label, appendPopUntil(jump.targetStackHeight));
}

void CodeTransform::operator()(Continue const& _continue)
{
	yulAssert(!m_context->forLoopStack.empty(), "Invalid continue-statement. Requires surrounding for-loop in code generation.");
	m_assembly.setSourceLocation(originLocationOf(_continue));

	Context::JumpInfo const& jump = m_context->forLoopStack.top().post;
	m_assembly.appendJumpTo(jump.label, appendPopUntil(jump.targetStackHeight));
}

void CodeTransform::operator()(Leave const& _leaveStatement)
{
	yulAssert(m_functionExitLabel, "Invalid leave-statement. Requires surrounding function in code generation.");
	yulAssert(m_functionExitStackHeight, "");
	m_assembly.setSourceLocation(originLocationOf(_leaveStatement));
	m_assembly.appendJumpTo(*m_functionExitLabel, appendPopUntil(*m_functionExitStackHeight));
}

void CodeTransform::operator()(Block const& _block)
{
	Scope* originalScope = m_scope;
	m_scope = m_info.scopes.at(&_block).get();

	for (auto const& statement: _block.statements)
		if (auto function = std::get_if<FunctionDefinition>(&statement))
			createFunctionEntryID(*function);

	int blockStartStackHeight = m_assembly.stackHeight();
	visitStatements(_block.statements);

	bool isOutermostFunctionBodyBlock = m_scope && m_scope->superScope && m_scope->superScope->functionScope;
	bool performValidation = !m_allowStackOpt || !isOutermostFunctionBodyBlock;
	finalizeBlock(_block, performValidation ? std::make_optional(blockStartStackHeight) : std::nullopt);
	m_scope = originalScope;
}

void CodeTransform::createFunctionEntryID(FunctionDefinition const& _function)
{
	Scope::Function& scopeFunction = std::get<Scope::Function>(m_scope->identifiers.at(_function.name));
	yulAssert(!m_context->functionEntryIDs.count(&scopeFunction), "");

	std::optional<size_t> astID;
	if (_function.debugData)
		astID = _function.debugData->astID;

	bool nameAlreadySeen = !m_assignedNamedLabels.insert(_function.name).second;

	if (m_useNamedLabelsForFunctions == UseNamedLabels::YesAndForceUnique)
		yulAssert(!nameAlreadySeen);

	m_context->functionEntryIDs[&scopeFunction] =
		(
			m_useNamedLabelsForFunctions != UseNamedLabels::Never &&
			!nameAlreadySeen
		) ?
		m_assembly.namedLabel(
			_function.name.str(),
			_function.parameters.size(),
			_function.returnVariables.size(),
			astID
		) :
		m_assembly.newLabelId();
}

AbstractAssembly::LabelID CodeTransform::functionEntryID(Scope::Function const& _scopeFunction) const
{
	yulAssert(m_context->functionEntryIDs.count(&_scopeFunction), "");
	return m_context->functionEntryIDs.at(&_scopeFunction);
}

void CodeTransform::visitExpression(Expression const& _expression)
{
	int height = m_assembly.stackHeight();
	std::visit(*this, _expression);
	expectDeposit(1, height);
}

void CodeTransform::setupReturnVariablesAndFunctionExit()
{
	yulAssert(isInsideFunction(), "");
	yulAssert(!returnVariablesAndFunctionExitAreSetup(), "");
	yulAssert(m_scope, "");

	ScopeGuard scopeGuard([oldScope = m_scope, this] { m_scope = oldScope; });
	if (!m_scope->functionScope)
	{
		yulAssert(m_scope->superScope && m_scope->superScope->functionScope, "");
		m_scope = m_scope->superScope;
	}

	// We could reuse unused slots for return variables, but it turns out this is detrimental in practice.
	m_unusedStackSlots.clear();

	if (m_delayedReturnVariables.empty())
	{
		m_functionExitStackHeight = 1;
		return;
	}

	// Allocate slots for return variables as if they were declared as variables in the virtual function scope.
	for (NameWithDebugData const& var: m_delayedReturnVariables)
		(*this)(VariableDeclaration{var.debugData, {var}, {}});

	m_functionExitStackHeight = ranges::max(m_delayedReturnVariables | ranges::views::transform([&](NameWithDebugData const& _name) {
		return variableStackHeight(_name.name);
	})) + 1;
	m_delayedReturnVariables.clear();
}

namespace
{

bool statementNeedsReturnVariableSetup(Statement const& _statement, std::vector<NameWithDebugData> const& _returnVariables)
{
	if (std::holds_alternative<FunctionDefinition>(_statement))
		return true;
	if (
		std::holds_alternative<ExpressionStatement>(_statement) ||
		std::holds_alternative<Assignment>(_statement)
	)
	{
		std::map<YulName, size_t> references = VariableReferencesCounter::countReferences(_statement);
		auto isReferenced = [&references](NameWithDebugData const& _returnVariable) {
			return references.count(_returnVariable.name);
		};
		if (ranges::none_of(_returnVariables, isReferenced))
			return false;
	}
	return true;
}

}

void CodeTransform::visitStatements(std::vector<Statement> const& _statements)
{
	std::optional<AbstractAssembly::LabelID> jumpTarget = std::nullopt;

	for (auto const& statement: _statements)
	{
		freeUnusedVariables();
		if (
			isInsideFunction() &&
			!returnVariablesAndFunctionExitAreSetup() &&
			statementNeedsReturnVariableSetup(statement, m_delayedReturnVariables)
		)
			setupReturnVariablesAndFunctionExit();

		auto const* functionDefinition = std::get_if<FunctionDefinition>(&statement);
		if (functionDefinition && !jumpTarget)
		{
			m_assembly.setSourceLocation(originLocationOf(*functionDefinition));
			jumpTarget = m_assembly.newLabelId();
			m_assembly.appendJumpTo(*jumpTarget, 0);
		}
		else if (!functionDefinition && jumpTarget)
		{
			m_assembly.appendLabel(*jumpTarget);
			jumpTarget = std::nullopt;
		}

		std::visit(*this, statement);
	}
	// we may have a leftover jumpTarget
	if (jumpTarget)
		m_assembly.appendLabel(*jumpTarget);

	freeUnusedVariables();
}

void CodeTransform::finalizeBlock(Block const& _block, std::optional<int> blockStartStackHeight)
{
	m_assembly.setSourceLocation(originLocationOf(_block));

	freeUnusedVariables();

	// pop variables
	yulAssert(m_info.scopes.at(&_block).get() == m_scope, "");
	for (auto const& id: m_scope->identifiers)
		if (std::holds_alternative<Scope::Variable>(id.second))
		{
			Scope::Variable const& var = std::get<Scope::Variable>(id.second);
			if (m_allowStackOpt)
			{
				yulAssert(!m_context->variableStackHeights.count(&var), "");
				yulAssert(!m_context->variableReferences.count(&var), "");
			}
			else
				m_assembly.appendInstruction(evmasm::Instruction::POP);
		}

	if (blockStartStackHeight)
	{
		int deposit = m_assembly.stackHeight() - *blockStartStackHeight;
		yulAssert(deposit == 0, "Invalid stack height at end of block: " + std::to_string(deposit));
	}
}

void CodeTransform::generateMultiAssignment(std::vector<Identifier> const& _variableNames)
{
	yulAssert(m_scope, "");
	for (auto const& variableName: _variableNames | ranges::views::reverse)
		generateAssignment(variableName);
}

void CodeTransform::generateAssignment(Identifier const& _variableName)
{
	yulAssert(m_scope, "");
	if (auto var = m_scope->lookup(_variableName.name))
	{
		Scope::Variable const& _var = std::get<Scope::Variable>(*var);
		if (size_t heightDiff = variableHeightDiff(_var, _variableName.name, true))
			m_assembly.appendInstruction(evmasm::swapInstruction(static_cast<unsigned>(heightDiff - 1)));
		m_assembly.appendInstruction(evmasm::Instruction::POP);
		decreaseReference(_variableName.name, _var);
	}
	else
	{
		yulAssert(
			m_identifierAccessCodeGen,
			"Identifier not found and no external access available."
		);
		m_identifierAccessCodeGen(_variableName, IdentifierContext::LValue, m_assembly);
	}
}

size_t CodeTransform::variableHeightDiff(Scope::Variable const& _var, YulName _varName, bool _forSwap)
{
	yulAssert(m_context->variableStackHeights.count(&_var), "");
	size_t heightDiff = static_cast<size_t>(m_assembly.stackHeight()) - m_context->variableStackHeights[&_var];
	yulAssert(heightDiff > (_forSwap ? 1 : 0), "Negative stack difference for variable.");
	size_t limit = _forSwap ? 17 : 16;
	if (heightDiff > limit)
	{
		m_stackErrors.emplace_back(
			_varName,
			heightDiff - limit,
			"Variable " +
			_varName.str() +
			" is " +
			std::to_string(heightDiff - limit) +
			" slot(s) too deep inside the stack. " +
			stackTooDeepString
		);
		m_assembly.markAsInvalid();
		return _forSwap ? 2 : 1;
	}
	return heightDiff;
}

int CodeTransform::variableStackHeight(YulName _name) const
{
	Scope::Variable const* var = std::get_if<Scope::Variable>(m_scope->lookup(_name));
	yulAssert(var, "");
	return static_cast<int>(m_context->variableStackHeights.at(var));
}

void CodeTransform::expectDeposit(int _deposit, int _oldHeight) const
{
	yulAssert(m_assembly.stackHeight() == _oldHeight + _deposit, "Invalid stack deposit.");
}
