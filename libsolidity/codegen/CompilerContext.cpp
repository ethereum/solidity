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
 * Utilities for the solidity compiler.
 */

#include <libsolidity/codegen/CompilerContext.h>

#include <libsolidity/ast/AST.h>
#include <libsolidity/codegen/Compiler.h>
#include <libsolidity/codegen/CompilerUtils.h>
#include <libsolidity/interface/Version.h>

#include <libyul/AST.h>
#include <libyul/AsmParser.h>
#include <libyul/AsmPrinter.h>
#include <libyul/AsmAnalysis.h>
#include <libyul/AsmAnalysisInfo.h>
#include <libyul/backends/evm/AsmCodeGen.h>
#include <libyul/backends/evm/EVMDialect.h>
#include <libyul/backends/evm/EVMMetrics.h>
#include <libyul/optimiser/Suite.h>
#include <libyul/Object.h>
#include <libyul/YulString.h>
#include <libyul/Utilities.h>

#include <libsolutil/Whiskers.h>
#include <libsolutil/FunctionSelector.h>
#include <libsolutil/StackTooDeepString.h>

#include <liblangutil/ErrorReporter.h>
#include <liblangutil/Scanner.h>
#include <liblangutil/SourceReferenceFormatter.h>

#include <utility>

// Change to "define" to output all intermediate code
#undef SOL_OUTPUT_ASM


using namespace std;
using namespace solidity;
using namespace solidity::util;
using namespace solidity::evmasm;
using namespace solidity::frontend;
using namespace solidity::langutil;

void CompilerContext::addStateVariable(
	VariableDeclaration const& _declaration,
	u256 const& _storageOffset,
	unsigned _byteOffset
)
{
	m_stateVariables[&_declaration] = make_pair(_storageOffset, _byteOffset);
}

void CompilerContext::addImmutable(VariableDeclaration const& _variable)
{
	solAssert(_variable.immutable(), "Attempted to register a non-immutable variable as immutable.");
	solUnimplementedAssert(_variable.annotation().type->isValueType(), "Only immutable variables of value type are supported.");
	solAssert(m_runtimeContext, "Attempted to register an immutable variable for runtime code generation.");
	m_immutableVariables[&_variable] = CompilerUtils::generalPurposeMemoryStart + *m_reservedMemory;
	solAssert(_variable.annotation().type->memoryHeadSize() == 32, "Memory writes might overlap.");
	*m_reservedMemory += _variable.annotation().type->memoryHeadSize();
}

size_t CompilerContext::immutableMemoryOffset(VariableDeclaration const& _variable) const
{
	solAssert(m_immutableVariables.count(&_variable), "Memory offset of unknown immutable queried.");
	solAssert(m_runtimeContext, "Attempted to fetch the memory offset of an immutable variable during runtime code generation.");
	return m_immutableVariables.at(&_variable);
}

vector<string> CompilerContext::immutableVariableSlotNames(VariableDeclaration const& _variable)
{
	string baseName = to_string(_variable.id());
	solAssert(_variable.annotation().type->sizeOnStack() > 0, "");
	if (_variable.annotation().type->sizeOnStack() == 1)
		return {baseName};
	vector<string> names;
	auto collectSlotNames = [&](string const& _baseName, Type const* type, auto const& _recurse) -> void {
		for (auto const& [slot, type]: type->stackItems())
			if (type)
				_recurse(_baseName + " " + slot, type, _recurse);
			else
				names.emplace_back(_baseName);
	};
	collectSlotNames(baseName, _variable.annotation().type, collectSlotNames);
	return names;
}

size_t CompilerContext::reservedMemory()
{
	solAssert(m_reservedMemory.has_value(), "Reserved memory was used before ");
	size_t reservedMemory = *m_reservedMemory;
	m_reservedMemory = std::nullopt;
	return reservedMemory;
}

void CompilerContext::startFunction(Declaration const& _function)
{
	m_functionCompilationQueue.startFunction(_function);
	*this << functionEntryLabel(_function);
}

