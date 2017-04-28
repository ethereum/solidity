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
 * @author Christian <c@ethdev.com>
 * @date 2014
 * Solidity compiler.
 */

#include <libsolidity/codegen/ContractCompiler.h>
#include <algorithm>
#include <boost/range/adaptor/reversed.hpp>
#include <libevmasm/Instruction.h>
#include <libevmasm/Assembly.h>
#include <libevmasm/GasMeter.h>
#include <libsolidity/inlineasm/AsmCodeGen.h>
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
	void check() { solAssert(m_context.stackHeight() == stackHeight, std::string("I sense a disturbance in the stack: ") + std::to_string(m_context.stackHeight()) + " vs " + std::to_string(stackHeight)); }
private:
	CompilerContext const& m_context;
	unsigned stackHeight;
};

void ContractCompiler::compileContract(
	ContractDefinition const& _contract,
	std::map<const ContractDefinition*, eth::Assembly const*> const& _contracts
)
{
	CompilerContext::LocationSetter locationSetter(m_context, _contract);
	initializeContext(_contract, _contracts);
	appendFunctionSelector(_contract);
	appendMissingFunctions();
}

size_t ContractCompiler::compileConstructor(
	ContractDefinition const& _contract,
	std::map<const ContractDefinition*, eth::Assembly const*> const& _contracts
)
{
	CompilerContext::LocationSetter locationSetter(m_context, _contract);
	initializeContext(_contract, _contracts);
	return packIntoContractCreator(_contract);
}

size_t ContractCompiler::compileClone(
	ContractDefinition const& _contract,
	map<ContractDefinition const*, eth::Assembly const*> const& _contracts
)
{
	initializeContext(_contract, _contracts);

	appendInitAndConstructorCode(_contract);

	//@todo determine largest return size of all runtime functions
	auto runtimeSub = m_context.addSubroutine(cloneRuntime());

	// stack contains sub size
	m_context << Instruction::DUP1 << runtimeSub << u256(0) << Instruction::CODECOPY;
	m_context << u256(0) << Instruction::RETURN;

	appendMissingFunctions();

	return size_t(runtimeSub.data());
}

void ContractCompiler::initializeContext(
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

void ContractCompiler::appendCallValueCheck()
{
	// Throw if function is not payable but call contained ether.
	m_context << Instruction::CALLVALUE;
	m_context.appendConditionalInvalid();
}

void ContractCompiler::appendInitAndConstructorCode(ContractDefinition const& _contract)
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
	else
		appendCallValueCheck();
}

size_t ContractCompiler::packIntoContractCreator(ContractDefinition const& _contract)
{
	solAssert(!!m_runtimeCompiler, "");

	appendInitAndConstructorCode(_contract);

	// We jump to the deploy routine because we first have to append all missing functions,
	// which can cause further functions to be added to the runtime context.
	eth::AssemblyItem deployRoutine = m_context.appendJumpToNew();

	// We have to include copies of functions in the construction time and runtime context
	// because of absolute jumps.
	appendMissingFunctions();
	m_runtimeCompiler->appendMissingFunctions();

	m_context << deployRoutine;

	solAssert(m_context.runtimeSub() != size_t(-1), "Runtime sub not registered");
	m_context.pushSubroutineSize(m_context.runtimeSub());
	m_context << Instruction::DUP1;
	m_context.pushSubroutineOffset(m_context.runtimeSub());
	m_context << u256(0) << Instruction::CODECOPY;
	m_context << u256(0) << Instruction::RETURN;

	return m_context.runtimeSub();
}

void ContractCompiler::appendBaseConstructor(FunctionDefinition const& _constructor)
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

void ContractCompiler::appendConstructor(FunctionDefinition const& _constructor)
{
	CompilerContext::LocationSetter locationSetter(m_context, _constructor);
	if (!_constructor.isPayable())
		appendCallValueCheck();

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
			m_context << Instruction::CODESIZE << Instruction::SUB;
		}
		else
			m_context << u256(argumentSize);
		// stack: <memptr> <argument size>
		m_context << Instruction::DUP1;
		m_context.appendProgramSize();
		m_context << Instruction::DUP4 << Instruction::CODECOPY;
		m_context << Instruction::DUP2 << Instruction::ADD;
		CompilerUtils(m_context).storeFreeMemoryPointer();
		// stack: <memptr>
		appendCalldataUnpacker(FunctionType(_constructor).parameterTypes(), true);
	}
	_constructor.accept(*this);
}

