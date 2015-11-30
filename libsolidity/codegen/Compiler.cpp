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

#include <libsolidity/codegen/Compiler.h>
#include <algorithm>
#include <boost/range/adaptor/reversed.hpp>
#include <libevmcore/Instruction.h>
#include <libethcore/ChainOperationParams.h>
#include <libevmasm/Assembly.h>
#include <libsolidity/ast/AST.h>
#include <libsolidity/codegen/ExpressionCompiler.h>
#include <libsolidity/codegen/CompilerUtils.h>
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
		m_context(_context), stackHeight(m_context.stackHeight()) {}
	void check() { solAssert(m_context.stackHeight() == stackHeight, "I sense a disturbance in the stack."); }
private:
	CompilerContext const& m_context;
	unsigned stackHeight;
};

void Compiler::compileContract(
	ContractDefinition const& _contract,
	std::map<const ContractDefinition*, eth::Assembly const*> const& _contracts
)
{
	m_context = CompilerContext();
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

	if (_contract.isLibrary())
	{
		solAssert(m_runtimeSub != size_t(-1), "");
		m_context.injectVersionStampIntoSub(m_runtimeSub);
	}
}

void Compiler::compileClone(
	ContractDefinition const& _contract,
	map<ContractDefinition const*, eth::Assembly const*> const& _contracts
)
{
	m_context = CompilerContext(); // clear it just in case
	initializeContext(_contract, _contracts);

	appendInitAndConstructorCode(_contract);

	//@todo determine largest return size of all runtime functions
	eth::AssemblyItem runtimeSub = m_context.addSubroutine(cloneRuntime());
	solAssert(runtimeSub.data() < numeric_limits<size_t>::max(), "");
	m_runtimeSub = size_t(runtimeSub.data());

	// stack contains sub size
	m_context << eth::Instruction::DUP1 << runtimeSub << u256(0) << eth::Instruction::CODECOPY;
	m_context << u256(0) << eth::Instruction::RETURN;

	appendFunctionsWithoutCode();

	if (m_optimize)
		m_context.optimise(m_optimizeRuns);
}

eth::AssemblyItem Compiler::functionEntryLabel(FunctionDefinition const& _function) const
{
	return m_runtimeContext.functionEntryLabelIfExists(_function);
}

void Compiler::initializeContext(
	ContractDefinition const& _contract,
	map<ContractDefinition const*, eth::Assembly const*> const& _compiledContracts
)
{
	m_context.setCompiledContracts(_compiledContracts);
	m_context.setInheritanceHierarchy(_contract.annotation().linearizedBaseContracts);
	CompilerUtils(m_context).initialiseFreeMemoryPointer();
	registerStateVariables(_contract);
	m_context.resetVisitedNodes(&_contract);
}

void Compiler::appendInitAndConstructorCode(ContractDefinition const& _contract)
{
	// Determine the arguments that are used for the base constructors.
	std::vector<ContractDefinition const*> const& bases = _contract.annotation().linearizedBaseContracts;
	for (ContractDefinition const* contract: bases)
	{
		if (FunctionDefinition const* constructor = contract->constructor())
			for (auto const& modifier: constructor->modifiers())
			{
				auto baseContract = dynamic_cast<ContractDefinition const*>(
					modifier->name()->annotation().referencedDeclaration);
				if (baseContract)
					if (m_baseArguments.count(baseContract->constructor()) == 0)
						m_baseArguments[baseContract->constructor()] = &modifier->arguments();
			}

		for (ASTPointer<InheritanceSpecifier> const& base: contract->baseContracts())
		{
			ContractDefinition const* baseContract = dynamic_cast<ContractDefinition const*>(
				base->name().annotation().referencedDeclaration
			);
			solAssert(baseContract, "");

			if (m_baseArguments.count(baseContract->constructor()) == 0)
				m_baseArguments[baseContract->constructor()] = &base->arguments();
		}
	}
	// Initialization of state variables in base-to-derived order.
	for (ContractDefinition const* contract: boost::adaptors::reverse(bases))
		initializeStateVariables(*contract);

	if (FunctionDefinition const* constructor = _contract.constructor())
		appendConstructor(*constructor);
	else if (auto c = m_context.nextConstructor(_contract))
		appendBaseConstructor(*c);
}

