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

#include <libsolidity/ast/AST.h>
#include <libsolidity/codegen/CompilerUtils.h>
#include <libsolidity/codegen/ContractCompiler.h>
#include <libsolidity/codegen/ExpressionCompiler.h>

#include <libyul/backends/evm/AsmCodeGen.h>

#include <libevmasm/Instruction.h>
#include <libevmasm/Assembly.h>
#include <libevmasm/GasMeter.h>

#include <liblangutil/ErrorReporter.h>

#include <boost/range/adaptor/reversed.hpp>
#include <algorithm>

using namespace std;
using namespace dev;
using namespace langutil;
using namespace dev::solidity;

namespace
{

/**
 * Simple helper class to ensure that the stack height is the same at certain places in the code.
 */
class StackHeightChecker
{
public:
	explicit StackHeightChecker(CompilerContext const& _context):
		m_context(_context), stackHeight(m_context.stackHeight()) {}
	void check() { solAssert(m_context.stackHeight() == stackHeight, std::string("I sense a disturbance in the stack: ") + to_string(m_context.stackHeight()) + " vs " + to_string(stackHeight)); }
private:
	CompilerContext const& m_context;
	unsigned stackHeight;
};

}

void ContractCompiler::compileContract(
	ContractDefinition const& _contract,
	map<ContractDefinition const*, shared_ptr<Compiler const>> const& _otherCompilers
)
{
	CompilerContext::LocationSetter locationSetter(m_context, _contract);

	if (_contract.isLibrary())
		// Check whether this is a call (true) or a delegatecall (false).
		// This has to be the first code in the contract.
		appendDelegatecallCheck();

	initializeContext(_contract, _otherCompilers);
	// This generates the dispatch function for externally visible functions
	// and adds the function to the compilation queue. Additionally internal functions,
	// which are referenced directly or indirectly will be added.
	appendFunctionSelector(_contract);
	// This processes the above populated queue until it is empty.
	appendMissingFunctions();
}

size_t ContractCompiler::compileConstructor(
	ContractDefinition const& _contract,
	std::map<ContractDefinition const*, shared_ptr<Compiler const>> const& _otherCompilers
)
{
	CompilerContext::LocationSetter locationSetter(m_context, _contract);
	if (_contract.isLibrary())
		return deployLibrary(_contract);
	else
	{
		initializeContext(_contract, _otherCompilers);
		return packIntoContractCreator(_contract);
	}
}

void ContractCompiler::initializeContext(
	ContractDefinition const& _contract,
	map<ContractDefinition const*, shared_ptr<Compiler const>> const& _otherCompilers
)
{
	m_context.setExperimentalFeatures(_contract.sourceUnit().annotation().experimentalFeatures);
	m_context.setOtherCompilers(_otherCompilers);
	m_context.setInheritanceHierarchy(_contract.annotation().linearizedBaseContracts);
	CompilerUtils(m_context).initialiseFreeMemoryPointer();
	registerStateVariables(_contract);
	m_context.resetVisitedNodes(&_contract);
}

void ContractCompiler::appendCallValueCheck()
{
	// Throw if function is not payable but call contained ether.
	m_context << Instruction::CALLVALUE;
	// TODO: error message?
	m_context.appendConditionalRevert();
}

void ContractCompiler::appendInitAndConstructorCode(ContractDefinition const& _contract)
{
	solAssert(!_contract.isLibrary(), "Tried to initialize library.");
	CompilerContext::LocationSetter locationSetter(m_context, _contract);

	m_baseArguments = &_contract.annotation().baseConstructorArguments;

	// Initialization of state variables in base-to-derived order.
	for (ContractDefinition const* contract: boost::adaptors::reverse(
		_contract.annotation().linearizedBaseContracts
	))
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
	solAssert(!_contract.isLibrary(), "Tried to use contract creator or library.");

	appendInitAndConstructorCode(_contract);

	// We jump to the deploy routine because we first have to append all missing functions,
	// which can cause further functions to be added to the runtime context.
	eth::AssemblyItem deployRoutine = m_context.appendJumpToNew();

	// We have to include copies of functions in the construction time and runtime context
	// because of absolute jumps.
	appendMissingFunctions();
	m_runtimeCompiler->appendMissingFunctions();

	CompilerContext::LocationSetter locationSetter(m_context, _contract);
	m_context << deployRoutine;

	solAssert(m_context.runtimeSub() != size_t(-1), "Runtime sub not registered");
	m_context.pushSubroutineSize(m_context.runtimeSub());
	m_context << Instruction::DUP1;
	m_context.pushSubroutineOffset(m_context.runtimeSub());
	m_context << u256(0) << Instruction::CODECOPY;
	m_context << u256(0) << Instruction::RETURN;

	return m_context.runtimeSub();
}