void CompilerContext::callLowLevelFunction(
	string const& _name,
	unsigned _inArgs,
	unsigned _outArgs,
	function<void(CompilerContext&)> const& _generator
)
{
	evmasm::AssemblyItem retTag = pushNewTag();
	CompilerUtils(*this).moveIntoStack(_inArgs);

	*this << lowLevelFunctionTag(_name, _inArgs, _outArgs, _generator);

	appendJump(evmasm::AssemblyItem::JumpType::IntoFunction);
	adjustStackOffset(static_cast<int>(_outArgs) - 1 - static_cast<int>(_inArgs));
	*this << retTag.tag();
}

void CompilerContext::callYulFunction(
	string const& _name,
	unsigned _inArgs,
	unsigned _outArgs
)
{
	m_externallyUsedYulFunctions.insert(_name);
	auto const retTag = pushNewTag();
	CompilerUtils(*this).moveIntoStack(_inArgs);
	appendJumpTo(namedTag(_name, _inArgs, _outArgs, {}), evmasm::AssemblyItem::JumpType::IntoFunction);
	adjustStackOffset(static_cast<int>(_outArgs) - 1 - static_cast<int>(_inArgs));
	*this << retTag.tag();
}

evmasm::AssemblyItem CompilerContext::lowLevelFunctionTag(
	string const& _name,
	unsigned _inArgs,
	unsigned _outArgs,
	function<void(CompilerContext&)> const& _generator
)
{
	auto it = m_lowLevelFunctions.find(_name);
	if (it == m_lowLevelFunctions.end())
	{
		evmasm::AssemblyItem tag = newTag().pushTag();
		m_lowLevelFunctions.insert(make_pair(_name, tag));
		m_lowLevelFunctionGenerationQueue.push(make_tuple(_name, _inArgs, _outArgs, _generator));
		return tag;
	}
	else
		return it->second;
}

void CompilerContext::appendMissingLowLevelFunctions()
{
	while (!m_lowLevelFunctionGenerationQueue.empty())
	{
		string name;
		unsigned inArgs;
		unsigned outArgs;
		function<void(CompilerContext&)> generator;
		tie(name, inArgs, outArgs, generator) = m_lowLevelFunctionGenerationQueue.front();
		m_lowLevelFunctionGenerationQueue.pop();

		setStackOffset(static_cast<int>(inArgs) + 1);
		*this << m_lowLevelFunctions.at(name).tag();
		generator(*this);
		CompilerUtils(*this).moveToStackTop(outArgs);
		appendJump(evmasm::AssemblyItem::JumpType::OutOfFunction);
		solAssert(stackHeight() == outArgs, "Invalid stack height in low-level function " + name + ".");
	}
}

void CompilerContext::appendYulUtilityFunctions(OptimiserSettings const& _optimiserSettings)
{
	solAssert(!m_appendYulUtilityFunctionsRan, "requestedYulFunctions called more than once.");
	m_appendYulUtilityFunctionsRan = true;

	string code = m_yulFunctionCollector.requestedFunctions();
	if (!code.empty())
	{
		appendInlineAssembly(
			yul::reindent("{\n" + move(code) + "\n}"),
			{},
			m_externallyUsedYulFunctions,
			true,
			_optimiserSettings,
			yulUtilityFileName()
		);
		solAssert(!m_generatedYulUtilityCode.empty(), "");
	}
}

void CompilerContext::addVariable(
	VariableDeclaration const& _declaration,
	unsigned _offsetToCurrent
)
{
	solAssert(m_asm->deposit() >= 0 && unsigned(m_asm->deposit()) >= _offsetToCurrent, "");
	unsigned sizeOnStack = _declaration.annotation().type->sizeOnStack();
	// Variables should not have stack size other than [1, 2],
	// but that might change when new types are introduced.
	solAssert(sizeOnStack == 1 || sizeOnStack == 2, "");
	m_localVariables[&_declaration].push_back(unsigned(m_asm->deposit()) - _offsetToCurrent);
}

