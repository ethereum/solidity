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
 * @author Christian <c@ethdev.com>
 * @date 2014
 * Solidity compiler.
 */


#include <libsolidity/ast/AST.h>
#include <libsolidity/ast/ASTUtils.h>
#include <libsolidity/ast/TypeProvider.h>
#include <libsolidity/codegen/CompilerUtils.h>
#include <libsolidity/codegen/ContractCompiler.h>
#include <libsolidity/codegen/ExpressionCompiler.h>

#include <libyul/AsmAnalysisInfo.h>
#include <libyul/AsmAnalysis.h>
#include <libyul/AST.h>
#include <libyul/backends/evm/AsmCodeGen.h>
#include <libyul/backends/evm/EVMMetrics.h>
#include <libyul/backends/evm/EVMDialect.h>
#include <libyul/optimiser/Suite.h>
#include <libyul/Object.h>
#include <libyul/optimiser/ASTCopier.h>
#include <libyul/YulName.h>

#include <libevmasm/Instruction.h>
#include <libevmasm/Assembly.h>
#include <libevmasm/GasMeter.h>

#include <liblangutil/ErrorReporter.h>

#include <libsolutil/Whiskers.h>
#include <libsolutil/FunctionSelector.h>
#include <libsolutil/StackTooDeepString.h>

#include <range/v3/view/reverse.hpp>

#include <algorithm>
#include <limits>

using namespace solidity;
using namespace solidity::evmasm;
using namespace solidity::frontend;
using namespace solidity::langutil;

using solidity::util::FixedHash;
using solidity::util::h256;
using solidity::util::errinfo_comment;

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
	void check()
	{
		solAssert(
			m_context.stackHeight() == stackHeight,
			std::string("I sense a disturbance in the stack: ") + std::to_string(m_context.stackHeight()) + " vs " + std::to_string(stackHeight)
		);
	}
private:
	CompilerContext const& m_context;
	unsigned stackHeight;
};

}

void ContractCompiler::compileContract(
	ContractDefinition const& _contract,
	std::map<ContractDefinition const*, std::shared_ptr<Compiler const>> const& _otherCompilers
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
}

size_t ContractCompiler::compileConstructor(
	ContractDefinition const& _contract,
	std::map<ContractDefinition const*, std::shared_ptr<Compiler const>> const& _otherCompilers
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
	std::map<ContractDefinition const*, std::shared_ptr<Compiler const>> const& _otherCompilers
)
{
	m_context.setUseABICoderV2(*_contract.sourceUnit().annotation().useABICoderV2);
	m_context.setOtherCompilers(_otherCompilers);
	m_context.setMostDerivedContract(_contract);
	if (m_runtimeCompiler)
		registerImmutableVariables(_contract);
	CompilerUtils(m_context).initialiseFreeMemoryPointer();
	registerStateVariables(_contract);
	m_context.resetVisitedNodes(&_contract);
}

void ContractCompiler::appendCallValueCheck()
{
	// Throw if function is not payable but call contained ether.
	m_context << Instruction::CALLVALUE;
	m_context.appendConditionalRevert(false, "Ether sent to non-payable function");
}

void ContractCompiler::appendInitAndConstructorCode(ContractDefinition const& _contract)
{
	solAssert(!_contract.isLibrary(), "Tried to initialize library.");
	CompilerContext::LocationSetter locationSetter(m_context, _contract);

	m_baseArguments = &_contract.annotation().baseConstructorArguments;

	// Initialization of state variables in base-to-derived order.
	for (ContractDefinition const* contract: _contract.annotation().linearizedBaseContracts | ranges::views::reverse)
		initializeStateVariables(*contract);

	if (FunctionDefinition const* constructor = _contract.constructor())
		appendConstructor(*constructor);
	else
	{
		// Implicit constructors are always non-payable.
		appendCallValueCheck();
		if (auto c = _contract.nextConstructor(m_context.mostDerivedContract()))
			appendBaseConstructor(*c);
	}
}

size_t ContractCompiler::packIntoContractCreator(ContractDefinition const& _contract)
{
	solAssert(!!m_runtimeCompiler, "");
	solAssert(!_contract.isLibrary(), "Tried to use contract creator or library.");

	appendInitAndConstructorCode(_contract);

	// We jump to the deploy routine because we first have to append all missing functions,
	// which can cause further functions to be added to the runtime context.
	evmasm::AssemblyItem deployRoutine = m_context.appendJumpToNew();

	// We have to include copies of functions in the construction time and runtime context
	// because of absolute jumps.
	appendMissingFunctions();
	m_runtimeCompiler->appendMissingFunctions();

	CompilerContext::LocationSetter locationSetter(m_context, _contract);
	m_context << deployRoutine;

	solAssert(m_context.runtimeSub() != std::numeric_limits<size_t>::max(), "Runtime sub not registered");

	ContractType contractType(_contract);
	auto const& immutables = contractType.immutableVariables();
	// Push all immutable values on the stack.
	for (auto const& immutable: immutables)
		CompilerUtils(m_context).loadFromMemory(
			static_cast<unsigned>(m_context.immutableMemoryOffset(*immutable)),
			*immutable->annotation().type,
			false,
			true
	);
	m_context.pushSubroutineSize(m_context.runtimeSub());
	if (immutables.empty())
		m_context << Instruction::DUP1;
	m_context.pushSubroutineOffset(m_context.runtimeSub());
	m_context << u256(0) << Instruction::CODECOPY;
	// Assign immutable values from stack in reversed order.
	for (auto const& immutable: immutables | ranges::views::reverse)
	{
		auto slotNames = m_context.immutableVariableSlotNames(*immutable);
		for (auto&& slotName: slotNames | ranges::views::reverse)
		{
			m_context << u256(0);
			m_context.appendImmutableAssignment(slotName);
		}
	}
	if (!immutables.empty())
		m_context.pushSubroutineSize(m_context.runtimeSub());
	m_context << u256(0) << Instruction::RETURN;

	return m_context.runtimeSub();
}

