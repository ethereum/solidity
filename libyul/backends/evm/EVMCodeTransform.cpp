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

#include <libyul/backends/evm/EVMCodeTransform.h>

#include <libyul/optimiser/NameCollector.h>
#include <libyul/AsmAnalysisInfo.h>
#include <libyul/AsmData.h>
#include <libyul/Utilities.h>

#include <liblangutil/Exceptions.h>

#include <boost/range/adaptor/reversed.hpp>

using namespace std;
using namespace dev;
using namespace yul;
using namespace dev::solidity;

void VariableReferenceCounter::operator()(Identifier const& _identifier)
{
	increaseRefIfFound(_identifier.name);
}

void VariableReferenceCounter::operator()(FunctionDefinition const& _function)
{
	Scope* originalScope = m_scope;

	solAssert(m_info.virtualBlocks.at(&_function), "");
	m_scope = m_info.scopes.at(m_info.virtualBlocks.at(&_function).get()).get();
	solAssert(m_scope, "Variable scope does not exist.");

	for (auto const& v: _function.returnVariables)
		increaseRefIfFound(v.name);

	VariableReferenceCounter{m_context, m_info}(_function.body);

	m_scope = originalScope;
}

void VariableReferenceCounter::operator()(ForLoop const& _forLoop)
{
	Scope* originalScope = m_scope;
	// Special scoping rules.
	m_scope = m_info.scopes.at(&_forLoop.pre).get();

	walkVector(_forLoop.pre.statements);
	visit(*_forLoop.condition);
	(*this)(_forLoop.body);
	(*this)(_forLoop.post);

	m_scope = originalScope;
}

void VariableReferenceCounter::operator()(Block const& _block)
{
	Scope* originalScope = m_scope;
	m_scope = m_info.scopes.at(&_block).get();

	ASTWalker::operator()(_block);

	m_scope = originalScope;
}

void VariableReferenceCounter::increaseRefIfFound(YulString _variableName)
{
	m_scope->lookup(_variableName, Scope::Visitor(
		[=](Scope::Variable const& _var)
		{
			++m_context.variableReferences[&_var];
		},
		[=](Scope::Label const&) { },
		[=](Scope::Function const&) { }
	));
}

CodeTransform::CodeTransform(
	AbstractAssembly& _assembly,
	AsmAnalysisInfo& _analysisInfo,
	Block const& _block,
	bool _allowStackOpt,
	EVMDialect const& _dialect,
	bool _evm15,
	ExternalIdentifierAccess const& _identifierAccess,
	bool _useNamedLabelsForFunctions,
	int _stackAdjustment,
	shared_ptr<Context> _context
):
	m_assembly(_assembly),
	m_info(_analysisInfo),
	m_dialect(_dialect),
	m_allowStackOpt(_allowStackOpt),
	m_evm15(_evm15),
	m_useNamedLabelsForFunctions(_useNamedLabelsForFunctions),
	m_identifierAccess(_identifierAccess),
	m_stackAdjustment(_stackAdjustment),
	m_context(_context)
{
	if (!m_context)
	{
		// initialize
		m_context = make_shared<Context>();
		if (m_allowStackOpt)
			VariableReferenceCounter{*m_context, m_info}(_block);
	}
}

void CodeTransform::decreaseReference(YulString, Scope::Variable const& _var)
{
	if (!m_allowStackOpt)
		return;

	unsigned& ref = m_context->variableReferences.at(&_var);
	solAssert(ref >= 1, "");
	--ref;
	if (ref == 0)
		m_variablesScheduledForDeletion.insert(&_var);
}

bool CodeTransform::unreferenced(Scope::Variable const& _var) const
{
	return !m_context->variableReferences.count(&_var) || m_context->variableReferences[&_var] == 0;
}