void Compiler::packIntoContractCreator(ContractDefinition const& _contract, CompilerContext const& _runtimeContext)
{
	appendInitAndConstructorCode(_contract);

	eth::AssemblyItem runtimeSub = m_context.addSubroutine(_runtimeContext.assembly());
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
	if (!constructorType.parameterTypes().empty())
	{
		solAssert(m_baseArguments.count(&_constructor), "");
		std::vector<ASTPointer<Expression>> const* arguments = m_baseArguments[&_constructor];
		solAssert(arguments, "");
		for (unsigned i = 0; i < arguments->size(); ++i)
			compileExpression(*(arguments->at(i)), constructorType.parameterTypes()[i]);
	}
	_constructor.accept(*this);
}

void Compiler::appendConstructor(FunctionDefinition const& _constructor)
{
	CompilerContext::LocationSetter locationSetter(m_context, _constructor);
	// copy constructor arguments from code to memory and then to stack, they are supplied after the actual program
	if (!_constructor.parameters().empty())
	{
		unsigned argumentSize = 0;
		for (ASTPointer<VariableDeclaration> const& var: _constructor.parameters())
			if (var->annotation().type->isDynamicallySized())
			{
				argumentSize = 0;
				break;
			}
			else
				argumentSize += var->annotation().type->calldataEncodedSize();

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
		m_context << eth::Instruction::DUP2 << eth::Instruction::ADD;
		CompilerUtils(m_context).storeFreeMemoryPointer();
		// stack: <memptr>
		appendCalldataUnpacker(FunctionType(_constructor).parameterTypes(), true);
	}
	_constructor.accept(*this);
}

void Compiler::appendFunctionSelector(ContractDefinition const& _contract)
{
	map<FixedHash<4>, FunctionTypePointer> interfaceFunctions = _contract.interfaceFunctions();
	map<FixedHash<4>, const eth::AssemblyItem> callDataUnpackerEntryPoints;

	FunctionDefinition const* fallback = _contract.fallbackFunction();
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
		appendReturnValuePacker(FunctionType(*fallback).returnParameterTypes(), _contract.isLibrary());
	}
	else if (_contract.isLibrary())
		// Reject invalid library calls and ether sent to a library.
		m_context.appendJumpTo(m_context.errorTag());
	else
		m_context << eth::Instruction::STOP; // function not found

	for (auto const& it: interfaceFunctions)
	{
		FunctionTypePointer const& functionType = it.second;
		solAssert(functionType->hasDeclaration(), "");
		CompilerContext::LocationSetter locationSetter(m_context, functionType->declaration());
		m_context << callDataUnpackerEntryPoints.at(it.first);
		eth::AssemblyItem returnTag = m_context.pushNewTag();
		m_context << CompilerUtils::dataStartOffset;
		appendCalldataUnpacker(functionType->parameterTypes());
		m_context.appendJumpTo(m_context.functionEntryLabel(functionType->declaration()));
		m_context << returnTag;
		appendReturnValuePacker(functionType->returnParameterTypes(), _contract.isLibrary());
	}
}