void ContractCompiler::appendFunctionSelector(ContractDefinition const& _contract)
{
	map<FixedHash<4>, FunctionTypePointer> interfaceFunctions = _contract.interfaceFunctions();
	map<FixedHash<4>, const eth::AssemblyItem> callDataUnpackerEntryPoints;

	FunctionDefinition const* fallback = _contract.fallbackFunction();
	eth::AssemblyItem notFound = m_context.newTag();
	// shortcut messages without data if we have many functions in order to be able to receive
	// ether with constant gas
	if (interfaceFunctions.size() > 5 || fallback)
	{
		m_context << Instruction::CALLDATASIZE << Instruction::ISZERO;
		m_context.appendConditionalJumpTo(notFound);
	}

	// retrieve the function signature hash from the calldata
	if (!interfaceFunctions.empty())
		CompilerUtils(m_context).loadFromMemory(0, IntegerType(CompilerUtils::dataStartOffset * 8), true);

	// stack now is: 1 0 <funhash>
	for (auto const& it: interfaceFunctions)
	{
		callDataUnpackerEntryPoints.insert(std::make_pair(it.first, m_context.newTag()));
		m_context << dupInstruction(1) << u256(FixedHash<4>::Arith(it.first)) << Instruction::EQ;
		m_context.appendConditionalJumpTo(callDataUnpackerEntryPoints.at(it.first));
	}
	m_context.appendJumpTo(notFound);

	m_context << notFound;
	if (fallback)
	{
		if (!fallback->isPayable())
			appendCallValueCheck();

		eth::AssemblyItem returnTag = m_context.pushNewTag();
		fallback->accept(*this);
		m_context << returnTag;
		appendReturnValuePacker(FunctionType(*fallback).returnParameterTypes(), _contract.isLibrary());
	}
	else
		m_context.appendInvalid();

	for (auto const& it: interfaceFunctions)
	{
		FunctionTypePointer const& functionType = it.second;
		solAssert(functionType->hasDeclaration(), "");
		CompilerContext::LocationSetter locationSetter(m_context, functionType->declaration());

		m_context << callDataUnpackerEntryPoints.at(it.first);
		// We have to allow this for libraries, because value of the previous
		// call is still visible in the delegatecall.
		if (!functionType->isPayable() && !_contract.isLibrary())
			appendCallValueCheck();

		eth::AssemblyItem returnTag = m_context.pushNewTag();
		m_context << CompilerUtils::dataStartOffset;
		appendCalldataUnpacker(functionType->parameterTypes());
		m_context.appendJumpTo(m_context.functionEntryLabel(functionType->declaration()));
		m_context << returnTag;
		appendReturnValuePacker(functionType->returnParameterTypes(), _contract.isLibrary());
	}
}