size_t ContractCompiler::deployLibrary(ContractDefinition const& _contract)
{
	solAssert(!!m_runtimeCompiler, "");
	solAssert(_contract.isLibrary(), "Tried to deploy contract as library.");

	CompilerContext::LocationSetter locationSetter(m_context, _contract);

	solAssert(m_context.runtimeSub() != size_t(-1), "Runtime sub not registered");
	m_context.pushSubroutineSize(m_context.runtimeSub());
	m_context.pushSubroutineOffset(m_context.runtimeSub());
	m_context.appendInlineAssembly(R"(
	{
		// If code starts at 11, an mstore(0) writes to the full PUSH20 plus data
		// without the need for a shift.
		let codepos := 11
		codecopy(codepos, subOffset, subSize)
		// Check that the first opcode is a PUSH20
		switch eq(0x73, byte(0, mload(codepos)))
		case 0 { invalid() }
		mstore(0, address())
		mstore8(codepos, 0x73)
		return(codepos, subSize)
	}
	)", {"subSize", "subOffset"});

	return m_context.runtimeSub();
}

void ContractCompiler::appendBaseConstructor(FunctionDefinition const& _constructor)
{
	CompilerContext::LocationSetter locationSetter(m_context, _constructor);
	FunctionType constructorType(_constructor);
	if (!constructorType.parameterTypes().empty())
	{
		solAssert(m_baseArguments, "");
		solAssert(m_baseArguments->count(&_constructor), "");
		std::vector<ASTPointer<Expression>> const* arguments = nullptr;
		ASTNode const* baseArgumentNode = m_baseArguments->at(&_constructor);
		if (auto inheritanceSpecifier = dynamic_cast<InheritanceSpecifier const*>(baseArgumentNode))
			arguments = inheritanceSpecifier->arguments();
		else if (auto modifierInvocation = dynamic_cast<ModifierInvocation const*>(baseArgumentNode))
			arguments = modifierInvocation->arguments();
		solAssert(arguments, "");
		solAssert(arguments->size() == constructorType.parameterTypes().size(), "");
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
		m_context << Instruction::DUP1;
		CompilerUtils(m_context).storeFreeMemoryPointer();
		// stack: <memptr>
		CompilerUtils(m_context).abiDecode(FunctionType(_constructor).parameterTypes(), true);
	}
	_constructor.accept(*this);
}

void ContractCompiler::appendDelegatecallCheck()
{
	// Special constant that will be replaced by the address at deploy time.
	// At compilation time, this is just "PUSH20 00...000".
	m_context.appendDeployTimeAddress();
	m_context << Instruction::ADDRESS << Instruction::EQ;
	// The result on the stack is
	// "We have not been called via DELEGATECALL".
}