void CodeTransform::freeUnusedVariables()
{
	if (!m_allowStackOpt)
		return;

	for (auto const& identifier: m_scope->identifiers)
		if (identifier.second.type() == typeid(Scope::Variable))
		{
			Scope::Variable const& var = boost::get<Scope::Variable>(identifier.second);
			if (m_variablesScheduledForDeletion.count(&var))
				deleteVariable(var);
		}

	while (m_unusedStackSlots.count(m_assembly.stackHeight() - 1))
	{
		solAssert(m_unusedStackSlots.erase(m_assembly.stackHeight() - 1), "");
		m_assembly.appendInstruction(solidity::Instruction::POP);
		--m_stackAdjustment;
	}
}

void CodeTransform::deleteVariable(Scope::Variable const& _var)
{
	solAssert(m_allowStackOpt, "");
	solAssert(m_context->variableStackHeights.count(&_var) > 0, "");
	m_unusedStackSlots.insert(m_context->variableStackHeights[&_var]);
	m_context->variableStackHeights.erase(&_var);
	m_context->variableReferences.erase(&_var);
	m_variablesScheduledForDeletion.erase(&_var);
}

void CodeTransform::operator()(VariableDeclaration const& _varDecl)
{
	solAssert(m_scope, "");

	int const numVariables = _varDecl.variables.size();
	int height = m_assembly.stackHeight();
	if (_varDecl.value)
	{
		boost::apply_visitor(*this, *_varDecl.value);
		expectDeposit(numVariables, height);
	}
	else
	{
		int variablesLeft = numVariables;
		while (variablesLeft--)
			m_assembly.appendConstant(u256(0));
	}

	bool atTopOfStack = true;
	for (int varIndex = numVariables - 1; varIndex >= 0; --varIndex)
	{
		YulString varName = _varDecl.variables[varIndex].name;
		auto& var = boost::get<Scope::Variable>(m_scope->identifiers.at(varName));
		m_context->variableStackHeights[&var] = height + varIndex;
		if (!m_allowStackOpt)
			continue;

		if (unreferenced(var))
		{
			if (atTopOfStack)
			{
				m_context->variableStackHeights.erase(&var);
				m_assembly.setSourceLocation(_varDecl.location);
				m_assembly.appendInstruction(solidity::Instruction::POP);
				--m_stackAdjustment;
			}
			else
				m_variablesScheduledForDeletion.insert(&var);
		}
		else if (m_unusedStackSlots.empty())
			atTopOfStack = false;
		else
		{
			int slot = *m_unusedStackSlots.begin();
			m_unusedStackSlots.erase(m_unusedStackSlots.begin());
			m_context->variableStackHeights[&var] = slot;
			m_assembly.setSourceLocation(_varDecl.location);
			if (int heightDiff = variableHeightDiff(var, varName, true))
				m_assembly.appendInstruction(solidity::swapInstruction(heightDiff - 1));
			m_assembly.appendInstruction(solidity::Instruction::POP);
			--m_stackAdjustment;
		}
	}
	checkStackHeight(&_varDecl);
}

void CodeTransform::stackError(StackTooDeepError _error, int _targetStackHeight)
{
	m_assembly.appendInstruction(solidity::Instruction::INVALID);
	// Correct the stack.
	while (m_assembly.stackHeight() > _targetStackHeight)
		m_assembly.appendInstruction(solidity::Instruction::POP);
	while (m_assembly.stackHeight() < _targetStackHeight)
		m_assembly.appendConstant(u256(0));
	// Store error.
	m_stackErrors.emplace_back(std::move(_error));
}

void CodeTransform::operator()(Assignment const& _assignment)
{
	int height = m_assembly.stackHeight();
	boost::apply_visitor(*this, *_assignment.value);
	expectDeposit(_assignment.variableNames.size(), height);

	m_assembly.setSourceLocation(_assignment.location);
	generateMultiAssignment(_assignment.variableNames);
	checkStackHeight(&_assignment);
}

void CodeTransform::operator()(StackAssignment const& _assignment)
{
	solAssert(!m_allowStackOpt, "");
	m_assembly.setSourceLocation(_assignment.location);
	generateAssignment(_assignment.variableName);
	checkStackHeight(&_assignment);
}

void CodeTransform::operator()(ExpressionStatement const& _statement)
{
	m_assembly.setSourceLocation(_statement.location);
	boost::apply_visitor(*this, _statement.expression);
	checkStackHeight(&_statement);
}