size_t ContractCompiler::deployLibrary(ContractDefinition const& _contract)
{
	solAssert(!!m_runtimeCompiler, "");
	solAssert(_contract.isLibrary(), "Tried to deploy contract as library.");

	appendMissingFunctions();
	m_runtimeCompiler->appendMissingFunctions();

	CompilerContext::LocationSetter locationSetter(m_context, _contract);

	solAssert(m_context.runtimeSub() != std::numeric_limits<size_t>::max(), "Runtime sub not registered");
	m_context.pushSubroutineSize(m_context.runtimeSub());
	m_context.pushSubroutineOffset(m_context.runtimeSub());
	// This code replaces the address added by appendDeployTimeAddress().
	m_context.appendInlineAssembly(
		util::Whiskers(R"(
		{
			// If code starts at 11, an mstore(0) writes to the full PUSH20 plus data
			// without the need for a shift.
			let codepos := 11
			codecopy(codepos, subOffset, subSize)
			// Check that the first opcode is a PUSH20
			if iszero(eq(0x73, byte(0, mload(codepos)))) {
				mstore(0, <panicSelector>)
				mstore(4, <panicCode>)
				revert(0, 0x24)
			}
			mstore(0, address())
			mstore8(codepos, 0x73)
			return(codepos, subSize)
		}
		)")
		("panicSelector", util::selectorFromSignatureU256("Panic(uint256)").str())
		("panicCode", "0")
		.render(),
		{"subSize", "subOffset"}
	);

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
		CompilerUtils(m_context).fetchFreeMemoryPointer();
		// CODESIZE returns the actual size of the code,
		// which is the size of the generated code (``programSize``)
		// plus the constructor arguments added to the transaction payload.
		m_context.appendProgramSize();
		m_context << Instruction::CODESIZE << Instruction::SUB;
		// stack: <memptr> <argument size>
		m_context << Instruction::DUP1;
		m_context.appendProgramSize();
		m_context << Instruction::DUP4 << Instruction::CODECOPY;
		// stack: <memptr> <argument size>
		m_context << Instruction::DUP2 << Instruction::DUP2 << Instruction::ADD;
		// stack: <memptr> <argument size> <mem end>
		CompilerUtils(m_context).storeFreeMemoryPointer();
		// stack: <memptr> <argument size>
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
	std::map<FixedHash<4>, evmasm::AssemblyItem const> const& _entryPoints,
	std::vector<FixedHash<4>> const& _ids,
	evmasm::AssemblyItem const& _notFoundTag,
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
	else if (_runs > (17 * evmasm::GasCosts::createDataGas) / 6)
		split = true;
	else
		split = (_runs * 6 * (_ids.size() - 4) > 17 * evmasm::GasCosts::createDataGas);

	if (split)
	{
		size_t pivotIndex = _ids.size() / 2;
		FixedHash<4> pivot{_ids.at(pivotIndex)};
		m_context << dupInstruction(1) << u256(FixedHash<4>::Arith(pivot)) << Instruction::GT;
		evmasm::AssemblyItem lessTag{m_context.appendConditionalJump()};
		// Here, we have funid >= pivot
		std::vector<FixedHash<4>> larger{_ids.begin() + static_cast<ptrdiff_t>(pivotIndex), _ids.end()};
		appendInternalSelector(_entryPoints, larger, _notFoundTag, _runs);
		m_context << lessTag;
		// Here, we have funid < pivot
		std::vector<FixedHash<4>> smaller{_ids.begin(), _ids.begin() + static_cast<ptrdiff_t>(pivotIndex)};
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
	if (_contract.receiveFunction())
		return true;

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
	std::map<FixedHash<4>, FunctionTypePointer> interfaceFunctions = _contract.interfaceFunctions();
	std::map<FixedHash<4>, evmasm::AssemblyItem const> callDataUnpackerEntryPoints;

	if (_contract.isLibrary())
	{
		solAssert(m_context.stackHeight() == 1, "CALL / DELEGATECALL flag expected.");
	}

	FunctionDefinition const* fallback = _contract.fallbackFunction();
	solAssert(!_contract.isLibrary() || !fallback, "Libraries can't have fallback functions");

	FunctionDefinition const* etherReceiver = _contract.receiveFunction();
	solAssert(!_contract.isLibrary() || !etherReceiver, "Libraries can't have ether receiver functions");

	bool needToAddCallvalueCheck = true;
	if (!hasPayableFunctions(_contract) && !interfaceFunctions.empty() && !_contract.isLibrary())
	{
		appendCallValueCheck();
		needToAddCallvalueCheck = false;
	}

	evmasm::AssemblyItem notFoundOrReceiveEther = m_context.newTag();
	// If there is neither a fallback nor a receive ether function, we only need one label to jump to, which
	// always reverts.
	evmasm::AssemblyItem notFound = (!fallback && !etherReceiver) ? notFoundOrReceiveEther : m_context.newTag();

	// directly jump to fallback or ether receiver if the data is too short to contain a function selector
	// also guards against short data
	m_context << u256(4) << Instruction::CALLDATASIZE << Instruction::LT;
	m_context.appendConditionalJumpTo(notFoundOrReceiveEther);

	// retrieve the function signature hash from the calldata
	if (!interfaceFunctions.empty())
	{
		CompilerUtils(m_context).loadFromMemory(0, IntegerType(CompilerUtils::dataStartOffset * 8), true, false);

		// stack now is: <can-call-non-view-functions>? <funhash>
		std::vector<FixedHash<4>> sortedIDs;
		for (auto const& it: interfaceFunctions)
		{
			callDataUnpackerEntryPoints.emplace(it.first, m_context.newTag());
			sortedIDs.emplace_back(it.first);
		}
		std::sort(sortedIDs.begin(), sortedIDs.end());
		appendInternalSelector(callDataUnpackerEntryPoints, sortedIDs, notFound, m_optimiserSettings.expectedExecutionsPerDeployment);
	}

	m_context << notFoundOrReceiveEther;

	if (!fallback && !etherReceiver)
		m_context.appendRevert("Contract does not have fallback nor receive functions");
	else
	{
		if (etherReceiver)
		{
			// directly jump to fallback, if there is calldata
			m_context << Instruction::CALLDATASIZE;
			m_context.appendConditionalJumpTo(notFound);

			solAssert(!_contract.isLibrary(), "");
			solAssert(etherReceiver->isReceive(), "");
			solAssert(FunctionType(*etherReceiver).parameterTypes().empty(), "");
			solAssert(FunctionType(*etherReceiver).returnParameterTypes().empty(), "");
			etherReceiver->accept(*this);
			m_context << Instruction::STOP;
		}

		m_context << notFound;
		if (fallback)
		{
			solAssert(!_contract.isLibrary(), "");
			if (!fallback->isPayable() && needToAddCallvalueCheck)
				appendCallValueCheck();

			solAssert(fallback->isFallback(), "");
			m_context.setStackOffset(0);

			if (!FunctionType(*fallback).parameterTypes().empty())
				m_context << u256(0) << Instruction::CALLDATASIZE;

			fallback->accept(*this);

			if (FunctionType(*fallback).returnParameterTypes().empty())
				m_context << Instruction::STOP;
			else
			{
				m_context << Instruction::DUP1 << Instruction::MLOAD << Instruction::SWAP1;
				m_context << u256(0x20) << Instruction::ADD;
				m_context << Instruction::RETURN;
			}
		}
		else
			m_context.appendRevert("Unknown signature and no fallback defined");
	}


	for (auto const& it: interfaceFunctions)
	{
		m_context.setStackOffset(1);
		FunctionTypePointer const& functionType = it.second;
		solAssert(functionType->hasDeclaration(), "");
		CompilerContext::LocationSetter locationSetter(m_context, functionType->declaration());

		m_context << callDataUnpackerEntryPoints.at(it.first);
		if (_contract.isLibrary() && functionType->stateMutability() > StateMutability::View)
		{
			// If the function is not a view function and is called without DELEGATECALL,
			// we revert.
			m_context << dupInstruction(2);
			m_context.appendConditionalRevert(false, "Non-view function of library called without DELEGATECALL");
		}
		m_context.setStackOffset(0);
		// We have to allow this for libraries, because value of the previous
		// call is still visible in the delegatecall.
		if (!functionType->isPayable() && !_contract.isLibrary() && needToAddCallvalueCheck)
			appendCallValueCheck();

		// Return tag is used to jump out of the function.
		evmasm::AssemblyItem returnTag = m_context.pushNewTag();
		if (!functionType->parameterTypes().empty())
		{
			// Parameter for calldataUnpacker
			m_context << CompilerUtils::dataStartOffset;
			m_context << Instruction::DUP1 << Instruction::CALLDATASIZE << Instruction::SUB;
			CompilerUtils(m_context).abiDecode(functionType->parameterTypes());
		}
		m_context.appendJumpTo(
			m_context.functionEntryLabel(functionType->declaration()),
			evmasm::AssemblyItem::JumpType::IntoFunction
		);
		m_context << returnTag;
		// Return tag and input parameters get consumed.
		m_context.adjustStackOffset(
			static_cast<int>(CompilerUtils::sizeOnStack(functionType->returnParameterTypes())) -
			static_cast<int>(CompilerUtils::sizeOnStack(functionType->parameterTypes())) -
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
	for (auto const location: {DataLocation::Storage, DataLocation::Transient})
		for (auto const& var: ContractType(_contract).stateVariables(location))
			m_context.addStateVariable(*std::get<0>(var), std::get<1>(var), std::get<2>(var));
}

void ContractCompiler::registerImmutableVariables(ContractDefinition const& _contract)
{
	solAssert(m_runtimeCompiler, "Attempted to register immutables for runtime code generation.");
	for (auto const& var: ContractType(_contract).immutableVariables())
		m_context.addImmutable(*var);
}

void ContractCompiler::initializeStateVariables(ContractDefinition const& _contract)
{
	solAssert(!_contract.isLibrary(), "Tried to initialize state variables of library.");
	for (VariableDeclaration const* variable: _contract.stateVariables())
	{
		solAssert(variable->referenceLocation() != VariableDeclaration::Location::Transient || !variable->value());
		if (variable->value() && !variable->isConstant())
			ExpressionCompiler(m_context, m_optimiserSettings.runOrderLiterals).appendStateVariableInitialization(*variable);
	}
}

bool ContractCompiler::visit(VariableDeclaration const& _variableDeclaration)
{
	solAssert(_variableDeclaration.isStateVariable(), "Compiler visit to non-state variable declaration.");
	CompilerContext::LocationSetter locationSetter(m_context, _variableDeclaration);

	m_context.startFunction(_variableDeclaration);
	m_breakTags.clear();
	m_continueTags.clear();

	if (_variableDeclaration.isConstant())
		ExpressionCompiler(m_context, m_optimiserSettings.runOrderLiterals)
			.appendConstStateVariableAccessor(_variableDeclaration);
	else
		ExpressionCompiler(m_context, m_optimiserSettings.runOrderLiterals)
			.appendStateVariableAccessor(_variableDeclaration);

	return false;
}

bool ContractCompiler::visit(FunctionDefinition const& _function)
{
	solAssert(_function.isImplemented(), "");

	CompilerContext::LocationSetter locationSetter(m_context, _function);

	m_context.startFunction(_function);

	// stack upon entry: [return address] [arg0] [arg1] ... [argn]
	// reserve additional slots: [retarg0] ... [retargm]

	unsigned parametersSize = CompilerUtils::sizeOnStack(_function.parameters());
	if (_function.isFallback())
		m_context.adjustStackOffset(static_cast<int>(parametersSize));
	else if (!_function.isConstructor())
		// adding 1 for return address.
		m_context.adjustStackOffset(static_cast<int>(parametersSize) + 1);
	for (ASTPointer<VariableDeclaration> const& variable: _function.parameters())
	{
		m_context.addVariable(*variable, parametersSize);
		parametersSize -= variable->annotation().type->sizeOnStack();
	}

	for (ASTPointer<VariableDeclaration> const& variable: _function.returnParameters())
		appendStackVariableInitialisation(*variable, /* _provideDefaultValue = */ true);

	if (_function.isConstructor())
		if (auto c = dynamic_cast<ContractDefinition const&>(*_function.scope()).nextConstructor(
				m_context.mostDerivedContract()
		))
			appendBaseConstructor(*c);

	solAssert(m_returnTags.empty(), "");
	m_breakTags.clear();
	m_continueTags.clear();
	m_currentFunction = &_function;
	m_modifierDepth = std::numeric_limits<unsigned>::max();
	m_scopeStackHeight.clear();
	m_context.setModifierDepth(0);

	appendModifierOrFunctionCode();
	m_context.setModifierDepth(0);
	solAssert(m_returnTags.empty(), "");

	// Now we need to re-shuffle the stack. For this we keep a record of the stack layout
	// that shows the target positions of the elements, where "-1" denotes that this element needs
	// to be removed from the stack.
	// Note that the fact that the return arguments are of increasing index is vital for this
	// algorithm to work.

	unsigned const c_argumentsSize = CompilerUtils::sizeOnStack(_function.parameters());
	unsigned const c_returnValuesSize = CompilerUtils::sizeOnStack(_function.returnParameters());

	std::vector<int> stackLayout;
	if (!_function.isConstructor() && !_function.isFallback())
		stackLayout.push_back(static_cast<int>(c_returnValuesSize)); // target of return address
	stackLayout += std::vector<int>(c_argumentsSize, -1); // discard all arguments
	for (size_t i = 0; i < c_returnValuesSize; ++i)
		stackLayout.push_back(static_cast<int>(i));

	if (stackLayout.size() > 17)
		BOOST_THROW_EXCEPTION(
			StackTooDeepError() <<
			errinfo_sourceLocation(_function.location()) <<
			util::errinfo_comment(util::stackTooDeepString)
		);
	while (!stackLayout.empty() && stackLayout.back() != static_cast<int>(stackLayout.size() - 1))
		if (stackLayout.back() < 0)
		{
			m_context << Instruction::POP;
			stackLayout.pop_back();
		}
		else
		{
			m_context << swapInstruction(static_cast<unsigned>(stackLayout.size()) - static_cast<unsigned>(stackLayout.back()) - 1u);
			std::swap(stackLayout[static_cast<size_t>(stackLayout.back())], stackLayout.back());
		}
	for (size_t i = 0; i < stackLayout.size(); ++i)
		if (stackLayout[i] != static_cast<int>(i))
			solAssert(false, "Invalid stack layout on cleanup.");

	for (ASTPointer<VariableDeclaration> const& variable: _function.parameters() + _function.returnParameters())
		m_context.removeVariable(*variable);

	m_context.adjustStackOffset(-(int)c_returnValuesSize);

	/// The constructor and the fallback function doesn't to jump out.
	if (!_function.isConstructor())
	{
		solAssert(m_context.numberOfLocalVariables() == 0, "");
		if (!_function.isFallback() && !_function.isReceive())
			m_context.appendJump(evmasm::AssemblyItem::JumpType::OutOfFunction);
	}

	return false;
}

bool ContractCompiler::visit(InlineAssembly const& _inlineAssembly)
{
	unsigned startStackHeight = m_context.stackHeight();
	yul::ExternalIdentifierAccess::CodeGenerator identifierAccessCodeGen = [&](
		yul::Identifier const& _identifier,
		yul::IdentifierContext _context,
		yul::AbstractAssembly& _assembly
	)
	{
		solAssert(_context == yul::IdentifierContext::RValue || _context == yul::IdentifierContext::LValue, "");
		auto ref = _inlineAssembly.annotation().externalReferences.find(&_identifier);
		solAssert(ref != _inlineAssembly.annotation().externalReferences.end(), "");
		Declaration const* decl = ref->second.declaration;
		solAssert(!!decl, "");
		if (_context == yul::IdentifierContext::RValue)
		{
			int const depositBefore = _assembly.stackHeight();
			solAssert(!!decl->type(), "Type of declaration required but not yet determined.");
			if (auto variable = dynamic_cast<VariableDeclaration const*>(decl))
			{
				solAssert(!variable->immutable(), "");
				if (variable->isConstant())
				{
					variable = rootConstVariableDeclaration(*variable);
					// If rootConstVariableDeclaration fails and returns nullptr,
					// it should have failed in TypeChecker already, causing a compilation error.
					// In such case we should not get here.
					solAssert(variable, "");

					u256 value;
					if (variable->value()->annotation().type->category() == Type::Category::RationalNumber)
					{
						value = dynamic_cast<RationalNumberType const&>(*variable->value()->annotation().type).literalValue(nullptr);
						if (FixedBytesType const* bytesType = dynamic_cast<FixedBytesType const*>(variable->type()))
							value = value << (256 - 8 * bytesType->numBytes());
						else
							solAssert(variable->type()->category() == Type::Category::Integer, "");
					}
					else if (Literal const* literal = dynamic_cast<Literal const*>(variable->value().get()))
					{
						Type const* type = literal->annotation().type;

						switch (type->category())
						{
						case Type::Category::Bool:
						case Type::Category::Address:
							// Either both the literal and the variable are bools, or they are both addresses.
							// If they are both bools, comparing category is the same as comparing the types.
							// If they are both addresses, compare category so that payable/nonpayable is not compared.
							solAssert(type->category() == variable->annotation().type->category(), "");
							value = type->literalValue(literal);
							break;
						case Type::Category::StringLiteral:
						{
							StringLiteralType const& stringLiteral = dynamic_cast<StringLiteralType const&>(*type);
							solAssert(variable->type()->category() == Type::Category::FixedBytes, "");
							unsigned const numBytes = dynamic_cast<FixedBytesType const&>(*variable->type()).numBytes();
							solAssert(stringLiteral.value().size() <= numBytes, "");
							value = u256(h256(stringLiteral.value(), h256::AlignLeft));
							break;
						}
						default:
							solAssert(false, "");
						}
					}
					else
						solAssert(false, "Invalid constant in inline assembly.");
					m_context << value;
				}
				else if (m_context.isStateVariable(decl))
				{
					auto const& location = m_context.storageLocationOfVariable(*decl);
					if (ref->second.suffix == "slot")
						m_context << location.first;
					else if (ref->second.suffix == "offset")
						m_context << u256(location.second);
					else
						solAssert(false, "");
				}
				else if (m_context.isLocalVariable(decl))
				{
					unsigned stackDiff = static_cast<unsigned>(_assembly.stackHeight()) - m_context.baseStackOffsetOfVariable(*variable);
					if (!ref->second.suffix.empty())
					{
						std::string const& suffix = ref->second.suffix;
						if (variable->type()->dataStoredIn(DataLocation::Storage))
						{
							solAssert(suffix == "offset" || suffix == "slot", "");
							unsigned size = variable->type()->sizeOnStack();
							if (size == 2)
							{
								// slot plus offset
								if (suffix == "offset")
									stackDiff--;
							}
							else
							{
								solAssert(size == 1, "");
								// only slot, offset is zero
								if (suffix == "offset")
								{
									_assembly.appendConstant(u256(0));
									return;
								}
							}
						}
						else if (variable->type()->dataStoredIn(DataLocation::CallData))
						{
							auto const* arrayType = dynamic_cast<ArrayType const*>(variable->type());
							solAssert(
								arrayType && arrayType->isDynamicallySized() && arrayType->dataStoredIn(DataLocation::CallData),
								""
							);
							solAssert(suffix == "offset" || suffix == "length", "");
							solAssert(variable->type()->sizeOnStack() == 2, "");
							if (suffix == "length")
								stackDiff--;
						}
						else if (
							auto const* functionType = dynamic_cast<FunctionType const*>(variable->type());
							functionType && functionType->kind() == FunctionType::Kind::External
						)
						{
							solAssert(suffix == "selector" || suffix == "address", "");
							solAssert(variable->type()->sizeOnStack() == 2, "");
							if (suffix == "selector")
								stackDiff--;
						}
						else
							solAssert(false, "");
					}
					else
						solAssert(variable->type()->sizeOnStack() == 1, "");
					if (stackDiff < 1 || stackDiff > 16)
						BOOST_THROW_EXCEPTION(
							StackTooDeepError() <<
							errinfo_sourceLocation(_inlineAssembly.location()) <<
							util::errinfo_comment(util::stackTooDeepString)
						);
					_assembly.appendInstruction(dupInstruction(stackDiff));
				}
				else
					solAssert(false, "");
			}
			else if (auto contract = dynamic_cast<ContractDefinition const*>(decl))
			{
				solAssert(ref->second.suffix.empty(), "");
				solAssert(contract->isLibrary(), "");
				_assembly.appendLinkerSymbol(contract->fullyQualifiedName());
			}
			else
				solAssert(false, "Invalid declaration type.");
			solAssert(_assembly.stackHeight() - depositBefore == static_cast<int>(ref->second.valueSize), "");
		}
		else
		{
			// lvalue context
			auto variable = dynamic_cast<VariableDeclaration const*>(decl);
			unsigned stackDiff = static_cast<unsigned>(_assembly.stackHeight()) - m_context.baseStackOffsetOfVariable(*variable) - 1;
			std::string const& suffix = ref->second.suffix;
			if (variable->type()->dataStoredIn(DataLocation::Storage))
			{
				solAssert(
					!!variable && m_context.isLocalVariable(variable),
					"Can only assign to stack variables in inline assembly."
				);
				solAssert(variable->type()->sizeOnStack() == 1, "");
				solAssert(suffix == "slot", "");
			}
			else if (variable->type()->dataStoredIn(DataLocation::CallData))
			{
				if (auto const* arrayType = dynamic_cast<ArrayType const*>(variable->type()))
				{
					if (arrayType->isDynamicallySized())
					{
						solAssert(suffix == "offset" || suffix == "length", "");
						solAssert(variable->type()->sizeOnStack() == 2, "");
						if (suffix == "length")
							stackDiff--;
					}
					else
					{
						solAssert(variable->type()->sizeOnStack() == 1, "");
						solAssert(suffix.empty(), "");
					}
				}
				else
				{
					auto const* structType = dynamic_cast<StructType const*>(variable->type());
					solAssert(structType, "");
					solAssert(variable->type()->sizeOnStack() == 1, "");
					solAssert(suffix.empty(), "");
				}
			}
			else if (
				auto const* functionType = dynamic_cast<FunctionType const*>(variable->type());
				functionType && functionType->kind() == FunctionType::Kind::External
			)
			{
				solAssert(suffix == "selector" || suffix == "address", "");
				solAssert(variable->type()->sizeOnStack() == 2, "");
				if (suffix == "selector")
					stackDiff--;
			}
			else
				solAssert(suffix.empty(), "");

			if (stackDiff > 16 || stackDiff < 1)
				BOOST_THROW_EXCEPTION(
					StackTooDeepError() <<
					errinfo_sourceLocation(_inlineAssembly.location()) <<
					util::errinfo_comment(util::stackTooDeepString)
				);
			_assembly.appendInstruction(swapInstruction(stackDiff));
			_assembly.appendInstruction(Instruction::POP);
		}
	};

	yul::AST const* code = &_inlineAssembly.operations();
	yul::AsmAnalysisInfo* analysisInfo = _inlineAssembly.annotation().analysisInfo.get();

	// Only used in the scope below, but required to live outside to keep the
	// std::shared_ptr's alive
	yul::Object object{_inlineAssembly.dialect()};

	// The optimiser cannot handle external references
	if (
		m_optimiserSettings.runYulOptimiser &&
		_inlineAssembly.annotation().externalReferences.empty()
	)
	{
		yul::EVMDialect const* dialect = dynamic_cast<decltype(dialect)>(&_inlineAssembly.dialect());
		solAssert(dialect, "");

		// Create a modifiable copy of the code and analysis
		object.setCode(std::make_shared<yul::AST>(yul::ASTCopier().translate(code->root())));
		object.analysisInfo = std::make_shared<yul::AsmAnalysisInfo>(yul::AsmAnalyzer::analyzeStrictAssertCorrect(*dialect, object));

		m_context.optimizeYul(object, *dialect, m_optimiserSettings);

		code = object.code().get();
		analysisInfo = object.analysisInfo.get();
	}

	yul::CodeGenerator::assemble(
		code->root(),
		*analysisInfo,
		*m_context.assemblyPtr(),
		m_context.evmVersion(),
		std::nullopt,
		identifierAccessCodeGen,
		false,
		m_optimiserSettings.optimizeStackAllocation
	);
	m_context.setStackOffset(static_cast<int>(startStackHeight));
	return false;
}

bool ContractCompiler::visit(TryStatement const& _tryStatement)
{
	StackHeightChecker checker(m_context);
	CompilerContext::LocationSetter locationSetter(m_context, _tryStatement);

	compileExpression(_tryStatement.externalCall());
	int const returnSize = static_cast<int>(_tryStatement.externalCall().annotation().type->sizeOnStack());

	// Stack: [ return values] <success flag>
	evmasm::AssemblyItem successTag = m_context.appendConditionalJump();

	// Catch case.
	m_context.adjustStackOffset(-returnSize);

	handleCatch(_tryStatement.clauses());

	evmasm::AssemblyItem endTag = m_context.appendJumpToNew();

	m_context << successTag;
	m_context.adjustStackOffset(returnSize);
	{
		// Success case.
		// Stack: return values
		TryCatchClause const& successClause = *_tryStatement.clauses().front();
		if (successClause.parameters())
		{
			std::vector<Type const*> exprTypes{_tryStatement.externalCall().annotation().type};
			if (auto tupleType = dynamic_cast<TupleType const*>(exprTypes.front()))
				exprTypes = tupleType->components();
			std::vector<ASTPointer<VariableDeclaration>> const& params = successClause.parameters()->parameters();
			solAssert(exprTypes.size() == params.size(), "");
			for (size_t i = 0; i < exprTypes.size(); ++i)
				solAssert(params[i] && exprTypes[i] && *params[i]->annotation().type == *exprTypes[i], "");
		}
		else
			CompilerUtils(m_context).popStackSlots(static_cast<size_t>(returnSize));

		_tryStatement.clauses().front()->accept(*this);
	}

	m_context << endTag;
	checker.check();
	return false;
}

void ContractCompiler::handleCatch(std::vector<ASTPointer<TryCatchClause>> const& _catchClauses)
{
	// Stack is empty.
	ASTPointer<TryCatchClause> error{};
	ASTPointer<TryCatchClause> panic{};
	ASTPointer<TryCatchClause> fallback{};
	for (size_t i = 1; i < _catchClauses.size(); ++i)
		if (_catchClauses[i]->errorName() == "Error")
			error = _catchClauses[i];
		else if (_catchClauses[i]->errorName() == "Panic")
			panic = _catchClauses[i];
		else if (_catchClauses[i]->errorName().empty())
			fallback = _catchClauses[i];
		else
			solAssert(false, "");

	solAssert(_catchClauses.size() == 1ul + (error ? 1 : 0) + (panic ? 1 : 0) + (fallback ? 1 : 0), "");

	evmasm::AssemblyItem endTag = m_context.newTag();
	evmasm::AssemblyItem fallbackTag = m_context.newTag();
	evmasm::AssemblyItem panicTag = m_context.newTag();
	if (error || panic)
		// Note that this function returns zero on failure, which is not a problem yet,
		// but will be a problem once we allow user-defined errors.
		m_context.callYulFunction(m_context.utilFunctions().returnDataSelectorFunction(), 0, 1);
		// stack: <selector>
	if (error)
	{
		solAssert(
			error->parameters() &&
			error->parameters()->parameters().size() == 1 &&
			error->parameters()->parameters().front() &&
			*error->parameters()->parameters().front()->annotation().type == *TypeProvider::stringMemory(),
			""
		);
		solAssert(m_context.evmVersion().supportsReturndata(), "");

		// stack: <selector>
		m_context << Instruction::DUP1 << util::selectorFromSignatureU32("Error(string)") << Instruction::EQ;
		m_context << Instruction::ISZERO;
		m_context.appendConditionalJumpTo(panicTag);
		m_context << Instruction::POP; // remove selector

		// Try to decode the error message.
		// If this fails, leaves 0 on the stack, otherwise the pointer to the data string.
		m_context.callYulFunction(m_context.utilFunctions().tryDecodeErrorMessageFunction(), 0, 1);
		m_context << Instruction::DUP1;
		AssemblyItem decodeSuccessTag = m_context.appendConditionalJump();
		m_context << Instruction::POP;
		m_context.appendJumpTo(fallbackTag);
		m_context.adjustStackOffset(1);

		m_context << decodeSuccessTag;
		error->accept(*this);
		m_context.appendJumpTo(endTag);
		m_context.adjustStackOffset(1);
	}
	m_context << panicTag;
	if (panic)
	{
		solAssert(
			panic->parameters() &&
			panic->parameters()->parameters().size() == 1 &&
			panic->parameters()->parameters().front() &&
			*panic->parameters()->parameters().front()->annotation().type == *TypeProvider::uint256(),
			""
		);
		solAssert(m_context.evmVersion().supportsReturndata(), "");

		// stack: <selector>
		m_context << util::selectorFromSignatureU32("Panic(uint256)") << Instruction::EQ;
		m_context << Instruction::ISZERO;
		m_context.appendConditionalJumpTo(fallbackTag);

		m_context.callYulFunction(m_context.utilFunctions().tryDecodePanicDataFunction(), 0, 2);
		m_context << Instruction::SWAP1;
		// stack: <code> <success>
		AssemblyItem decodeSuccessTag = m_context.appendConditionalJump();
		m_context << Instruction::POP;
		m_context.appendJumpTo(fallbackTag);
		m_context.adjustStackOffset(1);

		m_context << decodeSuccessTag;
		panic->accept(*this);
		m_context.appendJumpTo(endTag);
		m_context.adjustStackOffset(1);
	}
	if (error || panic)
		m_context << Instruction::POP; // selector
	m_context << fallbackTag;
	if (fallback)
	{
		if (fallback->parameters())
		{
			solAssert(m_context.evmVersion().supportsReturndata(), "");
			solAssert(
				fallback->parameters()->parameters().size() == 1 &&
				fallback->parameters()->parameters().front() &&
				*fallback->parameters()->parameters().front()->annotation().type == *TypeProvider::bytesMemory(),
				""
			);
			CompilerUtils(m_context).returnDataToArray();
		}

		fallback->accept(*this);
	}
	else
	{
		// re-throw
		if (m_context.evmVersion().supportsReturndata())
			m_context.appendInlineAssembly(R"({
				returndatacopy(0, 0, returndatasize())
				revert(0, returndatasize())
			})");
		else
			// Since both returndata and revert are >=byzantium, this should be unreachable.
			solAssert(false, "");
	}
	m_context << endTag;
}

bool ContractCompiler::visit(TryCatchClause const& _clause)
{
	CompilerContext::LocationSetter locationSetter(m_context, _clause);

	unsigned varSize = 0;

	if (_clause.parameters())
		for (ASTPointer<VariableDeclaration> const& varDecl: _clause.parameters()->parameters() | ranges::views::reverse)
		{
			solAssert(varDecl, "");
			varSize += varDecl->annotation().type->sizeOnStack();
			m_context.addVariable(*varDecl, varSize);
		}

	_clause.block().accept(*this);

	m_context.removeVariablesAboveStackHeight(m_context.stackHeight() - varSize);
	CompilerUtils(m_context).popStackSlots(varSize);

	return false;
}

bool ContractCompiler::visit(IfStatement const& _ifStatement)
{
	StackHeightChecker checker(m_context);
	CompilerContext::LocationSetter locationSetter(m_context, _ifStatement);
	compileExpression(_ifStatement.condition());
	m_context << Instruction::ISZERO;
	evmasm::AssemblyItem falseTag = m_context.appendConditionalJump();
	evmasm::AssemblyItem endTag = falseTag;
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

	evmasm::AssemblyItem loopStart = m_context.newTag();
	evmasm::AssemblyItem loopEnd = m_context.newTag();
	m_breakTags.emplace_back(loopEnd, m_context.stackHeight());

	m_context << loopStart;

	if (_whileStatement.isDoWhile())
	{
		evmasm::AssemblyItem condition = m_context.newTag();
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
	evmasm::AssemblyItem loopStart = m_context.newTag();
	evmasm::AssemblyItem loopEnd = m_context.newTag();
	evmasm::AssemblyItem loopNext = m_context.newTag();

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
	{
		Arithmetic previousArithmetic = m_context.arithmetic();
		if (
			*_forStatement.annotation().isSimpleCounterLoop &&
			m_optimiserSettings.simpleCounterForLoopUncheckedIncrement
		)
			m_context.setArithmetic(Arithmetic::Wrapping);
		_forStatement.loopExpression()->accept(*this);
		m_context.setArithmetic(previousArithmetic);
	}

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
		std::vector<ASTPointer<VariableDeclaration>> const& returnParameters =
			_return.annotation().functionReturnParameters->parameters();
		TypePointers types;
		for (auto const& retVariable: returnParameters)
			types.push_back(retVariable->annotation().type);

		Type const* expectedType;
		if (expression->annotation().type->category() == Type::Category::Tuple || types.size() != 1)
			expectedType = TypeProvider::tuple(std::move(types));
		else
			expectedType = types.front();
		compileExpression(*expression, expectedType);

		for (auto const& retVariable: returnParameters | ranges::views::reverse)
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

bool ContractCompiler::visit(RevertStatement const& _revert)
{
	CompilerContext::LocationSetter locationSetter(m_context, _revert);
	StackHeightChecker checker(m_context);
	compileExpression(_revert.errorCall());
	checker.check();
	return false;
}

bool ContractCompiler::visit(VariableDeclarationStatement const& _variableDeclarationStatement)
{
	CompilerContext::LocationSetter locationSetter(m_context, _variableDeclarationStatement);

	// Local variable slots are reserved when their declaration is visited,
	// and freed in the end of their scope.
	for (auto decl: _variableDeclarationStatement.declarations())
		if (decl)
			appendStackVariableInitialisation(*decl, !_variableDeclarationStatement.initialValue());

	StackHeightChecker checker(m_context);
	if (Expression const* expression = _variableDeclarationStatement.initialValue())
	{
		CompilerUtils utils(m_context);
		compileExpression(*expression);
		TypePointers valueTypes;
		if (auto tupleType = dynamic_cast<TupleType const*>(expression->annotation().type))
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
	solAssert(m_context.arithmetic() == Arithmetic::Checked, "Placeholder cannot be used inside unchecked block.");
	appendModifierOrFunctionCode();
	solAssert(m_context.arithmetic() == Arithmetic::Checked, "Arithmetic not reset to 'checked'.");
	checker.check();
	return true;
}

bool ContractCompiler::visit(Block const& _block)
{
	m_context.pushVisitedNodes(&_block);
	if (_block.unchecked())
	{
		solAssert(m_context.arithmetic() == Arithmetic::Checked, "");
		m_context.setArithmetic(Arithmetic::Wrapping);
	}
	storeStackHeight(&_block);
	return true;
}

void ContractCompiler::endVisit(Block const& _block)
{
	if (_block.unchecked())
	{
		solAssert(m_context.arithmetic() == Arithmetic::Wrapping, "");
		m_context.setArithmetic(Arithmetic::Checked);
	}
	// Frees local variables declared in the scope of this block.
	popScopedVariables(&_block);
	m_context.popVisitedNodes();
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
	m_context.appendYulUtilityFunctions(m_optimiserSettings);
}

void ContractCompiler::appendModifierOrFunctionCode()
{
	solAssert(m_currentFunction, "");
	unsigned stackSurplus = 0;
	Block const* codeBlock = nullptr;
	std::vector<VariableDeclaration const*> addedVariables;

	m_modifierDepth++;
	m_context.setModifierDepth(m_modifierDepth);

	if (m_modifierDepth >= m_currentFunction->modifiers().size())
	{
		solAssert(m_currentFunction->isImplemented(), "");
		codeBlock = &m_currentFunction->body();
	}
	else
	{
		ASTPointer<ModifierInvocation> const& modifierInvocation = m_currentFunction->modifiers()[m_modifierDepth];

		// constructor call should be excluded
		if (dynamic_cast<ContractDefinition const*>(modifierInvocation->name().annotation().referencedDeclaration))
			appendModifierOrFunctionCode();
		else
		{
			ModifierDefinition const& referencedModifier = dynamic_cast<ModifierDefinition const&>(
				*modifierInvocation->name().annotation().referencedDeclaration
			);
			VirtualLookup lookup = *modifierInvocation->name().annotation().requiredLookup;
			solAssert(lookup == VirtualLookup::Virtual || lookup == VirtualLookup::Static, "");
			ModifierDefinition const& modifier =
				lookup == VirtualLookup::Virtual ?
				referencedModifier.resolveVirtual(m_context.mostDerivedContract()) :
				referencedModifier;

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
		m_context.setArithmetic(Arithmetic::Checked);

		bool coderV2Outside = m_context.useABICoderV2();
		m_context.setUseABICoderV2(*codeBlock->sourceUnit().annotation().useABICoderV2);

		m_returnTags.emplace_back(m_context.newTag(), m_context.stackHeight());
		codeBlock->accept(*this);

		m_context.setUseABICoderV2(coderV2Outside);

		solAssert(!m_returnTags.empty(), "");
		m_context << m_returnTags.back().first;
		m_returnTags.pop_back();

		CompilerUtils(m_context).popStackSlots(stackSurplus);
		for (auto var: addedVariables)
			m_context.removeVariable(*var);
	}
	m_modifierDepth--;
	m_context.setModifierDepth(m_modifierDepth);
}

void ContractCompiler::appendStackVariableInitialisation(
	VariableDeclaration const& _variable,
	bool _provideDefaultValue
)
{
	CompilerContext::LocationSetter location(m_context, _variable);
	m_context.addVariable(_variable);
	if (!_provideDefaultValue && _variable.type()->dataStoredIn(DataLocation::Memory))
	{
		solAssert(_variable.type()->sizeOnStack() == 1, "");
		m_context << u256(0);
	}
	else
		CompilerUtils(m_context).pushZeroValue(*_variable.annotation().type);
}

void ContractCompiler::compileExpression(Expression const& _expression, Type const* _targetType)
{
	ExpressionCompiler expressionCompiler(m_context, m_optimiserSettings.runOrderLiterals);
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