void ContractCompiler::appendInternalSelector(
	map<FixedHash<4>, eth::AssemblyItem const> const& _entryPoints,
	vector<FixedHash<4>> const& _ids,
	eth::AssemblyItem const& _notFoundTag,
	size_t _runs
)
{
	// Code for selecting from n functions without split:
	//   n times: dup1, push4 <id_i>, eq, push2/3 <tag_i>, jumpi
	//   push2/3 <notfound> jump
	// (called SELECT[n])
	// Code for selecting from n functions with split:
	//   dup1, push4 <pivot>, gt, push2/3<tag_less>, jumpi
	//     SELECT[n/2]
	//   tag_less:
	//     SELECT[n/2]
	//
	// This means each split adds 16-18 bytes of additional code (note the additional jump out!)
	// The average execution cost if we do not split at all are:
	//   (3 + 3 + 3 + 3 + 10) * n/2 = 24 * n/2 = 12 * n
	// If we split once:
	//    (3 + 3 + 3 + 3 + 10) + 24 * n/4 = 24 * (n/4 + 1) = 6 * n + 24;
	//
	// We should split if
	//     _runs * 12 * n > _runs * (6 * n + 24) + 17 * createDataGas
	// <=> _runs * 6 * (n - 4) > 17 * createDataGas
	//
	// Which also means that the execution itself is not profitable
	// unless we have at least 5 functions.

	// Start with some comparisons to avoid overflow, then do the actual comparison.
	bool split = false;
	if (_ids.size() <= 4)
		split = false;
	else if (_runs > (17 * eth::GasCosts::createDataGas) / 6)
		split = true;
	else
		split = (_runs * 6 * (_ids.size() - 4) > 17 * eth::GasCosts::createDataGas);

	if (split)
	{
		size_t pivotIndex = _ids.size() / 2;
		FixedHash<4> pivot{_ids.at(pivotIndex)};
		m_context << dupInstruction(1) << u256(FixedHash<4>::Arith(pivot)) << Instruction::GT;
		eth::AssemblyItem lessTag{m_context.appendConditionalJump()};
		// Here, we have funid >= pivot
		vector<FixedHash<4>> larger{_ids.begin() + pivotIndex, _ids.end()};
		appendInternalSelector(_entryPoints, larger, _notFoundTag, _runs);
		m_context << lessTag;
		// Here, we have funid < pivot
		vector<FixedHash<4>> smaller{_ids.begin(), _ids.begin() + pivotIndex};
		appendInternalSelector(_entryPoints, smaller, _notFoundTag, _runs);
	}
	else
	{
		for (auto const& id: _ids)
		{
			m_context << dupInstruction(1) << u256(FixedHash<4>::Arith(id)) << Instruction::EQ;
			m_context.appendConditionalJumpTo(_entryPoints.at(id));
		}
		m_context.appendJumpTo(_notFoundTag);
	}
}

namespace
{

// Helper function to check if any function is payable
bool hasPayableFunctions(ContractDefinition const& _contract)
{
	FunctionDefinition const* fallback = _contract.fallbackFunction();
	if (fallback && fallback->isPayable())
		return true;

	for (auto const& it: _contract.interfaceFunctions())
		if (it.second->isPayable())
			return true;

	return false;
}

}