void Compiler::appendCalldataUnpacker(TypePointers const& _typeParameters, bool _fromMemory)
{
	// We do not check the calldata size, everything is zero-padded

	//@todo this does not yet support nested dynamic arrays

	// Retain the offset pointer as base_offset, the point from which the data offsets are computed.
	m_context << eth::Instruction::DUP1;
	for (TypePointer const& parameterType: _typeParameters)
	{
		// stack: v1 v2 ... v(k-1) base_offset current_offset
		TypePointer type = parameterType->decodingType();
		if (type->category() == Type::Category::Array)
		{
			auto const& arrayType = dynamic_cast<ArrayType const&>(*type);
			solAssert(!arrayType.baseType()->isDynamicallySized(), "Nested arrays not yet implemented.");
			if (_fromMemory)
			{
				solAssert(
					arrayType.baseType()->isValueType(),
					"Nested memory arrays not yet implemented here."
				);
				// @todo If base type is an array or struct, it is still calldata-style encoded, so
				// we would have to convert it like below.
				solAssert(arrayType.location() == DataLocation::Memory, "");
				if (arrayType.isDynamicallySized())
				{
					// compute data pointer
					m_context << eth::Instruction::DUP1 << eth::Instruction::MLOAD;
					m_context << eth::Instruction::DUP3 << eth::Instruction::ADD;
					m_context << eth::Instruction::SWAP2 << eth::Instruction::SWAP1;
					m_context << u256(0x20) << eth::Instruction::ADD;
				}
				else
				{
					m_context << eth::Instruction::DUP1;
					m_context << u256(arrayType.calldataEncodedSize(true)) << eth::Instruction::ADD;
				}
			}
			else
			{
				// first load from calldata and potentially convert to memory if arrayType is memory
				TypePointer calldataType = arrayType.copyForLocation(DataLocation::CallData, false);
				if (calldataType->isDynamicallySized())
				{
					// put on stack: data_pointer length
					CompilerUtils(m_context).loadFromMemoryDynamic(IntegerType(256), !_fromMemory);
					// stack: base_offset data_offset next_pointer
					m_context << eth::Instruction::SWAP1 << eth::Instruction::DUP3 << eth::Instruction::ADD;
					// stack: base_offset next_pointer data_pointer
					// retrieve length
					CompilerUtils(m_context).loadFromMemoryDynamic(IntegerType(256), !_fromMemory, true);
					// stack: base_offset next_pointer length data_pointer
					m_context << eth::Instruction::SWAP2;
					// stack: base_offset data_pointer length next_pointer
				}
				else
				{
					// leave the pointer on the stack
					m_context << eth::Instruction::DUP1;
					m_context << u256(calldataType->calldataEncodedSize()) << eth::Instruction::ADD;
				}
				if (arrayType.location() == DataLocation::Memory)
				{
					// stack: base_offset calldata_ref [length] next_calldata
					// copy to memory
					// move calldata type up again
					CompilerUtils(m_context).moveIntoStack(calldataType->sizeOnStack());
					CompilerUtils(m_context).convertType(*calldataType, arrayType);
					// fetch next pointer again
					CompilerUtils(m_context).moveToStackTop(arrayType.sizeOnStack());
				}
				// move base_offset up
				CompilerUtils(m_context).moveToStackTop(1 + arrayType.sizeOnStack());
				m_context << eth::Instruction::SWAP1;
			}
		}
		else
		{
			solAssert(!type->isDynamicallySized(), "Unknown dynamically sized type: " + type->toString());
			CompilerUtils(m_context).loadFromMemoryDynamic(*type, !_fromMemory, true);
			CompilerUtils(m_context).moveToStackTop(1 + type->sizeOnStack());
			m_context << eth::Instruction::SWAP1;
		}
		// stack: v1 v2 ... v(k-1) v(k) base_offset mem_offset
	}
	m_context << eth::Instruction::POP << eth::Instruction::POP;
}

void Compiler::appendReturnValuePacker(TypePointers const& _typeParameters, bool _isLibrary)
{
	CompilerUtils utils(m_context);
	if (_typeParameters.empty())
		m_context << eth::Instruction::STOP;
	else
	{
		utils.fetchFreeMemoryPointer();
		//@todo optimization: if we return a single memory array, there should be enough space before
		// its data to add the needed parts and we avoid a memory copy.
		utils.encodeToMemory(_typeParameters, _typeParameters, true, false, _isLibrary);
		utils.toSizeAfterFreeMemoryPointer();
		m_context << eth::Instruction::RETURN;
	}
}

void Compiler::registerStateVariables(ContractDefinition const& _contract)
{
	for (auto const& var: ContractType(_contract).stateVariables())
		m_context.addStateVariable(*get<0>(var), get<1>(var), get<2>(var));
}

void Compiler::initializeStateVariables(ContractDefinition const& _contract)
{
	for (VariableDeclaration const* variable: _contract.stateVariables())
		if (variable->value() && !variable->isConstant())
			ExpressionCompiler(m_context, m_optimize).appendStateVariableInitialization(*variable);
}

bool Compiler::visit(VariableDeclaration const& _variableDeclaration)
{
	solAssert(_variableDeclaration.isStateVariable(), "Compiler visit to non-state variable declaration.");
	CompilerContext::LocationSetter locationSetter(m_context, _variableDeclaration);

	m_context.startFunction(_variableDeclaration);
	m_breakTags.clear();
	m_continueTags.clear();

	if (_variableDeclaration.isConstant())
		ExpressionCompiler(m_context, m_optimize).appendConstStateVariableAccessor(_variableDeclaration);
	else
		ExpressionCompiler(m_context, m_optimize).appendStateVariableAccessor(_variableDeclaration);

	return false;
}