void CodeTransform::operator()(Label const& _label)
{
	solAssert(!m_allowStackOpt, "");
	m_assembly.setSourceLocation(_label.location);
	solAssert(m_scope, "");
	solAssert(m_scope->identifiers.count(_label.name), "");
	Scope::Label& label = boost::get<Scope::Label>(m_scope->identifiers.at(_label.name));
	m_assembly.appendLabel(labelID(label));
	checkStackHeight(&_label);
}

void CodeTransform::operator()(FunctionCall const& _call)
{
	solAssert(m_scope, "");

	if (BuiltinFunctionForEVM const* builtin = m_dialect.builtin(_call.functionName.name))
	{
		builtin->generateCode(_call, m_assembly, [&]() {
			for (auto const& arg: _call.arguments | boost::adaptors::reversed)
				visitExpression(arg);
			m_assembly.setSourceLocation(_call.location);
		});
	}
	else
	{
		m_assembly.setSourceLocation(_call.location);
		EVMAssembly::LabelID returnLabel(-1); // only used for evm 1.0
		if (!m_evm15)
		{
			returnLabel = m_assembly.newLabelId();
			m_assembly.appendLabelReference(returnLabel);
			m_stackAdjustment++;
		}

		Scope::Function* function = nullptr;
		solAssert(m_scope->lookup(_call.functionName.name, Scope::NonconstVisitor(
			[=](Scope::Variable&) { solAssert(false, "Expected function name."); },
			[=](Scope::Label&) { solAssert(false, "Expected function name."); },
			[&](Scope::Function& _function) { function = &_function; }
		)), "Function name not found.");
		solAssert(function, "");
		solAssert(function->arguments.size() == _call.arguments.size(), "");
		for (auto const& arg: _call.arguments | boost::adaptors::reversed)
			visitExpression(arg);
		m_assembly.setSourceLocation(_call.location);
		if (m_evm15)
			m_assembly.appendJumpsub(functionEntryID(_call.functionName.name, *function), function->arguments.size(), function->returns.size());
		else
		{
			m_assembly.appendJumpTo(functionEntryID(_call.functionName.name, *function), function->returns.size() - function->arguments.size() - 1);
			m_assembly.appendLabel(returnLabel);
			m_stackAdjustment--;
		}
		checkStackHeight(&_call);
	}
}

void CodeTransform::operator()(FunctionalInstruction const& _instruction)
{
	if (m_evm15 && (
		_instruction.instruction == solidity::Instruction::JUMP ||
		_instruction.instruction == solidity::Instruction::JUMPI
	))
	{
		bool const isJumpI = _instruction.instruction == solidity::Instruction::JUMPI;
		if (isJumpI)
		{
			solAssert(_instruction.arguments.size() == 2, "");
			visitExpression(_instruction.arguments.at(1));
		}
		else
		{
			solAssert(_instruction.arguments.size() == 1, "");
		}
		m_assembly.setSourceLocation(_instruction.location);
		auto label = labelFromIdentifier(boost::get<Identifier>(_instruction.arguments.at(0)));
		if (isJumpI)
			m_assembly.appendJumpToIf(label);
		else
			m_assembly.appendJumpTo(label);
	}
	else
	{
		for (auto const& arg: _instruction.arguments | boost::adaptors::reversed)
			visitExpression(arg);
		m_assembly.setSourceLocation(_instruction.location);
		m_assembly.appendInstruction(_instruction.instruction);
	}
	checkStackHeight(&_instruction);
}

void CodeTransform::operator()(Identifier const& _identifier)
{
	m_assembly.setSourceLocation(_identifier.location);
	// First search internals, then externals.
	solAssert(m_scope, "");
	if (m_scope->lookup(_identifier.name, Scope::NonconstVisitor(
		[=](Scope::Variable& _var)
		{
			// TODO: opportunity for optimization: Do not DUP if this is the last reference
			// to the top most element of the stack
			if (int heightDiff = variableHeightDiff(_var, _identifier.name, false))
				m_assembly.appendInstruction(solidity::dupInstruction(heightDiff));
			else
				// Store something to balance the stack
				m_assembly.appendConstant(u256(0));
			decreaseReference(_identifier.name, _var);
		},
		[=](Scope::Label& _label)
		{
			m_assembly.appendLabelReference(labelID(_label));
		},
		[=](Scope::Function&)
		{
			solAssert(false, "Function not removed during desugaring.");
		}
	)))
	{
		return;
	}
	solAssert(
		m_identifierAccess.generateCode,
		"Identifier not found and no external access available."
	);
	m_identifierAccess.generateCode(_identifier, IdentifierContext::RValue, m_assembly);
	checkStackHeight(&_identifier);
}

