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
#include <libevmcore/Params.h>
#include <libsolidity/AST.h>
#include <libsolidity/ExpressionCompiler.h>
#include <libsolidity/CompilerUtils.h>

using namespace std;
using namespace dev;
using namespace dev::solidity;

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
	{
		CompilerContext::LocationSetter locationSetterRunTime(m_context, _contract);
		initializeContext(_contract, _contracts);
		appendFunctionSelector(_contract);
		appendFunctionsWithoutCode();
	}

	// Swap the runtime context with the creation-time context
	swap(m_context, m_runtimeContext);
	CompilerContext::LocationSetter locationSetterCreationTime(m_context, _contract);
	initializeContext(_contract, _contracts);
	packIntoContractCreator(_contract, m_runtimeContext);
	if (m_optimize)
		m_context.optimise(m_optimizeRuns);
}

void Compiler::compileClone(
	ContractDefinition const& _contract,
	map<ContractDefinition const*, bytes const*> const& _contracts
)
{
	m_context = CompilerContext(); // clear it just in case
	initializeContext(_contract, _contracts);

	appendInitAndConstructorCode(_contract);

	//@todo determine largest return size of all runtime functions
	eth::AssemblyItem runtimeSub = m_context.addSubroutine(getCloneRuntime());
	solAssert(runtimeSub.data() < numeric_limits<size_t>::max(), "");
	m_runtimeSub = size_t(runtimeSub.data());

	// stack contains sub size
	m_context << eth::Instruction::DUP1 << runtimeSub << u256(0) << eth::Instruction::CODECOPY;
	m_context << u256(0) << eth::Instruction::RETURN;

	appendFunctionsWithoutCode();

	if (m_optimize)
		m_context.optimise(m_optimizeRuns);
}

eth::AssemblyItem Compiler::getFunctionEntryLabel(FunctionDefinition const& _function) const
{
	return m_runtimeContext.getFunctionEntryLabelIfExists(_function);
}

void Compiler::initializeContext(ContractDefinition const& _contract,
								 map<ContractDefinition const*, bytes const*> const& _contracts)
{
	CompilerUtils(m_context).initialiseFreeMemoryPointer();
	m_context.setCompiledContracts(_contracts);
	m_context.setInheritanceHierarchy(_contract.getLinearizedBaseContracts());
	registerStateVariables(_contract);
	m_context.resetVisitedNodes(&_contract);
}

void Compiler::appendInitAndConstructorCode(ContractDefinition const& _contract)
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
}

void Compiler::packIntoContractCreator(ContractDefinition const& _contract, CompilerContext const& _runtimeContext)
{
	appendInitAndConstructorCode(_contract);

	eth::AssemblyItem runtimeSub = m_context.addSubroutine(_runtimeContext.getAssembly());
	solAssert(runtimeSub.data() < numeric_limits<size_t>::max(), "");
	m_runtimeSub = size_t(runtimeSub.data());

	// stack contains sub size
	m_context << eth::Instruction::DUP1 << runtimeSub << u256(0) << eth::Instruction::CODECOPY;
	m_context << u256(0) << eth::Instruction::RETURN;

	// note that we have to include the functions again because of absolute jump labels
	appendFunctionsWithoutCode();
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
	if (!_constructor.getParameters().empty())
	{
		unsigned argumentSize = 0;
		for (ASTPointer<VariableDeclaration> const& var: _constructor.getParameters())
			if (var->getType()->isDynamicallySized())
			{
				argumentSize = 0;
				break;
			}
			else
				argumentSize += var->getType()->getCalldataEncodedSize();

		CompilerUtils(m_context).fetchFreeMemoryPointer();
		if (argumentSize == 0)
		{
			// argument size is dynamic, use CODESIZE to determine it
			m_context.appendProgramSize(); // program itself
			// CODESIZE is program plus manually added arguments
			m_context << eth::Instruction::CODESIZE << eth::Instruction::SUB;
		}
		else
			m_context << u256(argumentSize);
		// stack: <memptr> <argument size>
		m_context << eth::Instruction::DUP1;
		m_context.appendProgramSize();
		m_context << eth::Instruction::DUP4 << eth::Instruction::CODECOPY;
		m_context << eth::Instruction::ADD;
		CompilerUtils(m_context).storeFreeMemoryPointer();
		appendCalldataUnpacker(
			FunctionType(_constructor).getParameterTypes(),
			true,
			CompilerUtils::freeMemoryPointer + 0x20
		);
	}
	_constructor.accept(*this);
}