void CompilerContext::removeVariable(Declaration const& _declaration)
{
	solAssert(m_localVariables.count(&_declaration) && !m_localVariables[&_declaration].empty(), "");
	m_localVariables[&_declaration].pop_back();
	if (m_localVariables[&_declaration].empty())
		m_localVariables.erase(&_declaration);
}

void CompilerContext::removeVariablesAboveStackHeight(unsigned _stackHeight)
{
	vector<Declaration const*> toRemove;
	for (auto _var: m_localVariables)
	{
		solAssert(!_var.second.empty(), "");
		solAssert(_var.second.back() <= stackHeight(), "");
		if (_var.second.back() >= _stackHeight)
			toRemove.push_back(_var.first);
	}
	for (auto _var: toRemove)
		removeVariable(*_var);
}

unsigned CompilerContext::numberOfLocalVariables() const
{
	return static_cast<unsigned>(m_localVariables.size());
}

shared_ptr<evmasm::Assembly> CompilerContext::compiledContract(ContractDefinition const& _contract) const
{
	auto ret = m_otherCompilers.find(&_contract);
	solAssert(ret != m_otherCompilers.end(), "Compiled contract not found.");
	return ret->second->assemblyPtr();
}

shared_ptr<evmasm::Assembly> CompilerContext::compiledContractRuntime(ContractDefinition const& _contract) const
{
	auto ret = m_otherCompilers.find(&_contract);
	solAssert(ret != m_otherCompilers.end(), "Compiled contract not found.");
	return ret->second->runtimeAssemblyPtr();
}

bool CompilerContext::isLocalVariable(Declaration const* _declaration) const
{
	return !!m_localVariables.count(_declaration);
}

evmasm::AssemblyItem CompilerContext::functionEntryLabel(Declaration const& _declaration)
{
	return m_functionCompilationQueue.entryLabel(_declaration, *this);
}

evmasm::AssemblyItem CompilerContext::functionEntryLabelIfExists(Declaration const& _declaration) const
{
	return m_functionCompilationQueue.entryLabelIfExists(_declaration);
}

FunctionDefinition const& CompilerContext::superFunction(FunctionDefinition const& _function, ContractDefinition const& _base)
{
	solAssert(m_mostDerivedContract, "No most derived contract set.");
	ContractDefinition const* super = _base.superContract(mostDerivedContract());
	solAssert(super, "Super contract not available.");

	FunctionDefinition const& resolvedFunction = _function.resolveVirtual(mostDerivedContract(), super);
	solAssert(resolvedFunction.isImplemented(), "");

	return resolvedFunction;
}

ContractDefinition const& CompilerContext::mostDerivedContract() const
{
	solAssert(m_mostDerivedContract, "Most derived contract not set.");
	return *m_mostDerivedContract;
}

Declaration const* CompilerContext::nextFunctionToCompile() const
{
	return m_functionCompilationQueue.nextFunctionToCompile();
}

unsigned CompilerContext::baseStackOffsetOfVariable(Declaration const& _declaration) const
{
	auto res = m_localVariables.find(&_declaration);
	solAssert(res != m_localVariables.end(), "Variable not found on stack.");
	solAssert(!res->second.empty(), "");
	return res->second.back();
}

unsigned CompilerContext::baseToCurrentStackOffset(unsigned _baseOffset) const
{
	return static_cast<unsigned>(m_asm->deposit()) - _baseOffset - 1;
}

unsigned CompilerContext::currentToBaseStackOffset(unsigned _offset) const
{
	return static_cast<unsigned>(m_asm->deposit()) - _offset - 1;
}

pair<u256, unsigned> CompilerContext::storageLocationOfVariable(Declaration const& _declaration) const
{
	auto it = m_stateVariables.find(&_declaration);
	solAssert(it != m_stateVariables.end(), "Variable not found in storage.");
	return it->second;
}

CompilerContext& CompilerContext::appendJump(evmasm::AssemblyItem::JumpType _jumpType)
{
	evmasm::AssemblyItem item(Instruction::JUMP);
	item.setJumpType(_jumpType);
	return *this << item;
}