bool Compiler::visit(FunctionDefinition const& _function)
{
	CompilerContext::LocationSetter locationSetter(m_context, _function);

	m_context.startFunction(_function);

	// stack upon entry: [return address] [arg0] [arg1] ... [argn]
	// reserve additional slots: [retarg0] ... [retargm] [localvar0] ... [localvarp]

	unsigned parametersSize = CompilerUtils::sizeOnStack(_function.parameters());
	if (!_function.isConstructor())
		// adding 1 for return address.
		m_context.adjustStackOffset(parametersSize + 1);
	for (ASTPointer<VariableDeclaration const> const& variable: _function.parameters())
	{
		m_context.addVariable(*variable, parametersSize);
		parametersSize -= variable->annotation().type->sizeOnStack();
	}

	for (ASTPointer<VariableDeclaration const> const& variable: _function.returnParameters())
		appendStackVariableInitialisation(*variable);
	for (VariableDeclaration const* localVariable: _function.localVariables())
		appendStackVariableInitialisation(*localVariable);

	if (_function.isConstructor())
		if (auto c = m_context.nextConstructor(dynamic_cast<ContractDefinition const&>(*_function.scope())))
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

	unsigned const c_argumentsSize = CompilerUtils::sizeOnStack(_function.parameters());
	unsigned const c_returnValuesSize = CompilerUtils::sizeOnStack(_function.returnParameters());
	unsigned const c_localVariablesSize = CompilerUtils::sizeOnStack(_function.localVariables());

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

	for (ASTPointer<VariableDeclaration const> const& variable: _function.parameters() + _function.returnParameters())
		m_context.removeVariable(*variable);
	for (VariableDeclaration const* localVariable: _function.localVariables())
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
	compileExpression(_ifStatement.condition());
	m_context << eth::Instruction::ISZERO;
	eth::AssemblyItem falseTag = m_context.appendConditionalJump();
	eth::AssemblyItem endTag = falseTag;
	_ifStatement.trueStatement().accept(*this);
	if (_ifStatement.falseStatement())
	{
		endTag = m_context.appendJumpToNew();
		m_context << falseTag;
		_ifStatement.falseStatement()->accept(*this);
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
	compileExpression(_whileStatement.condition());
	m_context << eth::Instruction::ISZERO;
	m_context.appendConditionalJumpTo(loopEnd);

	_whileStatement.body().accept(*this);

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

	if (_forStatement.initializationExpression())
		_forStatement.initializationExpression()->accept(*this);

	m_context << loopStart;

	// if there is no terminating condition in for, default is to always be true
	if (_forStatement.condition())
	{
		compileExpression(*_forStatement.condition());
		m_context << eth::Instruction::ISZERO;
		m_context.appendConditionalJumpTo(loopEnd);
	}

	_forStatement.body().accept(*this);

	m_context << loopNext;

	// for's loop expression if existing
	if (_forStatement.loopExpression())
		_forStatement.loopExpression()->accept(*this);

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
	if (Expression const* expression = _return.expression())
	{
		solAssert(_return.annotation().functionReturnParameters, "Invalid return parameters pointer.");
		vector<ASTPointer<VariableDeclaration>> const& returnParameters =
			_return.annotation().functionReturnParameters->parameters();
		TypePointers types;
		for (auto const& retVariable: returnParameters)
			types.push_back(retVariable->annotation().type);

		TypePointer expectedType;
		if (expression->annotation().type->category() == Type::Category::Tuple || types.size() != 1)
			expectedType = make_shared<TupleType>(types);
		else
			expectedType = types.front();
		compileExpression(*expression, expectedType);

		for (auto const& retVariable: boost::adaptors::reverse(returnParameters))
			CompilerUtils(m_context).moveToStackVariable(*retVariable);
	}
	for (unsigned i = 0; i < m_stackCleanupForReturn; ++i)
		m_context << eth::Instruction::POP;
	m_context.appendJumpTo(m_returnTag);
	m_context.adjustStackOffset(m_stackCleanupForReturn);
	return false;
}

bool Compiler::visit(Throw const& _throw)
{
	CompilerContext::LocationSetter locationSetter(m_context, _throw);
	m_context.appendJumpTo(m_context.errorTag());
	return false;
}

bool Compiler::visit(VariableDeclarationStatement const& _variableDeclarationStatement)
{
	StackHeightChecker checker(m_context);
	CompilerContext::LocationSetter locationSetter(m_context, _variableDeclarationStatement);
	if (Expression const* expression = _variableDeclarationStatement.initialValue())
	{
		CompilerUtils utils(m_context);
		compileExpression(*expression);
		TypePointers valueTypes;
		if (auto tupleType = dynamic_cast<TupleType const*>(expression->annotation().type.get()))
			valueTypes = tupleType->components();
		else
			valueTypes = TypePointers{expression->annotation().type};
		auto const& assignments = _variableDeclarationStatement.annotation().assignments;
		solAssert(assignments.size() == valueTypes.size(), "");
		for (size_t i = 0; i < assignments.size(); ++i)
		{
			size_t j = assignments.size() - i - 1;
			solAssert(!!valueTypes[j], "");
			VariableDeclaration const* varDecl = assignments[j];
			if (!varDecl)
				utils.popStackElement(*valueTypes[j]);
			else
			{
				utils.convertType(*valueTypes[j], *varDecl->annotation().type);
				utils.moveToStackVariable(*varDecl);
			}
		}
	}
	checker.check();
	return false;
}

bool Compiler::visit(ExpressionStatement const& _expressionStatement)
{
	StackHeightChecker checker(m_context);
	CompilerContext::LocationSetter locationSetter(m_context, _expressionStatement);
	Expression const& expression = _expressionStatement.expression();
	compileExpression(expression);
	CompilerUtils(m_context).popStackElement(*expression.annotation().type);
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
	set<Declaration const*> functions = m_context.functionsWithoutCode();
	while (!functions.empty())
	{
		for (Declaration const* function: functions)
		{
			m_context.setStackOffset(0);
			function->accept(*this);
		}
		functions = m_context.functionsWithoutCode();
	}
}

void Compiler::appendModifierOrFunctionCode()
{
	solAssert(m_currentFunction, "");
	if (m_modifierDepth >= m_currentFunction->modifiers().size())
		m_currentFunction->body().accept(*this);
	else
	{
		ASTPointer<ModifierInvocation> const& modifierInvocation = m_currentFunction->modifiers()[m_modifierDepth];

		// constructor call should be excluded
		if (dynamic_cast<ContractDefinition const*>(modifierInvocation->name()->annotation().referencedDeclaration))
		{
			++m_modifierDepth;
			appendModifierOrFunctionCode();
			--m_modifierDepth;
			return;
		}

		ModifierDefinition const& modifier = m_context.functionModifier(modifierInvocation->name()->name());
		CompilerContext::LocationSetter locationSetter(m_context, modifier);
		solAssert(modifier.parameters().size() == modifierInvocation->arguments().size(), "");
		for (unsigned i = 0; i < modifier.parameters().size(); ++i)
		{
			m_context.addVariable(*modifier.parameters()[i]);
			compileExpression(
				*modifierInvocation->arguments()[i],
				modifier.parameters()[i]->annotation().type
			);
		}
		for (VariableDeclaration const* localVariable: modifier.localVariables())
			appendStackVariableInitialisation(*localVariable);

		unsigned const c_stackSurplus = CompilerUtils::sizeOnStack(modifier.parameters()) +
										CompilerUtils::sizeOnStack(modifier.localVariables());
		m_stackCleanupForReturn += c_stackSurplus;

		modifier.body().accept(*this);

		for (unsigned i = 0; i < c_stackSurplus; ++i)
			m_context << eth::Instruction::POP;
		m_stackCleanupForReturn -= c_stackSurplus;
	}
}

void Compiler::appendStackVariableInitialisation(VariableDeclaration const& _variable)
{
	CompilerContext::LocationSetter location(m_context, _variable);
	m_context.addVariable(_variable);
	CompilerUtils(m_context).pushZeroValue(*_variable.annotation().type);
}

void Compiler::compileExpression(Expression const& _expression, TypePointer const& _targetType)
{
	ExpressionCompiler expressionCompiler(m_context, m_optimize);
	expressionCompiler.compile(_expression);
	if (_targetType)
		CompilerUtils(m_context).convertType(*_expression.annotation().type, *_targetType);
}

eth::Assembly Compiler::cloneRuntime()
{
	eth::EVMSchedule schedule;
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
	a << u256(schedule.callGas + schedule.callValueTransferGas + 10) << eth::Instruction::GAS << eth::Instruction::SUB;
	a << eth::Instruction::CALLCODE;
	//Propagate error condition (if CALLCODE pushes 0 on stack).
	a << eth::Instruction::ISZERO;
	a.appendJumpI(a.errorTag());
	//@todo adjust for larger return values, make this dynamic.
	a << u256(0x20) << u256(0) << eth::Instruction::RETURN;
	return a;
}
