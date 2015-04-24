/*
	This file is part of cpp-ethereum.

	cpp-ethereum is free software: you can redistribute it and/or modify
	it under the terms of the GNU General Public License as published by
	the Free Software Foundation, either version 3 of the License, or
	(at your option) any later version.

	cpp-ethereum is distributed in the hope that it will be useful,
	but WITHOUT ANY WARRANTY; without even the implied warranty of
	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
	GNU General Public License for more details.

	You should have received a copy of the GNU General Public License
	along with cpp-ethereum.  If not, see <http://www.gnu.org/licenses/>.
*/
/**
 * @author Christian <c@ethdev.com>
 * @date 2014
 * Solidity compiler.
 */

#include <libsolidity/Compiler.h>
#include <algorithm>
#include <boost/range/adaptor/reversed.hpp>
#include <libevmcore/Instruction.h>
#include <libevmasm/Assembly.h>
#include <libsolidity/AST.h>
#include <libsolidity/ExpressionCompiler.h>
#include <libsolidity/CompilerUtils.h>

using namespace std;

namespace dev {
namespace solidity {

/**
 * Simple helper class to ensure that the stack height is the same at certain places in the code.
 */
class StackHeightChecker
{
public:
	StackHeightChecker(CompilerContext const& _context):
		m_context(_context), stackHeight(m_context.getStackHeight()) {}
	void check() { solAssert(m_context.getStackHeight() == stackHeight, "I sense a disturbance in the stack."); }
private:
	CompilerContext const& m_context;
	unsigned stackHeight;
};

void Compiler::compileContract(ContractDefinition const& _contract,
							   map<ContractDefinition const*, bytes const*> const& _contracts)
{
	m_context = CompilerContext(); // clear it just in case
	initializeContext(_contract, _contracts);
	appendFunctionSelector(_contract);
	set<Declaration const*> functions = m_context.getFunctionsWithoutCode();
	while (!functions.empty())
	{
		for (Declaration const* function: functions)
		{
			m_context.setStackOffset(0);
			function->accept(*this);
		}
		functions = m_context.getFunctionsWithoutCode();
	}

	// Swap the runtime context with the creation-time context
	swap(m_context, m_runtimeContext);
	initializeContext(_contract, _contracts);
	packIntoContractCreator(_contract, m_runtimeContext);
}

void Compiler::initializeContext(ContractDefinition const& _contract,
								 map<ContractDefinition const*, bytes const*> const& _contracts)
{
	m_context.setCompiledContracts(_contracts);
	m_context.setInheritanceHierarchy(_contract.getLinearizedBaseContracts());
	registerStateVariables(_contract);
	m_context.resetVisitedNodes(&_contract);
}

void Compiler::packIntoContractCreator(ContractDefinition const& _contract, CompilerContext const& _runtimeContext)
{
	// Determine the arguments that are used for the base constructors.
	std::vector<ContractDefinition const*> const& bases = _contract.getLinearizedBaseContracts();
	for (ContractDefinition const* contract: bases)
	{
		if (FunctionDefinition const* constructor = contract->getConstructor())
			for (auto const& modifier: constructor->getModifiers())
			{
				auto baseContract = dynamic_cast<ContractDefinition const*>(
					&modifier->getName()->getReferencedDeclaration());
				if (baseContract)
					if (m_baseArguments.count(baseContract->getConstructor()) == 0)
						m_baseArguments[baseContract->getConstructor()] = &modifier->getArguments();
			}

		for (ASTPointer<InheritanceSpecifier> const& base: contract->getBaseContracts())
		{
			ContractDefinition const* baseContract = dynamic_cast<ContractDefinition const*>(
						&base->getName()->getReferencedDeclaration());
			solAssert(baseContract, "");

			if (m_baseArguments.count(baseContract->getConstructor()) == 0)
				m_baseArguments[baseContract->getConstructor()] = &base->getArguments();
		}
	}
	// Initialization of state variables in base-to-derived order.
	for (ContractDefinition const* contract: boost::adaptors::reverse(bases))
		initializeStateVariables(*contract);

	if (FunctionDefinition const* constructor = _contract.getConstructor())
		appendConstructor(*constructor);
	else if (auto c = m_context.getNextConstructor(_contract))
		appendBaseConstructor(*c);

	eth::AssemblyItem sub = m_context.addSubroutine(_runtimeContext.getAssembly());
	// stack contains sub size
	m_context << eth::Instruction::DUP1 << sub << u256(0) << eth::Instruction::CODECOPY;
	m_context << u256(0) << eth::Instruction::RETURN;

	// note that we have to include the functions again because of absolute jump labels
	set<Declaration const*> functions = m_context.getFunctionsWithoutCode();
	while (!functions.empty())
	{
		for (Declaration const* function: functions)
			function->accept(*this);
		functions = m_context.getFunctionsWithoutCode();
	}
}

void Compiler::appendBaseConstructor(FunctionDefinition const& _constructor)
{
	CompilerContext::LocationSetter locationSetter(m_context, _constructor);
	FunctionType constructorType(_constructor);
	if (!constructorType.getParameterTypes().empty())
	{
		solAssert(m_baseArguments.count(&_constructor), "");
		std::vector<ASTPointer<Expression>> const* arguments = m_baseArguments[&_constructor];
		solAssert(arguments, "");
		for (unsigned i = 0; i < arguments->size(); ++i)
			compileExpression(*(arguments->at(i)), constructorType.getParameterTypes()[i]);
	}
	_constructor.accept(*this);
}

void Compiler::appendConstructor(FunctionDefinition const& _constructor)
{
	CompilerContext::LocationSetter locationSetter(m_context, _constructor);
	// copy constructor arguments from code to memory and then to stack, they are supplied after the actual program
	unsigned argumentSize = 0;
	for (ASTPointer<VariableDeclaration> const& var: _constructor.getParameters())
		argumentSize += var->getType()->getCalldataEncodedSize();

	if (argumentSize > 0)
	{
		m_context << u256(argumentSize);
		m_context.appendProgramSize();
		m_context << u256(CompilerUtils::dataStartOffset); // copy it to byte four as expected for ABI calls
		m_context << eth::Instruction::CODECOPY;
		appendCalldataUnpacker(FunctionType(_constructor).getParameterTypes(), true);
	}
	_constructor.accept(*this);
}

void Compiler::appendFunctionSelector(ContractDefinition const& _contract)
{
	map<FixedHash<4>, FunctionTypePointer> interfaceFunctions = _contract.getInterfaceFunctions();
	map<FixedHash<4>, const eth::AssemblyItem> callDataUnpackerEntryPoints;

	// retrieve the function signature hash from the calldata
	if (!interfaceFunctions.empty())
		CompilerUtils(m_context).loadFromMemory(0, IntegerType(CompilerUtils::dataStartOffset * 8), true);

	// stack now is: 1 0 <funhash>
	for (auto const& it: interfaceFunctions)
	{
		callDataUnpackerEntryPoints.insert(std::make_pair(it.first, m_context.newTag()));
		m_context << eth::dupInstruction(1) << u256(FixedHash<4>::Arith(it.first)) << eth::Instruction::EQ;
		m_context.appendConditionalJumpTo(callDataUnpackerEntryPoints.at(it.first));
	}
	if (FunctionDefinition const* fallback = _contract.getFallbackFunction())
	{
		eth::AssemblyItem returnTag = m_context.pushNewTag();
		fallback->accept(*this);
		m_context << returnTag;
		appendReturnValuePacker(FunctionType(*fallback).getReturnParameterTypes());
	}
	else
		m_context << eth::Instruction::STOP; // function not found

	for (auto const& it: interfaceFunctions)
	{
		FunctionTypePointer const& functionType = it.second;
		solAssert(functionType->hasDeclaration(), "");
		CompilerContext::LocationSetter locationSetter(m_context, functionType->getDeclaration());
		m_context << callDataUnpackerEntryPoints.at(it.first);
		eth::AssemblyItem returnTag = m_context.pushNewTag();
		appendCalldataUnpacker(functionType->getParameterTypes());
		m_context.appendJumpTo(m_context.getFunctionEntryLabel(functionType->getDeclaration()));
		m_context << returnTag;
		appendReturnValuePacker(functionType->getReturnParameterTypes());
	}
}

void Compiler::appendCalldataUnpacker(TypePointers const& _typeParameters, bool _fromMemory)
{
	// We do not check the calldata size, everything is zero-padded.
	unsigned offset(CompilerUtils::dataStartOffset);

	bigint parameterHeadEnd = offset;
	for (TypePointer const& type: _typeParameters)
		parameterHeadEnd += type->isDynamicallySized() ? 32 : type->getCalldataEncodedSize();
	solAssert(parameterHeadEnd <= numeric_limits<unsigned>::max(), "Arguments too large.");

	unsigned stackHeightOfPreviousDynamicArgument = 0;
	ArrayType const* previousDynamicType = nullptr;
	for (TypePointer const& type: _typeParameters)
	{
		switch (type->getCategory())
		{
		case Type::Category::Array:
			if (type->isDynamicallySized())
			{
				// put on stack: data_offset length
				unsigned newStackHeight = m_context.getStackHeight();
				if (previousDynamicType)
				{
					// Retrieve data start offset by adding length to start offset of previous dynamic type
					unsigned stackDepth = m_context.getStackHeight() - stackHeightOfPreviousDynamicArgument;
					solAssert(stackDepth <= 16, "Stack too deep.");
					m_context << eth::dupInstruction(stackDepth) << eth::dupInstruction(stackDepth);
					ArrayUtils(m_context).convertLengthToSize(*previousDynamicType, true);
					m_context << eth::Instruction::ADD;
				}
				else
					m_context << u256(parameterHeadEnd);
				stackHeightOfPreviousDynamicArgument = newStackHeight;
				previousDynamicType = &dynamic_cast<ArrayType const&>(*type);
				offset += CompilerUtils(m_context).loadFromMemory(offset, IntegerType(256), !_fromMemory);
			}
			else
			{
				m_context << u256(offset);
				offset += type->getCalldataEncodedSize();
			}
			break;
		default:
			solAssert(!type->isDynamicallySized(), "Unknown dynamically sized type: " + type->toString());
			offset += CompilerUtils(m_context).loadFromMemory(offset, *type, !_fromMemory, true);
		}
	}
}

void Compiler::appendReturnValuePacker(TypePointers const& _typeParameters)
{
	unsigned dataOffset = 0;
	unsigned stackDepth = 0;
	for (TypePointer const& type: _typeParameters)
		stackDepth += type->getSizeOnStack();

	for (TypePointer const& type: _typeParameters)
	{
		CompilerUtils(m_context).copyToStackTop(stackDepth, type->getSizeOnStack());
		ExpressionCompiler(m_context, m_optimize).appendTypeConversion(*type, *type, true);
		bool const c_padToWords = true;
		dataOffset += CompilerUtils(m_context).storeInMemory(dataOffset, *type, c_padToWords);
		stackDepth -= type->getSizeOnStack();
	}
	// note that the stack is not cleaned up here
	m_context << u256(dataOffset) << u256(0) << eth::Instruction::RETURN;
}

void Compiler::registerStateVariables(ContractDefinition const& _contract)
{
	for (auto const& var: ContractType(_contract).getStateVariables())
		m_context.addStateVariable(*get<0>(var), get<1>(var), get<2>(var));
}

void Compiler::initializeStateVariables(ContractDefinition const& _contract)
{
	for (ASTPointer<VariableDeclaration> const& variable: _contract.getStateVariables())
		if (variable->getValue() && !variable->isConstant())
			ExpressionCompiler(m_context, m_optimize).appendStateVariableInitialization(*variable);
}

bool Compiler::visit(VariableDeclaration const& _variableDeclaration)
{
	solAssert(_variableDeclaration.isStateVariable(), "Compiler visit to non-state variable declaration.");
	CompilerContext::LocationSetter locationSetter(m_context, _variableDeclaration);

	m_context.startFunction(_variableDeclaration);
	m_breakTags.clear();
	m_continueTags.clear();

	m_context << m_context.getFunctionEntryLabel(_variableDeclaration);
	ExpressionCompiler(m_context, m_optimize).appendStateVariableAccessor(_variableDeclaration);

	return false;
}

bool Compiler::visit(FunctionDefinition const& _function)
{
	CompilerContext::LocationSetter locationSetter(m_context, _function);

	m_context.startFunction(_function);

	// stack upon entry: [return address] [arg0] [arg1] ... [argn]
	// reserve additional slots: [retarg0] ... [retargm] [localvar0] ... [localvarp]

	unsigned parametersSize = CompilerUtils::getSizeOnStack(_function.getParameters());
	if (!_function.isConstructor())
		// adding 1 for return address.
		m_context.adjustStackOffset(parametersSize + 1);
	for (ASTPointer<VariableDeclaration const> const& variable: _function.getParameters())
	{
		m_context.addVariable(*variable, parametersSize);
		parametersSize -= variable->getType()->getSizeOnStack();
	}

	for (ASTPointer<VariableDeclaration const> const& variable: _function.getReturnParameters())
		m_context.addAndInitializeVariable(*variable);
	for (VariableDeclaration const* localVariable: _function.getLocalVariables())
		m_context.addAndInitializeVariable(*localVariable);

	if (_function.isConstructor())
		if (auto c = m_context.getNextConstructor(dynamic_cast<ContractDefinition const&>(*_function.getScope())))
			appendBaseConstructor(*c);

	m_returnTag = m_context.newTag();
	m_breakTags.clear();
	m_continueTags.clear();
	m_stackCleanupForReturn = 0;
	m_currentFunction = &_function;
	m_modifierDepth = 0;

	appendModifierOrFunctionCode();

	m_context << m_returnTag;

	// Now we need to re-shuffle the stack. For this we keep a record of the stack layout
	// that shows the target positions of the elements, where "-1" denotes that this element needs
	// to be removed from the stack.
	// Note that the fact that the return arguments are of increasing index is vital for this
	// algorithm to work.

	unsigned const c_argumentsSize = CompilerUtils::getSizeOnStack(_function.getParameters());
	unsigned const c_returnValuesSize = CompilerUtils::getSizeOnStack(_function.getReturnParameters());
	unsigned const c_localVariablesSize = CompilerUtils::getSizeOnStack(_function.getLocalVariables());

	vector<int> stackLayout;
	stackLayout.push_back(c_returnValuesSize); // target of return address
	stackLayout += vector<int>(c_argumentsSize, -1); // discard all arguments
	for (unsigned i = 0; i < c_returnValuesSize; ++i)
		stackLayout.push_back(i);
	stackLayout += vector<int>(c_localVariablesSize, -1);

	solAssert(stackLayout.size() <= 17, "Stack too deep.");
	while (stackLayout.back() != int(stackLayout.size() - 1))
		if (stackLayout.back() < 0)
		{
			m_context << eth::Instruction::POP;
			stackLayout.pop_back();
		}
		else
		{
			m_context << eth::swapInstruction(stackLayout.size() - stackLayout.back() - 1);
			swap(stackLayout[stackLayout.back()], stackLayout.back());
		}
	//@todo assert that everything is in place now

	for (ASTPointer<VariableDeclaration const> const& variable: _function.getParameters() + _function.getReturnParameters())
		m_context.removeVariable(*variable);
	for (VariableDeclaration const* localVariable: _function.getLocalVariables())
		m_context.removeVariable(*localVariable);

	m_context.adjustStackOffset(-(int)c_returnValuesSize);

	if (!_function.isConstructor())
		m_context.appendJump(eth::AssemblyItem::JumpType::OutOfFunction);
	return false;
}

bool Compiler::visit(IfStatement const& _ifStatement)
{
	StackHeightChecker checker(m_context);
	CompilerContext::LocationSetter locationSetter(m_context, _ifStatement);
	compileExpression(_ifStatement.getCondition());
	eth::AssemblyItem trueTag = m_context.appendConditionalJump();
	if (_ifStatement.getFalseStatement())
		_ifStatement.getFalseStatement()->accept(*this);
	eth::AssemblyItem endTag = m_context.appendJumpToNew();
	m_context << trueTag;
	_ifStatement.getTrueStatement().accept(*this);
	m_context << endTag;

	checker.check();
	return false;
}

bool Compiler::visit(WhileStatement const& _whileStatement)
{
	StackHeightChecker checker(m_context);
	CompilerContext::LocationSetter locationSetter(m_context, _whileStatement);
	eth::AssemblyItem loopStart = m_context.newTag();
	eth::AssemblyItem loopEnd = m_context.newTag();
	m_continueTags.push_back(loopStart);
	m_breakTags.push_back(loopEnd);

	m_context << loopStart;
	compileExpression(_whileStatement.getCondition());
	m_context << eth::Instruction::ISZERO;
	m_context.appendConditionalJumpTo(loopEnd);

	_whileStatement.getBody().accept(*this);

	m_context.appendJumpTo(loopStart);
	m_context << loopEnd;

	m_continueTags.pop_back();
	m_breakTags.pop_back();

	checker.check();
	return false;
}

bool Compiler::visit(ForStatement const& _forStatement)
{
	StackHeightChecker checker(m_context);
	CompilerContext::LocationSetter locationSetter(m_context, _forStatement);
	eth::AssemblyItem loopStart = m_context.newTag();
	eth::AssemblyItem loopEnd = m_context.newTag();
	m_continueTags.push_back(loopStart);
	m_breakTags.push_back(loopEnd);

	if (_forStatement.getInitializationExpression())
		_forStatement.getInitializationExpression()->accept(*this);

	m_context << loopStart;

	// if there is no terminating condition in for, default is to always be true
	if (_forStatement.getCondition())
	{
		compileExpression(*_forStatement.getCondition());
		m_context << eth::Instruction::ISZERO;
		m_context.appendConditionalJumpTo(loopEnd);
	}

	_forStatement.getBody().accept(*this);

	// for's loop expression if existing
	if (_forStatement.getLoopExpression())
		_forStatement.getLoopExpression()->accept(*this);

	m_context.appendJumpTo(loopStart);
	m_context << loopEnd;

	m_continueTags.pop_back();
	m_breakTags.pop_back();

	checker.check();
	return false;
}

bool Compiler::visit(Continue const& _continueStatement)
{
	CompilerContext::LocationSetter locationSetter(m_context, _continueStatement);
	if (!m_continueTags.empty())
		m_context.appendJumpTo(m_continueTags.back());
	return false;
}

bool Compiler::visit(Break const& _breakStatement)
{
	CompilerContext::LocationSetter locationSetter(m_context, _breakStatement);
	if (!m_breakTags.empty())
		m_context.appendJumpTo(m_breakTags.back());
	return false;
}

bool Compiler::visit(Return const& _return)
{
	CompilerContext::LocationSetter locationSetter(m_context, _return);
	//@todo modifications are needed to make this work with functions returning multiple values
	if (Expression const* expression = _return.getExpression())
	{
		solAssert(_return.getFunctionReturnParameters(), "Invalid return parameters pointer.");
		VariableDeclaration const& firstVariable = *_return.getFunctionReturnParameters()->getParameters().front();
		compileExpression(*expression, firstVariable.getType());
		CompilerUtils(m_context).moveToStackVariable(firstVariable);
	}
	for (unsigned i = 0; i < m_stackCleanupForReturn; ++i)
		m_context << eth::Instruction::POP;
	m_context.appendJumpTo(m_returnTag);
	m_context.adjustStackOffset(m_stackCleanupForReturn);
	return false;
}

bool Compiler::visit(VariableDeclarationStatement const& _variableDeclarationStatement)
{
	StackHeightChecker checker(m_context);
	CompilerContext::LocationSetter locationSetter(m_context, _variableDeclarationStatement);
	if (Expression const* expression = _variableDeclarationStatement.getExpression())
	{
		compileExpression(*expression, _variableDeclarationStatement.getDeclaration().getType());
		CompilerUtils(m_context).moveToStackVariable(_variableDeclarationStatement.getDeclaration());
	}
	checker.check();
	return false;
}

bool Compiler::visit(ExpressionStatement const& _expressionStatement)
{
	StackHeightChecker checker(m_context);
	CompilerContext::LocationSetter locationSetter(m_context, _expressionStatement);
	Expression const& expression = _expressionStatement.getExpression();
	compileExpression(expression);
	CompilerUtils(m_context).popStackElement(*expression.getType());
	checker.check();
	return false;
}

bool Compiler::visit(PlaceholderStatement const& _placeholderStatement)
{
	StackHeightChecker checker(m_context);
	CompilerContext::LocationSetter locationSetter(m_context, _placeholderStatement);
	++m_modifierDepth;
	appendModifierOrFunctionCode();
	--m_modifierDepth;
	checker.check();
	return true;
}

void Compiler::appendModifierOrFunctionCode()
{
	solAssert(m_currentFunction, "");
	if (m_modifierDepth >= m_currentFunction->getModifiers().size())
		m_currentFunction->getBody().accept(*this);
	else
	{
		ASTPointer<ModifierInvocation> const& modifierInvocation = m_currentFunction->getModifiers()[m_modifierDepth];

		// constructor call should be excluded
		if (dynamic_cast<ContractDefinition const*>(&modifierInvocation->getName()->getReferencedDeclaration()))
		{
			++m_modifierDepth;
			appendModifierOrFunctionCode();
			--m_modifierDepth;
			return;
		}

		ModifierDefinition const& modifier = m_context.getFunctionModifier(modifierInvocation->getName()->getName());
		CompilerContext::LocationSetter locationSetter(m_context, modifier);
		solAssert(modifier.getParameters().size() == modifierInvocation->getArguments().size(), "");
		for (unsigned i = 0; i < modifier.getParameters().size(); ++i)
		{
			m_context.addVariable(*modifier.getParameters()[i]);
			compileExpression(*modifierInvocation->getArguments()[i],
							  modifier.getParameters()[i]->getType());
		}
		for (VariableDeclaration const* localVariable: modifier.getLocalVariables())
			m_context.addAndInitializeVariable(*localVariable);

		unsigned const c_stackSurplus = CompilerUtils::getSizeOnStack(modifier.getParameters()) +
										CompilerUtils::getSizeOnStack(modifier.getLocalVariables());
		m_stackCleanupForReturn += c_stackSurplus;

		modifier.getBody().accept(*this);

		for (unsigned i = 0; i < c_stackSurplus; ++i)
			m_context << eth::Instruction::POP;
		m_stackCleanupForReturn -= c_stackSurplus;
	}
}

void Compiler::compileExpression(Expression const& _expression, TypePointer const& _targetType)
{
	ExpressionCompiler expressionCompiler(m_context, m_optimize);
	expressionCompiler.compile(_expression);
	if (_targetType)
		expressionCompiler.appendTypeConversion(*_expression.getType(), *_targetType);
}

}
}