CompilerContext& CompilerContext::appendPanic(util::PanicCode _code)
{
	callYulFunction(utilFunctions().panicFunction(_code), 0, 0);
	return *this;
}

CompilerContext& CompilerContext::appendConditionalPanic(util::PanicCode _code)
{
	*this << Instruction::ISZERO;
	evmasm::AssemblyItem afterTag = appendConditionalJump();
	appendPanic(_code);
	*this << afterTag;
	return *this;
}

CompilerContext& CompilerContext::appendRevert(string const& _message)
{
	appendInlineAssembly("{ " + revertReasonIfDebug(_message) + " }");
	return *this;
}

CompilerContext& CompilerContext::appendConditionalRevert(bool _forwardReturnData, string const& _message)
{
	if (_forwardReturnData && m_evmVersion.supportsReturndata())
		appendInlineAssembly(R"({
			if condition {
				returndatacopy(0, 0, returndatasize())
				revert(0, returndatasize())
			}
		})", {"condition"});
	else
		appendInlineAssembly("{ if condition { " + revertReasonIfDebug(_message) + " } }", {"condition"});
	*this << Instruction::POP;
	return *this;
}

void CompilerContext::resetVisitedNodes(ASTNode const* _node)
{
	stack<ASTNode const*> newStack;
	newStack.push(_node);
	std::swap(m_visitedNodes, newStack);
	updateSourceLocation();
}