void CodeTransform::operator()(Literal const& _literal)
{
	m_assembly.setSourceLocation(_literal.location);
	m_assembly.appendConstant(valueOfLiteral(_literal));

	checkStackHeight(&_literal);
}

void CodeTransform::operator()(yul::Instruction const& _instruction)
{
	solAssert(!m_allowStackOpt, "");
	solAssert(!m_evm15 || _instruction.instruction != solidity::Instruction::JUMP, "Bare JUMP instruction used for EVM1.5");
	solAssert(!m_evm15 || _instruction.instruction != solidity::Instruction::JUMPI, "Bare JUMPI instruction used for EVM1.5");
	m_assembly.setSourceLocation(_instruction.location);
	m_assembly.appendInstruction(_instruction.instruction);
	checkStackHeight(&_instruction);
}

void CodeTransform::operator()(If const& _if)
{
	visitExpression(*_if.condition);
	m_assembly.setSourceLocation(_if.location);
	m_assembly.appendInstruction(solidity::Instruction::ISZERO);
	AbstractAssembly::LabelID end = m_assembly.newLabelId();
	m_assembly.appendJumpToIf(end);
	(*this)(_if.body);
	m_assembly.setSourceLocation(_if.location);
	m_assembly.appendLabel(end);
	checkStackHeight(&_if);
}

void CodeTransform::operator()(Switch const& _switch)
{
	//@TODO use JUMPV in EVM1.5?

	visitExpression(*_switch.expression);
	int expressionHeight = m_assembly.stackHeight();
	map<Case const*, AbstractAssembly::LabelID> caseBodies;
	AbstractAssembly::LabelID end = m_assembly.newLabelId();
	for (Case const& c: _switch.cases)
	{
		if (c.value)
		{
			(*this)(*c.value);
			m_assembly.setSourceLocation(c.location);
			AbstractAssembly::LabelID bodyLabel = m_assembly.newLabelId();
			caseBodies[&c] = bodyLabel;
			solAssert(m_assembly.stackHeight() == expressionHeight + 1, "");
			m_assembly.appendInstruction(solidity::dupInstruction(2));
			m_assembly.appendInstruction(solidity::Instruction::EQ);
			m_assembly.appendJumpToIf(bodyLabel);
		}
		else
			// default case
			(*this)(c.body);
	}
	m_assembly.setSourceLocation(_switch.location);
	m_assembly.appendJumpTo(end);

	size_t numCases = caseBodies.size();
	for (auto const& c: caseBodies)
	{
		m_assembly.setSourceLocation(c.first->location);
		m_assembly.appendLabel(c.second);
		(*this)(c.first->body);
		// Avoid useless "jump to next" for the last case.
		if (--numCases > 0)
		{
			m_assembly.setSourceLocation(c.first->location);
			m_assembly.appendJumpTo(end);
		}
	}

	m_assembly.setSourceLocation(_switch.location);
	m_assembly.appendLabel(end);
	m_assembly.appendInstruction(solidity::Instruction::POP);
	checkStackHeight(&_switch);
}