void ContractCompiler::appendFunctionSelector(ContractDefinition const& _contract)
{
	map<FixedHash<4>, FunctionTypePointer> interfaceFunctions = _contract.interfaceFunctions();
	map<FixedHash<4>, eth::AssemblyItem const> callDataUnpackerEntryPoints;

	if (_contract.isLibrary())
	{
		solAssert(m_context.stackHeight() == 1, "CALL / DELEGATECALL flag expected.");
	}

	FunctionDefinition const* fallback = _contract.fallbackFunction();
	solAssert(!_contract.isLibrary() || !fallback, "Libraries can't have fallback functions");

	bool needToAddCallvalueCheck = true;
	if (!hasPayableFunctions(_contract) && !interfaceFunctions.empty() && !_contract.isLibrary())
	{
		appendCallValueCheck();
		needToAddCallvalueCheck = false;
	}

	eth::AssemblyItem notFound = m_context.newTag();
	// directly jump to fallback if the data is too short to contain a function selector
	// also guards against short data
	m_context << u256(4) << Instruction::CALLDATASIZE << Instruction::LT;
	m_context.appendConditionalJumpTo(notFound);

	// retrieve the function signature hash from the calldata
	if (!interfaceFunctions.empty())
	{
		CompilerUtils(m_context).loadFromMemory(0, IntegerType(CompilerUtils::dataStartOffset * 8), true);

		// stack now is: <can-call-non-view-functions>? <funhash>
		vector<FixedHash<4>> sortedIDs;
		for (auto const& it: interfaceFunctions)
		{
			callDataUnpackerEntryPoints.emplace(it.first, m_context.newTag());
			sortedIDs.emplace_back(it.first);
		}
		std::sort(sortedIDs.begin(), sortedIDs.end());
		appendInternalSelector(callDataUnpackerEntryPoints, sortedIDs, notFound, m_optimise_runs);
	}

	m_context << notFound;

	if (fallback)
	{
		solAssert(!_contract.isLibrary(), "");
		if (!fallback->isPayable() && needToAddCallvalueCheck)
			appendCallValueCheck();

		solAssert(fallback->isFallback(), "");
		solAssert(FunctionType(*fallback).parameterTypes().empty(), "");
		solAssert(FunctionType(*fallback).returnParameterTypes().empty(), "");
		fallback->accept(*this);
		m_context << Instruction::STOP;
	}
	else
		// TODO: error message here?
		m_context.appendRevert();

	for (auto const& it: interfaceFunctions)
	{
		FunctionTypePointer const& functionType = it.second;
		solAssert(functionType->hasDeclaration(), "");
		CompilerContext::LocationSetter locationSetter(m_context, functionType->declaration());

		m_context << callDataUnpackerEntryPoints.at(it.first);
		if (_contract.isLibrary() && functionType->stateMutability() > StateMutability::View)
		{
			// If the function is not a view function and is called without DELEGATECALL,
			// we revert.
			m_context << dupInstruction(2);
			m_context.appendConditionalRevert();
		}
		m_context.setStackOffset(0);
		// We have to allow this for libraries, because value of the previous
		// call is still visible in the delegatecall.
		if (!functionType->isPayable() && !_contract.isLibrary() && needToAddCallvalueCheck)
			appendCallValueCheck();

		// Return tag is used to jump out of the function.
		eth::AssemblyItem returnTag = m_context.pushNewTag();
		if (!functionType->parameterTypes().empty())
		{
			// Parameter for calldataUnpacker
			m_context << CompilerUtils::dataStartOffset;
			m_context << Instruction::DUP1 << Instruction::CALLDATASIZE << Instruction::SUB;
			CompilerUtils(m_context).abiDecode(functionType->parameterTypes());
		}
		m_context.appendJumpTo(
			m_context.functionEntryLabel(functionType->declaration()),
			eth::AssemblyItem::JumpType::IntoFunction
		);
		m_context << returnTag;
		// Return tag and input parameters get consumed.
		m_context.adjustStackOffset(
			CompilerUtils(m_context).sizeOnStack(functionType->returnParameterTypes()) -
			CompilerUtils(m_context).sizeOnStack(functionType->parameterTypes()) -
			1
		);
		// Consumes the return parameters.
		appendReturnValuePacker(functionType->returnParameterTypes(), _contract.isLibrary());
	}
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
		utils.abiEncode(_typeParameters, _typeParameters, _isLibrary);
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
	solAssert(!_contract.isLibrary(), "Tried to initialize state variables of library.");
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
	// reserve additional slots: [retarg0] ... [retargm]

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

	if (_function.isConstructor())
		if (auto c = m_context.nextConstructor(dynamic_cast<ContractDefinition const&>(*_function.scope())))
			appendBaseConstructor(*c);

	solAssert(m_returnTags.empty(), "");
	m_breakTags.clear();
	m_continueTags.clear();
	m_currentFunction = &_function;
	m_modifierDepth = -1;
	m_scopeStackHeight.clear();

	appendModifierOrFunctionCode();
	solAssert(m_returnTags.empty(), "");

	// Now we need to re-shuffle the stack. For this we keep a record of the stack layout
	// that shows the target positions of the elements, where "-1" denotes that this element needs
	// to be removed from the stack.
	// Note that the fact that the return arguments are of increasing index is vital for this
	// algorithm to work.

	unsigned const c_argumentsSize = CompilerUtils::sizeOnStack(_function.parameters());
	unsigned const c_returnValuesSize = CompilerUtils::sizeOnStack(_function.returnParameters());

	vector<int> stackLayout;
	stackLayout.push_back(c_returnValuesSize); // target of return address
	stackLayout += vector<int>(c_argumentsSize, -1); // discard all arguments
	for (unsigned i = 0; i < c_returnValuesSize; ++i)
		stackLayout.push_back(i);

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
	for (int i = 0; i < int(stackLayout.size()); ++i)
		if (stackLayout[i] != i)
			solAssert(false, "Invalid stack layout on cleanup.");

	for (ASTPointer<VariableDeclaration const> const& variable: _function.parameters() + _function.returnParameters())
		m_context.removeVariable(*variable);

	m_context.adjustStackOffset(-(int)c_returnValuesSize);

	/// The constructor and the fallback function doesn't to jump out.
	if (!_function.isConstructor())
	{
		solAssert(m_context.numberOfLocalVariables() == 0, "");
		if (!_function.isFallback())
			m_context.appendJump(eth::AssemblyItem::JumpType::OutOfFunction);
	}

	return false;
}