void CompilerContext::appendInlineAssembly(
	string const& _assembly,
	vector<string> const& _localVariables,
	set<string> const& _externallyUsedFunctions,
	bool _system,
	OptimiserSettings const& _optimiserSettings,
	string _sourceName
)
{
	unsigned startStackHeight = stackHeight();

	set<yul::YulString> externallyUsedIdentifiers;
	for (auto const& fun: _externallyUsedFunctions)
		externallyUsedIdentifiers.insert(yul::YulString(fun));
	for (auto const& var: _localVariables)
		externallyUsedIdentifiers.insert(yul::YulString(var));

	yul::ExternalIdentifierAccess identifierAccess;
	identifierAccess.resolve = [&](
		yul::Identifier const& _identifier,
		yul::IdentifierContext,
		bool _insideFunction
	) -> bool
	{
		if (_insideFunction)
			return false;
		return util::contains(_localVariables, _identifier.name.str());
	};
	identifierAccess.generateCode = [&](
		yul::Identifier const& _identifier,
		yul::IdentifierContext _context,
		yul::AbstractAssembly& _assembly
	)
	{
		solAssert(_context == yul::IdentifierContext::RValue || _context == yul::IdentifierContext::LValue, "");
		auto it = std::find(_localVariables.begin(), _localVariables.end(), _identifier.name.str());
		solAssert(it != _localVariables.end(), "");
		auto stackDepth = static_cast<size_t>(distance(it, _localVariables.end()));
		size_t stackDiff = static_cast<size_t>(_assembly.stackHeight()) - startStackHeight + stackDepth;
		if (_context == yul::IdentifierContext::LValue)
			stackDiff -= 1;
		if (stackDiff < 1 || stackDiff > 16)
			BOOST_THROW_EXCEPTION(
				StackTooDeepError() <<
				errinfo_sourceLocation(nativeLocationOf(_identifier)) <<
				util::errinfo_comment(util::stackTooDeepString)
			);
		if (_context == yul::IdentifierContext::RValue)
			_assembly.appendInstruction(dupInstruction(static_cast<unsigned>(stackDiff)));
		else
		{
			_assembly.appendInstruction(swapInstruction(static_cast<unsigned>(stackDiff)));
			_assembly.appendInstruction(Instruction::POP);
		}
	};

	ErrorList errors;
	ErrorReporter errorReporter(errors);
	langutil::CharStream charStream(_assembly, _sourceName);
	yul::EVMDialect const& dialect = yul::EVMDialect::strictAssemblyForEVM(m_evmVersion);
	optional<langutil::SourceLocation> locationOverride;
	if (!_system)
		locationOverride = m_asm->currentSourceLocation();
	shared_ptr<yul::Block> parserResult =
		yul::Parser(errorReporter, dialect, std::move(locationOverride))
		.parse(charStream);
#ifdef SOL_OUTPUT_ASM
	cout << yul::AsmPrinter(&dialect)(*parserResult) << endl;
#endif

	auto reportError = [&](string const& _context)
	{
		string message =
			"Error parsing/analyzing inline assembly block:\n" +
			_context + "\n"
			"------------------ Input: -----------------\n" +
			_assembly + "\n"
			"------------------ Errors: ----------------\n";
		for (auto const& error: errorReporter.errors())
			// TODO if we have "locationOverride", it will be the wrong char stream,
			// but we do not have access to the solidity scanner.
			message += SourceReferenceFormatter::formatErrorInformation(*error, charStream);
		message += "-------------------------------------------\n";

		solAssert(false, message);
	};

	yul::AsmAnalysisInfo analysisInfo;
	bool analyzerResult = false;
	if (parserResult)
		analyzerResult = yul::AsmAnalyzer(
			analysisInfo,
			errorReporter,
			dialect,
			identifierAccess.resolve
		).analyze(*parserResult);
	if (!parserResult || !errorReporter.errors().empty() || !analyzerResult)
		reportError("Invalid assembly generated by code generator.");

	// Several optimizer steps cannot handle externally supplied stack variables,
	// so we essentially only optimize the ABI functions.
	if (_optimiserSettings.runYulOptimiser && _localVariables.empty())
	{
		yul::Object obj;
		obj.code = parserResult;
		obj.analysisInfo = make_shared<yul::AsmAnalysisInfo>(analysisInfo);

		solAssert(!dialect.providesObjectAccess());
		optimizeYul(obj, dialect, _optimiserSettings, externallyUsedIdentifiers);

		if (_system)
		{
			// Store as generated sources, but first re-parse to update the source references.
			solAssert(m_generatedYulUtilityCode.empty(), "");
			m_generatedYulUtilityCode = yul::AsmPrinter(dialect)(*obj.code);
			string code = yul::AsmPrinter{dialect}(*obj.code);
			langutil::CharStream charStream(m_generatedYulUtilityCode, _sourceName);
			obj.code = yul::Parser(errorReporter, dialect).parse(charStream);
			*obj.analysisInfo = yul::AsmAnalyzer::analyzeStrictAssertCorrect(dialect, obj);
		}

		analysisInfo = std::move(*obj.analysisInfo);
		parserResult = std::move(obj.code);

#ifdef SOL_OUTPUT_ASM
		cout << "After optimizer:" << endl;
		cout << yul::AsmPrinter(&dialect)(*parserResult) << endl;
#endif
	}
	else if (_system)
	{
		// Store as generated source.
		solAssert(m_generatedYulUtilityCode.empty(), "");
		m_generatedYulUtilityCode = _assembly;
	}

	if (!errorReporter.errors().empty())
		reportError("Failed to analyze inline assembly block.");

	solAssert(errorReporter.errors().empty(), "Failed to analyze inline assembly block.");
	yul::CodeGenerator::assemble(
		*parserResult,
		analysisInfo,
		*m_asm,
		m_evmVersion,
		identifierAccess.generateCode,
		_system,
		_optimiserSettings.optimizeStackAllocation
	);

	// Reset the source location to the one of the node (instead of the CODEGEN source location)
	updateSourceLocation();
}