void CodeTransform::operator()(FunctionDefinition const& _function)
{
	solAssert(m_scope, "");
	solAssert(m_scope->identifiers.count(_function.name), "");
	Scope::Function& function = boost::get<Scope::Function>(m_scope->identifiers.at(_function.name));

	int const localStackAdjustment = m_evm15 ? 0 : 1;
	int height = localStackAdjustment;
	solAssert(m_info.scopes.at(&_function.body), "");
	Scope* varScope = m_info.scopes.at(m_info.virtualBlocks.at(&_function).get()).get();
	solAssert(varScope, "");
	for (auto const& v: _function.parameters | boost::adaptors::reversed)
	{
		auto& var = boost::get<Scope::Variable>(varScope->identifiers.at(v.name));
		m_context->variableStackHeights[&var] = height++;
	}

	m_assembly.setSourceLocation(_function.location);
	int stackHeightBefore = m_assembly.stackHeight();
	AbstractAssembly::LabelID afterFunction = m_assembly.newLabelId();

	if (m_evm15)
	{
		m_assembly.appendJumpTo(afterFunction, -stackHeightBefore);
		m_assembly.appendBeginsub(functionEntryID(_function.name, function), _function.parameters.size());
	}
	else
	{
		m_assembly.appendJumpTo(afterFunction, -stackHeightBefore + height);
		m_assembly.appendLabel(functionEntryID(_function.name, function));
	}
	m_stackAdjustment += localStackAdjustment;

	for (auto const& v: _function.returnVariables)
	{
		auto& var = boost::get<Scope::Variable>(varScope->identifiers.at(v.name));
		m_context->variableStackHeights[&var] = height++;
		// Preset stack slots for return variables to zero.
		m_assembly.appendConstant(u256(0));
	}

	try
	{
		CodeTransform(
			m_assembly,
			m_info,
			_function.body,
			m_allowStackOpt,
			m_dialect,
			m_evm15,
			m_identifierAccess,
			m_useNamedLabelsForFunctions,
			localStackAdjustment,
			m_context
		)(_function.body);
	}
	catch (StackTooDeepError const& _error)
	{
		// This exception will be re-thrown after the end of the surrounding block.
		// It enables us to see which functions compiled successfully and which did not.
		// Even if we emit actual code, add an illegal instruction to make sure that tests
		// will catch it.
		StackTooDeepError error(_error);
		if (error.functionName.empty())
			error.functionName = _function.name;
		stackError(std::move(error), height);
	}

	{
		// The stack layout here is:
		// <return label>? <arguments...> <return values...>
		// But we would like it to be:
		// <return values...> <return label>?
		// So we have to append some SWAP and POP instructions.

		// This vector holds the desired target positions of all stack slots and is
		// modified parallel to the actual stack.
		vector<int> stackLayout;
		if (!m_evm15)
			stackLayout.push_back(_function.returnVariables.size()); // Move return label to the top
		stackLayout += vector<int>(_function.parameters.size(), -1); // discard all arguments

		for (size_t i = 0; i < _function.returnVariables.size(); ++i)
			stackLayout.push_back(i); // Move return values down, but keep order.

		if (stackLayout.size() > 17)
		{
			StackTooDeepError error(_function.name, YulString{}, stackLayout.size() - 17);
			error << errinfo_comment(
				"The function " +
				_function.name.str() +
				" has " +
				to_string(stackLayout.size() - 17) +
				" parameters or return variables too many to fit the stack size."
			);
			stackError(std::move(error), m_assembly.stackHeight() - _function.parameters.size());
		}
		else
		{
			while (!stackLayout.empty() && stackLayout.back() != int(stackLayout.size() - 1))
				if (stackLayout.back() < 0)
				{
					m_assembly.appendInstruction(solidity::Instruction::POP);
					stackLayout.pop_back();
				}
				else
				{
					m_assembly.appendInstruction(swapInstruction(stackLayout.size() - stackLayout.back() - 1));
					swap(stackLayout[stackLayout.back()], stackLayout.back());
				}
			for (int i = 0; size_t(i) < stackLayout.size(); ++i)
				solAssert(i == stackLayout[i], "Error reshuffling stack.");
		}
	}
	if (m_evm15)
		m_assembly.appendReturnsub(_function.returnVariables.size(), stackHeightBefore);
	else
		m_assembly.appendJump(stackHeightBefore - _function.returnVariables.size());
	m_stackAdjustment -= localStackAdjustment;
	m_assembly.appendLabel(afterFunction);
	checkStackHeight(&_function);
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

	m_assembly.setSourceLocation(_forLoop.location);
	m_assembly.appendLabel(loopStart);

	visitExpression(*_forLoop.condition);
	m_assembly.setSourceLocation(_forLoop.location);
	m_assembly.appendInstruction(solidity::Instruction::ISZERO);
	m_assembly.appendJumpToIf(loopEnd);

	int const stackHeightBody = m_assembly.stackHeight();
	m_context->forLoopStack.emplace(Context::ForLoopLabels{ {postPart, stackHeightBody}, {loopEnd, stackHeightBody} });
	(*this)(_forLoop.body);

	m_assembly.setSourceLocation(_forLoop.location);
	m_assembly.appendLabel(postPart);

	(*this)(_forLoop.post);

	m_assembly.setSourceLocation(_forLoop.location);
	m_assembly.appendJumpTo(loopStart);
	m_assembly.appendLabel(loopEnd);

	finalizeBlock(_forLoop.pre, stackStartHeight);
	m_context->forLoopStack.pop();
	m_scope = originalScope;
	checkStackHeight(&_forLoop);
}