bool ContractCompiler::visit(InlineAssembly const& _inlineAssembly)
{
	unsigned startStackHeight = m_context.stackHeight();
	yul::ExternalIdentifierAccess identifierAccess;
	identifierAccess.resolve = [&](yul::Identifier const& _identifier, yul::IdentifierContext, bool)
	{
		auto ref = _inlineAssembly.annotation().externalReferences.find(&_identifier);
		if (ref == _inlineAssembly.annotation().externalReferences.end())
			return size_t(-1);
		return ref->second.valueSize;
	};
	identifierAccess.generateCode = [&](yul::Identifier const& _identifier, yul::IdentifierContext _context, yul::AbstractAssembly& _assembly)
	{
		auto ref = _inlineAssembly.annotation().externalReferences.find(&_identifier);
		solAssert(ref != _inlineAssembly.annotation().externalReferences.end(), "");
		Declaration const* decl = ref->second.declaration;
		solAssert(!!decl, "");
		if (_context == yul::IdentifierContext::RValue)
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
	yul::CodeGenerator::assemble(
		_inlineAssembly.operations(),
		*_inlineAssembly.annotation().analysisInfo,
		*m_context.assemblyPtr(),
		m_context.evmVersion(),
		identifierAccess
	);
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
	m_breakTags.emplace_back(loopEnd, m_context.stackHeight());

	m_context << loopStart;

	if (_whileStatement.isDoWhile())
	{
		eth::AssemblyItem condition = m_context.newTag();
		m_continueTags.emplace_back(condition, m_context.stackHeight());

		_whileStatement.body().accept(*this);

		m_context << condition;
		compileExpression(_whileStatement.condition());
		m_context << Instruction::ISZERO << Instruction::ISZERO;
		m_context.appendConditionalJumpTo(loopStart);
	}
	else
	{
		m_continueTags.emplace_back(loopStart, m_context.stackHeight());
		compileExpression(_whileStatement.condition());
		m_context << Instruction::ISZERO;
		m_context.appendConditionalJumpTo(loopEnd);

		_whileStatement.body().accept(*this);

		m_context.appendJumpTo(loopStart);
	}
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

	storeStackHeight(&_forStatement);

	if (_forStatement.initializationExpression())
		_forStatement.initializationExpression()->accept(*this);

	m_breakTags.emplace_back(loopEnd, m_context.stackHeight());
	m_continueTags.emplace_back(loopNext, m_context.stackHeight());
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

	// For the case where no break/return is executed:
	// loop initialization variables have to be freed
	popScopedVariables(&_forStatement);

	checker.check();
	return false;
}

bool ContractCompiler::visit(Continue const& _continueStatement)
{
	CompilerContext::LocationSetter locationSetter(m_context, _continueStatement);
	solAssert(!m_continueTags.empty(), "");
	CompilerUtils(m_context).popAndJump(m_continueTags.back().second, m_continueTags.back().first);
	return false;
}

bool ContractCompiler::visit(Break const& _breakStatement)
{
	CompilerContext::LocationSetter locationSetter(m_context, _breakStatement);
	solAssert(!m_breakTags.empty(), "");
	CompilerUtils(m_context).popAndJump(m_breakTags.back().second, m_breakTags.back().first);
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

	CompilerUtils(m_context).popAndJump(m_returnTags.back().second, m_returnTags.back().first);
	return false;
}

bool ContractCompiler::visit(Throw const&)
{
	solAssert(false, "Throw statement is disallowed.");
	return false;
}

bool ContractCompiler::visit(EmitStatement const& _emit)
{
	CompilerContext::LocationSetter locationSetter(m_context, _emit);
	StackHeightChecker checker(m_context);
	compileExpression(_emit.eventCall());
	checker.check();
	return false;
}

bool ContractCompiler::visit(VariableDeclarationStatement const& _variableDeclarationStatement)
{
	CompilerContext::LocationSetter locationSetter(m_context, _variableDeclarationStatement);

	// Local variable slots are reserved when their declaration is visited,
	// and freed in the end of their scope.
	for (auto _decl: _variableDeclarationStatement.declarations())
		if (_decl)
			appendStackVariableInitialisation(*_decl);

	StackHeightChecker checker(m_context);
	if (Expression const* expression = _variableDeclarationStatement.initialValue())
	{
		CompilerUtils utils(m_context);
		compileExpression(*expression);
		TypePointers valueTypes;
		if (auto tupleType = dynamic_cast<TupleType const*>(expression->annotation().type.get()))
			valueTypes = tupleType->components();
		else
			valueTypes = TypePointers{expression->annotation().type};
		auto const& declarations = _variableDeclarationStatement.declarations();
		solAssert(declarations.size() == valueTypes.size(), "");
		for (size_t i = 0; i < declarations.size(); ++i)
		{
			size_t j = declarations.size() - i - 1;
			solAssert(!!valueTypes[j], "");
			if (VariableDeclaration const* varDecl = declarations[j].get())
			{
				utils.convertType(*valueTypes[j], *varDecl->annotation().type);
				utils.moveToStackVariable(*varDecl);
			}
			else
				utils.popStackElement(*valueTypes[j]);
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

bool ContractCompiler::visit(Block const& _block)
{
	storeStackHeight(&_block);
	return true;
}

void ContractCompiler::endVisit(Block const& _block)
{
	// Frees local variables declared in the scope of this block.
	popScopedVariables(&_block);
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
	auto abiFunctions = m_context.abiFunctions().requestedFunctions();
	if (!abiFunctions.first.empty())
		m_context.appendInlineAssembly("{" + move(abiFunctions.first) + "}", {}, abiFunctions.second, true);
}

void ContractCompiler::appendModifierOrFunctionCode()
{
	solAssert(m_currentFunction, "");
	unsigned stackSurplus = 0;
	Block const* codeBlock = nullptr;
	vector<VariableDeclaration const*> addedVariables;

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
			ModifierDefinition const& nonVirtualModifier = dynamic_cast<ModifierDefinition const&>(
				*modifierInvocation->name()->annotation().referencedDeclaration
			);
			ModifierDefinition const& modifier = m_context.resolveVirtualFunctionModifier(nonVirtualModifier);
			CompilerContext::LocationSetter locationSetter(m_context, modifier);
			std::vector<ASTPointer<Expression>> const& modifierArguments =
				modifierInvocation->arguments() ? *modifierInvocation->arguments() : std::vector<ASTPointer<Expression>>();

			solAssert(modifier.parameters().size() == modifierArguments.size(), "");
			for (unsigned i = 0; i < modifier.parameters().size(); ++i)
			{
				m_context.addVariable(*modifier.parameters()[i]);
				addedVariables.push_back(modifier.parameters()[i].get());
				compileExpression(
					*modifierArguments[i],
					modifier.parameters()[i]->annotation().type
				);
			}

			stackSurplus = CompilerUtils::sizeOnStack(modifier.parameters());
			codeBlock = &modifier.body();
		}
	}

	if (codeBlock)
	{
		m_returnTags.emplace_back(m_context.newTag(), m_context.stackHeight());
		codeBlock->accept(*this);

		solAssert(!m_returnTags.empty(), "");
		m_context << m_returnTags.back().first;
		m_returnTags.pop_back();

		CompilerUtils(m_context).popStackSlots(stackSurplus);
		for (auto var: addedVariables)
			m_context.removeVariable(*var);
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

void ContractCompiler::popScopedVariables(ASTNode const* _node)
{
	unsigned blockHeight = m_scopeStackHeight.at(m_modifierDepth).at(_node);
	m_context.removeVariablesAboveStackHeight(blockHeight);
	solAssert(m_context.stackHeight() >= blockHeight, "");
	unsigned stackDiff = m_context.stackHeight() - blockHeight;
	CompilerUtils(m_context).popStackSlots(stackDiff);
	m_scopeStackHeight[m_modifierDepth].erase(_node);
	if (m_scopeStackHeight[m_modifierDepth].empty())
		m_scopeStackHeight.erase(m_modifierDepth);
}

void ContractCompiler::storeStackHeight(ASTNode const* _node)
{
	m_scopeStackHeight[m_modifierDepth][_node] = m_context.stackHeight();
}