void CompilerContext::optimizeYul(yul::Object& _object, yul::EVMDialect const& _dialect, OptimiserSettings const& _optimiserSettings, std::set<yul::YulString> const& _externalIdentifiers)
{
#ifdef SOL_OUTPUT_ASM
	cout << yul::AsmPrinter(*dialect)(*_object.code) << endl;
#endif

	bool const isCreation = runtimeContext() != nullptr;
	yul::GasMeter meter(_dialect, isCreation, _optimiserSettings.expectedExecutionsPerDeployment);
	yul::OptimiserSuite::run(
		_dialect,
		&meter,
		_object,
		_optimiserSettings.optimizeStackAllocation,
		_optimiserSettings.yulOptimiserSteps,
		isCreation? nullopt : make_optional(_optimiserSettings.expectedExecutionsPerDeployment),
		_externalIdentifiers
	);

#ifdef SOL_OUTPUT_ASM
	cout << "After optimizer:" << endl;
	cout << yul::AsmPrinter(*dialect)(*object.code) << endl;
#endif
}

string CompilerContext::revertReasonIfDebug(string const& _message)
{
	return YulUtilFunctions::revertReasonIfDebugBody(
		m_revertStrings,
		"mload(" + to_string(CompilerUtils::freeMemoryPointer) + ")",
		_message
	);
}

void CompilerContext::updateSourceLocation()
{
	m_asm->setSourceLocation(m_visitedNodes.empty() ? SourceLocation() : m_visitedNodes.top()->location());
}

evmasm::Assembly::OptimiserSettings CompilerContext::translateOptimiserSettings(OptimiserSettings const& _settings)
{
	// Constructing it this way so that we notice changes in the fields.
	evmasm::Assembly::OptimiserSettings asmSettings{false,  false, false, false, false, false, m_evmVersion, 0};
	asmSettings.runInliner = _settings.runInliner;
	asmSettings.runJumpdestRemover = _settings.runJumpdestRemover;
	asmSettings.runPeephole = _settings.runPeephole;
	asmSettings.runDeduplicate = _settings.runDeduplicate;
	asmSettings.runCSE = _settings.runCSE;
	asmSettings.runConstantOptimiser = _settings.runConstantOptimiser;
	asmSettings.expectedExecutionsPerDeployment = _settings.expectedExecutionsPerDeployment;
	asmSettings.evmVersion = m_evmVersion;
	return asmSettings;
}

evmasm::AssemblyItem CompilerContext::FunctionCompilationQueue::entryLabel(
	Declaration const& _declaration,
	CompilerContext& _context
)
{
	auto res = m_entryLabels.find(&_declaration);
	if (res == m_entryLabels.end())
	{
		size_t params = 0;
		size_t returns = 0;
		if (auto const* function = dynamic_cast<FunctionDefinition const*>(&_declaration))
		{
			FunctionType functionType(*function, FunctionType::Kind::Internal);
			params = CompilerUtils::sizeOnStack(functionType.parameterTypes());
			returns = CompilerUtils::sizeOnStack(functionType.returnParameterTypes());
		}

		// some name that cannot clash with yul function names.
		string labelName = "@" + _declaration.name() + "_" + to_string(_declaration.id());
		evmasm::AssemblyItem tag = _context.namedTag(
			labelName,
			params,
			returns,
			_declaration.id()
		);
		m_entryLabels.insert(make_pair(&_declaration, tag));
		m_functionsToCompile.push(&_declaration);
		return tag.tag();
	}
	else
		return res->second.tag();

}

evmasm::AssemblyItem CompilerContext::FunctionCompilationQueue::entryLabelIfExists(Declaration const& _declaration) const
{
	auto res = m_entryLabels.find(&_declaration);
	return res == m_entryLabels.end() ? evmasm::AssemblyItem(evmasm::UndefinedItem) : res->second.tag();
}

Declaration const* CompilerContext::FunctionCompilationQueue::nextFunctionToCompile() const
{
	while (!m_functionsToCompile.empty())
	{
		if (m_alreadyCompiledFunctions.count(m_functionsToCompile.front()))
			m_functionsToCompile.pop();
		else
			return m_functionsToCompile.front();
	}
	return nullptr;
}

void CompilerContext::FunctionCompilationQueue::startFunction(Declaration const& _function)
{
	if (!m_functionsToCompile.empty() && m_functionsToCompile.front() == &_function)
		m_functionsToCompile.pop();
	m_alreadyCompiledFunctions.insert(&_function);
}