int CodeTransform::appendPopUntil(int _targetDepth)
{
	int const stackDiffAfter = m_assembly.stackHeight() - _targetDepth;
	for (int i = 0; i < stackDiffAfter; ++i)
		m_assembly.appendInstruction(solidity::Instruction::POP);
	return stackDiffAfter;
}

void CodeTransform::operator()(Break const& _break)
{
	yulAssert(!m_context->forLoopStack.empty(), "Invalid break-statement. Requires surrounding for-loop in code generation.");
	m_assembly.setSourceLocation(_break.location);

	Context::JumpInfo const& jump = m_context->forLoopStack.top().done;
	m_assembly.appendJumpTo(jump.label, appendPopUntil(jump.targetStackHeight));

	checkStackHeight(&_break);
}

void CodeTransform::operator()(Continue const& _continue)
{
	yulAssert(!m_context->forLoopStack.empty(), "Invalid continue-statement. Requires surrounding for-loop in code generation.");
	m_assembly.setSourceLocation(_continue.location);

	Context::JumpInfo const& jump = m_context->forLoopStack.top().post;
	m_assembly.appendJumpTo(jump.label, appendPopUntil(jump.targetStackHeight));

	checkStackHeight(&_continue);
}

void CodeTransform::operator()(Block const& _block)
{
	Scope* originalScope = m_scope;
	m_scope = m_info.scopes.at(&_block).get();

	int blockStartStackHeight = m_assembly.stackHeight();
	visitStatements(_block.statements);

	finalizeBlock(_block, blockStartStackHeight);
	m_scope = originalScope;

	if (!m_stackErrors.empty())
		BOOST_THROW_EXCEPTION(m_stackErrors.front());
}

AbstractAssembly::LabelID CodeTransform::labelFromIdentifier(Identifier const& _identifier)
{
	AbstractAssembly::LabelID label = AbstractAssembly::LabelID(-1);
	if (!m_scope->lookup(_identifier.name, Scope::NonconstVisitor(
		[=](Scope::Variable&) { solAssert(false, "Expected label"); },
		[&](Scope::Label& _label)
		{
			label = labelID(_label);
		},
		[=](Scope::Function&) { solAssert(false, "Expected label"); }
	)))
	{
		solAssert(false, "Identifier not found.");
	}
	return label;
}

AbstractAssembly::LabelID CodeTransform::labelID(Scope::Label const& _label)
{
	if (!m_context->labelIDs.count(&_label))
		m_context->labelIDs[&_label] = m_assembly.newLabelId();
	return m_context->labelIDs[&_label];
}

AbstractAssembly::LabelID CodeTransform::functionEntryID(YulString _name, Scope::Function const& _function)
{
	if (!m_context->functionEntryIDs.count(&_function))
	{
		AbstractAssembly::LabelID id =
			m_useNamedLabelsForFunctions ?
			m_assembly.namedLabel(_name.str()) :
			m_assembly.newLabelId();
		m_context->functionEntryIDs[&_function] = id;
	}
	return m_context->functionEntryIDs[&_function];
}