void Compiler::appendFunctionSelector(ContractDefinition const& _contract)
{
	map<FixedHash<4>, FunctionTypePointer> interfaceFunctions = _contract.getInterfaceFunctions();
	map<FixedHash<4>, const eth::AssemblyItem> callDataUnpackerEntryPoints;

	FunctionDefinition const* fallback = _contract.getFallbackFunction();
	eth::AssemblyItem notFound = m_context.newTag();
	// shortcut messages without data if we have many functions in order to be able to receive
	// ether with constant gas
	if (interfaceFunctions.size() > 5 || fallback)
	{
		m_context << eth::Instruction::CALLDATASIZE << eth::Instruction::ISZERO;
		m_context.appendConditionalJumpTo(notFound);
	}

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
	m_context.appendJumpTo(notFound);

	m_context << notFound;
	if (fallback)
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

void Compiler::appendCalldataUnpacker(
	TypePointers const& _typeParameters,
	bool _fromMemory,
	u256 _startOffset
)
{
	// We do not check the calldata size, everything is zero-paddedd

	//@todo this does not yet support nested dynamic arrays

	if (_startOffset == u256(-1))
		_startOffset = u256(CompilerUtils::dataStartOffset);

	m_context << _startOffset;
	for (TypePointer const& type: _typeParameters)
	{
		// stack: v1 v2 ... v(k-1) mem_offset
		switch (type->getCategory())
		{
		case Type::Category::Array:
		{
			auto const& arrayType = dynamic_cast<ArrayType const&>(*type);
			solAssert(arrayType.location() != DataLocation::Storage, "");
			solAssert(!arrayType.getBaseType()->isDynamicallySized(), "Nested arrays not yet implemented.");
			if (_fromMemory)
			{
				solAssert(
					arrayType.getBaseType()->isValueType(),
					"Nested memory arrays not yet implemented here."
				);
				// @todo If base type is an array or struct, it is still calldata-style encoded, so
				// we would have to convert it like below.
				solAssert(arrayType.location() == DataLocation::Memory, "");
				// compute data pointer
				m_context << eth::Instruction::DUP1 << eth::Instruction::MLOAD;
				//@todo once we support nested arrays, this offset needs to be dynamic.
				m_context << _startOffset << eth::Instruction::ADD;
				m_context << eth::Instruction::SWAP1 << u256(0x20) << eth::Instruction::ADD;
			}
			else
			{
				// first load from calldata and potentially convert to memory if arrayType is memory
				TypePointer calldataType = arrayType.copyForLocation(DataLocation::CallData, false);
				if (calldataType->isDynamicallySized())
				{
					// put on stack: data_pointer length
					CompilerUtils(m_context).loadFromMemoryDynamic(IntegerType(256), !_fromMemory);
					// stack: data_offset next_pointer
					//@todo once we support nested arrays, this offset needs to be dynamic.
					m_context << eth::Instruction::SWAP1 << _startOffset << eth::Instruction::ADD;
					// stack: next_pointer data_pointer
					// retrieve length
					CompilerUtils(m_context).loadFromMemoryDynamic(IntegerType(256), !_fromMemory, true);
					// stack: next_pointer length data_pointer
					m_context << eth::Instruction::SWAP2;
				}
				else
				{
					// leave the pointer on the stack
					m_context << eth::Instruction::DUP1;
					m_context << u256(calldataType->getCalldataEncodedSize()) << eth::Instruction::ADD;
				}
				if (arrayType.location() == DataLocation::Memory)
				{
					// stack: calldata_ref [length] next_calldata
					// copy to memory
					// move calldata type up again
					CompilerUtils(m_context).moveIntoStack(calldataType->getSizeOnStack());
					CompilerUtils(m_context).convertType(*calldataType, arrayType);
					// fetch next pointer again
					CompilerUtils(m_context).moveToStackTop(arrayType.getSizeOnStack());
				}
			}
			break;
		}
		default:
			solAssert(!type->isDynamicallySized(), "Unknown dynamically sized type: " + type->toString());
			CompilerUtils(m_context).loadFromMemoryDynamic(*type, !_fromMemory, true);
		}
	}
	m_context << eth::Instruction::POP;
}

void Compiler::appendReturnValuePacker(TypePointers const& _typeParameters)
{
	CompilerUtils utils(m_context);
	if (_typeParameters.empty())
		m_context << eth::Instruction::STOP;
	else
	{
		utils.fetchFreeMemoryPointer();
		//@todo optimization: if we return a single memory array, there should be enough space before
		// its data to add the needed parts and we avoid a memory copy.
		utils.encodeToMemory(_typeParameters, _typeParameters);
		utils.toSizeAfterFreeMemoryPointer();
		m_context << eth::Instruction::RETURN;
	}
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
		appendStackVariableInitialisation(*variable);
	for (VariableDeclaration const* localVariable: _function.getLocalVariables())
		appendStackVariableInitialisation(*localVariable);

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

	solAssert(stackLayout.size() <= 17, "Stack too deep, try removing local variables.");
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
	m_context << eth::Instruction::ISZERO;
	eth::AssemblyItem falseTag = m_context.appendConditionalJump();
	eth::AssemblyItem endTag = falseTag;
	_ifStatement.getTrueStatement().accept(*this);
	if (_ifStatement.getFalseStatement())
	{
		endTag = m_context.appendJumpToNew();
		m_context << falseTag;
		_ifStatement.getFalseStatement()->accept(*this);
	}
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
	eth::AssemblyItem loopNext = m_context.newTag();
	m_continueTags.push_back(loopNext);
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

	m_context << loopNext;

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

void Compiler::appendFunctionsWithoutCode()
{
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
			appendStackVariableInitialisation(*localVariable);

		unsigned const c_stackSurplus = CompilerUtils::getSizeOnStack(modifier.getParameters()) +
										CompilerUtils::getSizeOnStack(modifier.getLocalVariables());
		m_stackCleanupForReturn += c_stackSurplus;

		modifier.getBody().accept(*this);

		for (unsigned i = 0; i < c_stackSurplus; ++i)
			m_context << eth::Instruction::POP;
		m_stackCleanupForReturn -= c_stackSurplus;
	}
}

void Compiler::appendStackVariableInitialisation(VariableDeclaration const& _variable)
{
	CompilerContext::LocationSetter location(m_context, _variable);
	m_context.addVariable(_variable);
	CompilerUtils(m_context).pushZeroValue(*_variable.getType());
}

void Compiler::compileExpression(Expression const& _expression, TypePointer const& _targetType)
{
	ExpressionCompiler expressionCompiler(m_context, m_optimize);
	expressionCompiler.compile(_expression);
	if (_targetType)
		CompilerUtils(m_context).convertType(*_expression.getType(), *_targetType);
}

eth::Assembly Compiler::getCloneRuntime()
{
	eth::Assembly a;
	a << eth::Instruction::CALLDATASIZE;
	a << u256(0) << eth::Instruction::DUP1 << eth::Instruction::CALLDATACOPY;
	//@todo adjust for larger return values, make this dynamic.
	a << u256(0x20) << u256(0) << eth::Instruction::CALLDATASIZE;
	// unfortunately, we have to send the value again, so that CALLVALUE returns the correct value
	// in the callcoded contract.
	a << u256(0) << eth::Instruction::CALLVALUE;
	// this is the address which has to be substituted by the linker.
	//@todo implement as special "marker" AssemblyItem.
	a << u256("0xcafecafecafecafecafecafecafecafecafecafe");
	a << u256(eth::c_callGas + eth::c_callValueTransferGas + 10) << eth::Instruction::GAS << eth::Instruction::SUB;
	a << eth::Instruction::CALLCODE;
	//@todo adjust for larger return values, make this dynamic.
	a << u256(0x20) << u256(0) << eth::Instruction::RETURN;
	return a;
}