void ContractCompiler::appendCalldataUnpacker(TypePointers const& _typeParameters, bool _fromMemory)
{
	// We do not check the calldata size, everything is zero-padded

	//@todo this does not yet support nested dynamic arrays

	// Retain the offset pointer as base_offset, the point from which the data offsets are computed.
	m_context << Instruction::DUP1;
	for (TypePointer const& parameterType: _typeParameters)
	{
		// stack: v1 v2 ... v(k-1) base_offset current_offset
		TypePointer type = parameterType->decodingType();
		solAssert(type, "No decoding type found.");
		if (type->category() == Type::Category::Array)
		{
			auto const& arrayType = dynamic_cast<ArrayType const&>(*type);
			solUnimplementedAssert(!arrayType.baseType()->isDynamicallySized(), "Nested arrays not yet implemented.");
			if (_fromMemory)
			{
				solUnimplementedAssert(
					arrayType.baseType()->isValueType(),
					"Nested memory arrays not yet implemented here."
				);
				// @todo If base type is an array or struct, it is still calldata-style encoded, so
				// we would have to convert it like below.
				solAssert(arrayType.location() == DataLocation::Memory, "");
				if (arrayType.isDynamicallySized())
				{
					// compute data pointer
					m_context << Instruction::DUP1 << Instruction::MLOAD;
					m_context << Instruction::DUP3 << Instruction::ADD;
					m_context << Instruction::SWAP2 << Instruction::SWAP1;
					m_context << u256(0x20) << Instruction::ADD;
				}
				else
				{
					m_context << Instruction::SWAP1 << Instruction::DUP2;
					m_context << u256(arrayType.calldataEncodedSize(true)) << Instruction::ADD;
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
					m_context << Instruction::SWAP1 << Instruction::DUP3 << Instruction::ADD;
					// stack: base_offset next_pointer data_pointer
					// retrieve length
					CompilerUtils(m_context).loadFromMemoryDynamic(IntegerType(256), !_fromMemory, true);
					// stack: base_offset next_pointer length data_pointer
					m_context << Instruction::SWAP2;
					// stack: base_offset data_pointer length next_pointer
				}
				else
				{
					// leave the pointer on the stack
					m_context << Instruction::DUP1;
					m_context << u256(calldataType->calldataEncodedSize()) << Instruction::ADD;
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
				m_context << Instruction::SWAP1;
			}
		}
		else
		{
			solAssert(!type->isDynamicallySized(), "Unknown dynamically sized type: " + type->toString());
			CompilerUtils(m_context).loadFromMemoryDynamic(*type, !_fromMemory, true);
			CompilerUtils(m_context).moveToStackTop(1 + type->sizeOnStack());
			m_context << Instruction::SWAP1;
		}
		// stack: v1 v2 ... v(k-1) v(k) base_offset mem_offset
	}
	m_context << Instruction::POP << Instruction::POP;
}

void ContractCompiler::appendReturnValuePacker(TypePointers const& _typeParameters, bool _isLibrary)
{
	CompilerUtils utils(m_context);
	if (_typeParameters.empty())
		m_context << Instruction::STOP;
	else
	{
		utils.fetchFreeMemoryPointer();
		//@todo optimization: if we return a single memory array, there should be enough space before
		// its data to add the needed parts and we avoid a memory copy.
		utils.encodeToMemory(_typeParameters, _typeParameters, true, false, _isLibrary);
		utils.toSizeAfterFreeMemoryPointer();
		m_context << Instruction::RETURN;
	}
}

void ContractCompiler::registerStateVariables(ContractDefinition const& _contract)
{
	for (auto const& var: ContractType(_contract).stateVariables())
		m_context.addStateVariable(*get<0>(var), get<1>(var), get<2>(var));
}

void ContractCompiler::initializeStateVariables(ContractDefinition const& _contract)
{
	for (VariableDeclaration const* variable: _contract.stateVariables())
		if (variable->value() && !variable->isConstant())
			ExpressionCompiler(m_context, m_optimise).appendStateVariableInitialization(*variable);
}

bool ContractCompiler::visit(VariableDeclaration const& _variableDeclaration)
{
	solAssert(_variableDeclaration.isStateVariable(), "Compiler visit to non-state variable declaration.");
	CompilerContext::LocationSetter locationSetter(m_context, _variableDeclaration);

	m_context.startFunction(_variableDeclaration);
	m_breakTags.clear();
	m_continueTags.clear();

	if (_variableDeclaration.isConstant())
		ExpressionCompiler(m_context, m_optimise).appendConstStateVariableAccessor(_variableDeclaration);
	else
		ExpressionCompiler(m_context, m_optimise).appendStateVariableAccessor(_variableDeclaration);

	return false;
}

bool ContractCompiler::visit(FunctionDefinition const& _function)
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

	solAssert(m_returnTags.empty(), "");
	m_breakTags.clear();
	m_continueTags.clear();
	m_stackCleanupForReturn = 0;
	m_currentFunction = &_function;
	m_modifierDepth = -1;

	appendModifierOrFunctionCode();

	solAssert(m_returnTags.empty(), "");

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

	if (stackLayout.size() > 17)
		BOOST_THROW_EXCEPTION(
			CompilerError() <<
			errinfo_sourceLocation(_function.location()) <<
			errinfo_comment("Stack too deep, try removing local variables.")
		);
	while (stackLayout.back() != int(stackLayout.size() - 1))
		if (stackLayout.back() < 0)
		{
			m_context << Instruction::POP;
			stackLayout.pop_back();
		}
		else
		{
			m_context << swapInstruction(stackLayout.size() - stackLayout.back() - 1);
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

bool ContractCompiler::visit(InlineAssembly const& _inlineAssembly)
{
	ErrorList errors;
	assembly::CodeGenerator codeGen(errors);
	unsigned startStackHeight = m_context.stackHeight();
	assembly::ExternalIdentifierAccess identifierAccess;
	identifierAccess.resolve = [&](assembly::Identifier const& _identifier, assembly::IdentifierContext)
	{
		auto ref = _inlineAssembly.annotation().externalReferences.find(&_identifier);
		if (ref == _inlineAssembly.annotation().externalReferences.end())
			return size_t(-1);
		return ref->second.valueSize;
	};
	identifierAccess.generateCode = [&](assembly::Identifier const& _identifier, assembly::IdentifierContext _context, julia::AbstractAssembly& _assembly)
	{
		auto ref = _inlineAssembly.annotation().externalReferences.find(&_identifier);
		solAssert(ref != _inlineAssembly.annotation().externalReferences.end(), "");
		Declaration const* decl = ref->second.declaration;
		solAssert(!!decl, "");
		if (_context == assembly::IdentifierContext::RValue)
		{
			int const depositBefore = _assembly.stackHeight();
			solAssert(!!decl->type(), "Type of declaration required but not yet determined.");
			if (FunctionDefinition const* functionDef = dynamic_cast<FunctionDefinition const*>(decl))
			{
				solAssert(!ref->second.isOffset && !ref->second.isSlot, "");
				functionDef = &m_context.resolveVirtualFunction(*functionDef);
				auto functionEntryLabel = m_context.functionEntryLabel(*functionDef).pushTag();
				solAssert(functionEntryLabel.data() <= std::numeric_limits<size_t>::max(), "");
				_assembly.appendLabelReference(size_t(functionEntryLabel.data()));
				// If there is a runtime context, we have to merge both labels into the same
				// stack slot in case we store it in storage.
				if (CompilerContext* rtc = m_context.runtimeContext())
				{
					_assembly.appendConstant(u256(1) << 32);
					_assembly.appendInstruction(Instruction::MUL);
					auto runtimeEntryLabel = rtc->functionEntryLabel(*functionDef).toSubAssemblyTag(m_context.runtimeSub());
					solAssert(runtimeEntryLabel.data() <= std::numeric_limits<size_t>::max(), "");
					_assembly.appendLabelReference(size_t(runtimeEntryLabel.data()));
					_assembly.appendInstruction(Instruction::OR);
				}
			}
			else if (auto variable = dynamic_cast<VariableDeclaration const*>(decl))
			{
				solAssert(!variable->isConstant(), "");
				if (m_context.isStateVariable(decl))
				{
					auto const& location = m_context.storageLocationOfVariable(*decl);
					if (ref->second.isSlot)
						m_context << location.first;
					else if (ref->second.isOffset)
						m_context << u256(location.second);
					else
						solAssert(false, "");
				}
				else if (m_context.isLocalVariable(decl))
				{
					int stackDiff = _assembly.stackHeight() - m_context.baseStackOffsetOfVariable(*variable);
					if (ref->second.isSlot || ref->second.isOffset)
					{
						solAssert(variable->type()->dataStoredIn(DataLocation::Storage), "");
						unsigned size = variable->type()->sizeOnStack();
						if (size == 2)
						{
							// slot plus offset
							if (ref->second.isOffset)
								stackDiff--;
						}
						else
						{
							solAssert(size == 1, "");
							// only slot, offset is zero
							if (ref->second.isOffset)
							{
								_assembly.appendConstant(u256(0));
								return;
							}
						}
					}
					else
						solAssert(variable->type()->sizeOnStack() == 1, "");
					if (stackDiff < 1 || stackDiff > 16)
						BOOST_THROW_EXCEPTION(
							CompilerError() <<
							errinfo_sourceLocation(_inlineAssembly.location()) <<
							errinfo_comment("Stack too deep, try removing local variables.")
						);
					solAssert(variable->type()->sizeOnStack() == 1, "");
					_assembly.appendInstruction(dupInstruction(stackDiff));
				}
				else
					solAssert(false, "");
			}
			else if (auto contract = dynamic_cast<ContractDefinition const*>(decl))
			{
				solAssert(!ref->second.isOffset && !ref->second.isSlot, "");
				solAssert(contract->isLibrary(), "");
				_assembly.appendLinkerSymbol(contract->fullyQualifiedName());
			}
			else
				solAssert(false, "Invalid declaration type.");
			solAssert(_assembly.stackHeight() - depositBefore == int(ref->second.valueSize), "");
		}
		else
		{
			// lvalue context
			solAssert(!ref->second.isOffset && !ref->second.isSlot, "");
			auto variable = dynamic_cast<VariableDeclaration const*>(decl);
			solAssert(
				!!variable && m_context.isLocalVariable(variable),
				"Can only assign to stack variables in inline assembly."
			);
			solAssert(variable->type()->sizeOnStack() == 1, "");
			int stackDiff = _assembly.stackHeight() - m_context.baseStackOffsetOfVariable(*variable) - 1;
			if (stackDiff > 16 || stackDiff < 1)
				BOOST_THROW_EXCEPTION(
					CompilerError() <<
					errinfo_sourceLocation(_inlineAssembly.location()) <<
					errinfo_comment("Stack too deep(" + to_string(stackDiff) + "), try removing local variables.")
				);
			_assembly.appendInstruction(swapInstruction(stackDiff));
			_assembly.appendInstruction(Instruction::POP);
		}
	};
	solAssert(_inlineAssembly.annotation().analysisInfo, "");
	codeGen.assemble(
		_inlineAssembly.operations(),
		*_inlineAssembly.annotation().analysisInfo,
		m_context.nonConstAssembly(),
		identifierAccess
	);
	solAssert(Error::containsOnlyWarnings(errors), "Code generation for inline assembly with errors requested.");
	m_context.setStackOffset(startStackHeight);
	return false;
}

bool ContractCompiler::visit(IfStatement const& _ifStatement)
{
	StackHeightChecker checker(m_context);
	CompilerContext::LocationSetter locationSetter(m_context, _ifStatement);
	compileExpression(_ifStatement.condition());
	m_context << Instruction::ISZERO;
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

bool ContractCompiler::visit(WhileStatement const& _whileStatement)
{
	StackHeightChecker checker(m_context);
	CompilerContext::LocationSetter locationSetter(m_context, _whileStatement);
	eth::AssemblyItem loopStart = m_context.newTag();
	eth::AssemblyItem loopEnd = m_context.newTag();
	m_continueTags.push_back(loopStart);
	m_breakTags.push_back(loopEnd);

	m_context << loopStart;

	// While loops have the condition prepended
	if (!_whileStatement.isDoWhile())
	{
		compileExpression(_whileStatement.condition());
		m_context << Instruction::ISZERO;
		m_context.appendConditionalJumpTo(loopEnd);
	}

	_whileStatement.body().accept(*this);

	// Do-while loops have the condition appended
	if (_whileStatement.isDoWhile())
	{
		compileExpression(_whileStatement.condition());
		m_context << Instruction::ISZERO;
		m_context.appendConditionalJumpTo(loopEnd);
	}

	m_context.appendJumpTo(loopStart);
	m_context << loopEnd;

	m_continueTags.pop_back();
	m_breakTags.pop_back();

	checker.check();
	return false;
}

bool ContractCompiler::visit(ForStatement const& _forStatement)
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
		m_context << Instruction::ISZERO;
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

bool ContractCompiler::visit(Continue const& _continueStatement)
{
	CompilerContext::LocationSetter locationSetter(m_context, _continueStatement);
	if (!m_continueTags.empty())
		m_context.appendJumpTo(m_continueTags.back());
	return false;
}

bool ContractCompiler::visit(Break const& _breakStatement)
{
	CompilerContext::LocationSetter locationSetter(m_context, _breakStatement);
	if (!m_breakTags.empty())
		m_context.appendJumpTo(m_breakTags.back());
	return false;
}

bool ContractCompiler::visit(Return const& _return)
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
		m_context << Instruction::POP;
	m_context.appendJumpTo(m_returnTags.back());
	m_context.adjustStackOffset(m_stackCleanupForReturn);
	return false;
}

bool ContractCompiler::visit(Throw const& _throw)
{
	CompilerContext::LocationSetter locationSetter(m_context, _throw);
	// Do not send back an error detail.
	m_context << u256(0) << u256(0);
	m_context << Instruction::REVERT;
	return false;
}

bool ContractCompiler::visit(VariableDeclarationStatement const& _variableDeclarationStatement)
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

bool ContractCompiler::visit(ExpressionStatement const& _expressionStatement)
{
	StackHeightChecker checker(m_context);
	CompilerContext::LocationSetter locationSetter(m_context, _expressionStatement);
	Expression const& expression = _expressionStatement.expression();
	compileExpression(expression);
	CompilerUtils(m_context).popStackElement(*expression.annotation().type);
	checker.check();
	return false;
}

bool ContractCompiler::visit(PlaceholderStatement const& _placeholderStatement)
{
	StackHeightChecker checker(m_context);
	CompilerContext::LocationSetter locationSetter(m_context, _placeholderStatement);
	appendModifierOrFunctionCode();
	checker.check();
	return true;
}

void ContractCompiler::appendMissingFunctions()
{
	while (Declaration const* function = m_context.nextFunctionToCompile())
	{
		m_context.setStackOffset(0);
		function->accept(*this);
		solAssert(m_context.nextFunctionToCompile() != function, "Compiled the wrong function?");
	}
	m_context.appendMissingLowLevelFunctions();
}

void ContractCompiler::appendModifierOrFunctionCode()
{
	solAssert(m_currentFunction, "");
	unsigned stackSurplus = 0;
	Block const* codeBlock = nullptr;

	m_modifierDepth++;

	if (m_modifierDepth >= m_currentFunction->modifiers().size())
	{
		solAssert(m_currentFunction->isImplemented(), "");
		codeBlock = &m_currentFunction->body();
	}
	else
	{
		ASTPointer<ModifierInvocation> const& modifierInvocation = m_currentFunction->modifiers()[m_modifierDepth];

		// constructor call should be excluded
		if (dynamic_cast<ContractDefinition const*>(modifierInvocation->name()->annotation().referencedDeclaration))
			appendModifierOrFunctionCode();
		else
		{
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

			stackSurplus =
				CompilerUtils::sizeOnStack(modifier.parameters()) +
				CompilerUtils::sizeOnStack(modifier.localVariables());
			codeBlock = &modifier.body();
		}
	}

	if (codeBlock)
	{
		m_returnTags.push_back(m_context.newTag());

		codeBlock->accept(*this);

		solAssert(!m_returnTags.empty(), "");
		m_context << m_returnTags.back();
		m_returnTags.pop_back();

		CompilerUtils(m_context).popStackSlots(stackSurplus);
	}
	m_modifierDepth--;
}

void ContractCompiler::appendStackVariableInitialisation(VariableDeclaration const& _variable)
{
	CompilerContext::LocationSetter location(m_context, _variable);
	m_context.addVariable(_variable);
	CompilerUtils(m_context).pushZeroValue(*_variable.annotation().type);
}

void ContractCompiler::compileExpression(Expression const& _expression, TypePointer const& _targetType)
{
	ExpressionCompiler expressionCompiler(m_context, m_optimise);
	expressionCompiler.compile(_expression);
	if (_targetType)
		CompilerUtils(m_context).convertType(*_expression.annotation().type, *_targetType);
}

eth::AssemblyPointer ContractCompiler::cloneRuntime()
{
	eth::Assembly a;
	a << Instruction::CALLDATASIZE;
	a << u256(0) << Instruction::DUP1 << Instruction::CALLDATACOPY;
	//@todo adjust for larger return values, make this dynamic.
	a << u256(0x20) << u256(0) << Instruction::CALLDATASIZE;
	a << u256(0);
	// this is the address which has to be substituted by the linker.
	//@todo implement as special "marker" AssemblyItem.
	a << u256("0xcafecafecafecafecafecafecafecafecafecafe");
	a << u256(eth::GasCosts::callGas + 10) << Instruction::GAS << Instruction::SUB;
	a << Instruction::DELEGATECALL;
	//Propagate error condition (if DELEGATECALL pushes 0 on stack).
	a << Instruction::ISZERO;
	a << Instruction::ISZERO;
	eth::AssemblyItem afterTag = a.appendJumpI().tag();
	a << Instruction::INVALID << afterTag;
	//@todo adjust for larger return values, make this dynamic.
	a << u256(0x20) << u256(0) << Instruction::RETURN;
	return make_shared<eth::Assembly>(a);
}