void CodeTransform::visitExpression(Expression const& _expression)
{
	int height = m_assembly.stackHeight();
	boost::apply_visitor(*this, _expression);
	expectDeposit(1, height);
}

void CodeTransform::visitStatements(vector<Statement> const& _statements)
{
	for (auto const& statement: _statements)
	{
		freeUnusedVariables();
		boost::apply_visitor(*this, statement);
	}
	freeUnusedVariables();
}

void CodeTransform::finalizeBlock(Block const& _block, int blockStartStackHeight)
{
	m_assembly.setSourceLocation(_block.location);

	freeUnusedVariables();

	// pop variables
	solAssert(m_info.scopes.at(&_block).get() == m_scope, "");
	for (auto const& id: m_scope->identifiers)
		if (id.second.type() == typeid(Scope::Variable))
		{
			Scope::Variable const& var = boost::get<Scope::Variable>(id.second);
			if (m_allowStackOpt)
			{
				solAssert(!m_context->variableStackHeights.count(&var), "");
				solAssert(!m_context->variableReferences.count(&var), "");
				m_stackAdjustment++;
			}
			else
				m_assembly.appendInstruction(solidity::Instruction::POP);
		}

	int deposit = m_assembly.stackHeight() - blockStartStackHeight;
	solAssert(deposit == 0, "Invalid stack height at end of block: " + to_string(deposit));
	checkStackHeight(&_block);
}

void CodeTransform::generateMultiAssignment(vector<Identifier> const& _variableNames)
{
	solAssert(m_scope, "");
	for (auto const& variableName: _variableNames | boost::adaptors::reversed)
		generateAssignment(variableName);
}

void CodeTransform::generateAssignment(Identifier const& _variableName)
{
	solAssert(m_scope, "");
	if (auto var = m_scope->lookup(_variableName.name))
	{
		Scope::Variable const& _var = boost::get<Scope::Variable>(*var);
		if (int heightDiff = variableHeightDiff(_var, _variableName.name, true))
			m_assembly.appendInstruction(solidity::swapInstruction(heightDiff - 1));
		m_assembly.appendInstruction(solidity::Instruction::POP);
		decreaseReference(_variableName.name, _var);
	}
	else
	{
		solAssert(
			m_identifierAccess.generateCode,
			"Identifier not found and no external access available."
		);
		m_identifierAccess.generateCode(_variableName, IdentifierContext::LValue, m_assembly);
	}
}

int CodeTransform::variableHeightDiff(Scope::Variable const& _var, YulString _varName, bool _forSwap)
{
	solAssert(m_context->variableStackHeights.count(&_var), "");
	int heightDiff = m_assembly.stackHeight() - m_context->variableStackHeights[&_var];
	solAssert(heightDiff > (_forSwap ? 1 : 0), "Negative stack difference for variable.");
	int limit = _forSwap ? 17 : 16;
	if (heightDiff > limit)
	{
		m_stackErrors.emplace_back(_varName, heightDiff - limit);
		m_stackErrors.back() << errinfo_comment(
			"Variable " +
			_varName.str() +
			" is " +
			to_string(heightDiff - limit) +
			" slot(s) too deep inside the stack."
		);
		BOOST_THROW_EXCEPTION(m_stackErrors.back());
	}
	return heightDiff;
}

void CodeTransform::expectDeposit(int _deposit, int _oldHeight) const
{
	solAssert(m_assembly.stackHeight() == _oldHeight + _deposit, "Invalid stack deposit.");
}

void CodeTransform::checkStackHeight(void const* _astElement) const
{
	solAssert(m_info.stackHeightInfo.count(_astElement), "Stack height for AST element not found.");
	int stackHeightInAnalysis = m_info.stackHeightInfo.at(_astElement);
	int stackHeightInCodegen = m_assembly.stackHeight() - m_stackAdjustment;
	solAssert(
		stackHeightInAnalysis == stackHeightInCodegen,
		"Stack height mismatch between analysis and code generation phase: Analysis: " +
		to_string(stackHeightInAnalysis) +
		" code gen: " +
		to_string(stackHeightInCodegen)
	);
}
