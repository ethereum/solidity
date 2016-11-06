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
 * Utilities for the solidity compiler.
 */

#include <libsolidity/codegen/CompilerContext.h>
#include <utility>
#include <numeric>
#include <boost/algorithm/string/replace.hpp>
#include <libsolidity/ast/AST.h>
#include <libsolidity/codegen/Compiler.h>
#include <libsolidity/interface/Version.h>
#include <libsolidity/inlineasm/AsmData.h>
#include <libsolidity/inlineasm/AsmStack.h>

using namespace std;

namespace dev
{
namespace solidity
{

void CompilerContext::addMagicGlobal(MagicVariableDeclaration const& _declaration)
{
	m_magicGlobals.insert(&_declaration);
}

void CompilerContext::addStateVariable(
	VariableDeclaration const& _declaration,
	u256 const& _storageOffset,
	unsigned _byteOffset
)
{
	m_stateVariables[&_declaration] = make_pair(_storageOffset, _byteOffset);
}

void CompilerContext::startFunction(Declaration const& _function)
{
	m_functionCompilationQueue.startFunction(_function);
	*this << functionEntryLabel(_function);
}

void CompilerContext::addVariable(VariableDeclaration const& _declaration,
								  unsigned _offsetToCurrent)
{
	solAssert(m_asm.deposit() >= 0 && unsigned(m_asm.deposit()) >= _offsetToCurrent, "");
	m_localVariables[&_declaration] = unsigned(m_asm.deposit()) - _offsetToCurrent;
}

void CompilerContext::removeVariable(VariableDeclaration const& _declaration)
{
	solAssert(!!m_localVariables.count(&_declaration), "");
	m_localVariables.erase(&_declaration);
}

eth::AssemblyMutation const& CompilerContext::compiledContract(const ContractDefinition& _contract) const
{
	auto ret = m_compiledContracts.find(&_contract);
	solAssert(ret != m_compiledContracts.end(), "Compiled contract not found.");
	return *ret->second;
}

bool CompilerContext::isLocalVariable(Declaration const* _declaration) const
{
	return !!m_localVariables.count(_declaration);
}

eth::AssemblyItem CompilerContext::functionEntryLabel(Declaration const& _declaration)
{
	return m_functionCompilationQueue.entryLabel(_declaration, *this);
}

eth::AssemblyItem CompilerContext::functionEntryLabelIfExists(Declaration const& _declaration) const
{
	return m_functionCompilationQueue.entryLabelIfExists(_declaration);
}

eth::AssemblyItem CompilerContext::virtualFunctionEntryLabel(FunctionDefinition const& _function)
{
	// Libraries do not allow inheritance and their functions can be inlined, so we should not
	// search the inheritance hierarchy (which will be the wrong one in case the function
	// is inlined).
	if (auto scope = dynamic_cast<ContractDefinition const*>(_function.scope()))
		if (scope->isLibrary())
			return functionEntryLabel(_function);
	solAssert(!m_inheritanceHierarchy.empty(), "No inheritance hierarchy set.");
	return virtualFunctionEntryLabel(_function, m_inheritanceHierarchy.begin());
}

eth::AssemblyItem CompilerContext::superFunctionEntryLabel(FunctionDefinition const& _function, ContractDefinition const& _base)
{
	solAssert(!m_inheritanceHierarchy.empty(), "No inheritance hierarchy set.");
	return virtualFunctionEntryLabel(_function, superContract(_base));
}

FunctionDefinition const* CompilerContext::nextConstructor(ContractDefinition const& _contract) const
{
	vector<ContractDefinition const*>::const_iterator it = superContract(_contract);
	for (; it != m_inheritanceHierarchy.end(); ++it)
		if ((*it)->constructor())
			return (*it)->constructor();

	return nullptr;
}

Declaration const* CompilerContext::nextFunctionToCompile() const
{
	return m_functionCompilationQueue.nextFunctionToCompile();
}

ModifierDefinition const& CompilerContext::functionModifier(string const& _name) const
{
	solAssert(!m_inheritanceHierarchy.empty(), "No inheritance hierarchy set.");
	for (ContractDefinition const* contract: m_inheritanceHierarchy)
		for (ModifierDefinition const* modifier: contract->functionModifiers())
			if (modifier->name() == _name)
				return *modifier;
	BOOST_THROW_EXCEPTION(InternalCompilerError()
		<< errinfo_comment("Function modifier " + _name + " not found."));
}

unsigned CompilerContext::baseStackOffsetOfVariable(Declaration const& _declaration) const
{
	auto res = m_localVariables.find(&_declaration);
	solAssert(res != m_localVariables.end(), "Variable not found on stack.");
	return res->second;
}

unsigned CompilerContext::baseToCurrentStackOffset(unsigned _baseOffset) const
{
	return m_asm.deposit() - _baseOffset - 1;
}

unsigned CompilerContext::currentToBaseStackOffset(unsigned _offset) const
{
	return m_asm.deposit() - _offset - 1;
}

pair<u256, unsigned> CompilerContext::storageLocationOfVariable(const Declaration& _declaration) const
{
	auto it = m_stateVariables.find(&_declaration);
	solAssert(it != m_stateVariables.end(), "Variable not found in storage.");
	return it->second;
}

CompilerContext& CompilerContext::appendJump(eth::AssemblyItem::JumpType _jumpType)
{
	eth::AssemblyItem item(Instruction::JUMP);
	item.setJumpType(_jumpType);
	return *this << item;
}

void CompilerContext::resetVisitedNodes(ASTNode const* _node)
{
	stack<ASTNode const*> newStack;
	newStack.push(_node);
	std::swap(m_visitedNodes, newStack);
	updateSourceLocation();
}

void CompilerContext::mutateCompareOperatorCode(BinaryOperation const& _binaryOperation)
{
	Type const& commonType = *_binaryOperation.annotation().commonType;
	Token::Value const c_op = _binaryOperation.getOperator();

	if (c_op == Token::Equal || c_op == Token::NotEqual)
	{
		*this << Instruction::EQ;
		if (c_op == Token::NotEqual)
			*this << Instruction::ISZERO;
	}
	else
	{
		bool isSigned = false;
		if (auto type = dynamic_cast<IntegerType const*>(&commonType))
			isSigned = type->isSigned();

		if (!m_mutate) 
		{
			// don't want to do anything specific for mutation if it is not required.
			appendCompareOperatorCode(_binaryOperation);
			return;
		}

		eth::Assembly bud = m_asm.ordinary();
		appendCompareOperatorCode(_binaryOperation);

		SourceLocation location = _binaryOperation.location();

		switch (c_op)
		{
		case Token::GreaterThanOrEqual:
			bud << (isSigned ? Instruction::SGT : Instruction::GT) 
				<< Instruction::ISZERO;
			addMutant(Token::GreaterThanOrEqual, Token::LessThanOrEqual, bud, location);
			break;
		case Token::LessThanOrEqual:
			bud <<
				(isSigned ? Instruction::SLT : Instruction::LT) <<
				Instruction::ISZERO;
			addMutant(Token::LessThanOrEqual, Token::GreaterThanOrEqual, bud, location);	
			break;
		case Token::GreaterThan:
			bud << (isSigned ? Instruction::SLT : Instruction::LT);
			addMutant(Token::GreaterThan, Token::LessThan, bud, location);
			break;
		case Token::LessThan:
			bud << (isSigned ? Instruction::SGT : Instruction::GT);
			addMutant(Token::LessThan, Token::GreaterThan, bud, location);
			break;
		default:
			BOOST_THROW_EXCEPTION(InternalCompilerError() << errinfo_comment("Unknown comparison operator."));
		}
	}
}

void CompilerContext::mutateArithmeticOperatorCode(BinaryOperation const& _binaryOperation)
{
	Type const& commonType = *_binaryOperation.annotation().commonType;
	Token::Value const c_op = _binaryOperation.getOperator();

	if (!m_mutate) 
	{
		// don't want to do anything specific for mutation if it is not required.
		appendArithmeticOperatorCode(c_op, commonType);
		return;
	}

	IntegerType const& type = dynamic_cast<IntegerType const&>(commonType);
	bool const c_isSigned = type.isSigned();

	eth::Assembly ordinary = m_asm.ordinary();
	appendArithmeticOperatorCode(c_op, commonType);

	SourceLocation const& location = _binaryOperation.location();

	switch (c_op)
	{
	case Token::Add:
		mutateAdd(ordinary, location);
		break;
	case Token::Sub:
		mutateSub(ordinary, location);
		break;
	case Token::Mul:
		mutateMul(ordinary, location);
		break;
	case Token::Div:
	case Token::Mod:
	{
		eth::Assembly bud = ordinary;
		bud << Instruction::DUP2 << Instruction::ISZERO;
		bud.appendJumpI(errorTag());

		if (c_op == Token::Div)
		{
			bud << (c_isSigned ? Instruction::SMOD : Instruction::MOD);
			addMutant(Token::Div, Token::Mod, bud, location);
		}
		else
		{
			bud << (c_isSigned ? Instruction::SDIV : Instruction::DIV);
			addMutant(Token::Mod, Token::Div, bud, location);
		}

		break;
	}
	case Token::Exp:
		mutateExp(ordinary, location);
		break;
	default:
		BOOST_THROW_EXCEPTION(InternalCompilerError() << errinfo_comment("Unknown arithmetic operator."));
	}
}

void CompilerContext::appendArithmeticOperatorCode(Token::Value _operator, Type const& _type)
{
	IntegerType const& type = dynamic_cast<IntegerType const&>(_type);
	bool const c_isSigned = type.isSigned();

	if (_type.category() == Type::Category::FixedPoint)
		solAssert(false, "Not yet implemented - FixedPointType.");

	switch (_operator)
	{
	case Token::Add:
		*this << Instruction::ADD;
		break;
	case Token::Sub:
		*this << Instruction::SUB;
		break;
	case Token::Mul:
		*this << Instruction::MUL;
		break;
	case Token::Div:
	case Token::Mod:
	{
		// Test for division by zero
		*this << Instruction::DUP2 << Instruction::ISZERO;
		appendConditionalJumpTo(errorTag());

		if (_operator == Token::Div)
			*this << (c_isSigned ? Instruction::SDIV : Instruction::DIV);
		else
			*this << (c_isSigned ? Instruction::SMOD : Instruction::MOD);
		break;
	}
	case Token::Exp:
		*this << Instruction::EXP;
		break;
	default:
		BOOST_THROW_EXCEPTION(InternalCompilerError() << errinfo_comment("Unknown arithmetic operator."));
	}
}

void CompilerContext::appendInlineAssembly(
	string const& _assembly,
	vector<string> const& _localVariables,
	map<string, string> const& _replacements
)
{
	string replacedAssembly;
	string const* assembly = &_assembly;
	if (!_replacements.empty())
	{
		replacedAssembly = _assembly;
		for (auto const& replacement: _replacements)
			replacedAssembly = boost::algorithm::replace_all_copy(replacedAssembly, replacement.first, replacement.second);
		assembly = &replacedAssembly;
	}

	unsigned startStackHeight = stackHeight();
	auto identifierAccess = [&](
		assembly::Identifier const& _identifier,
		eth::AssemblyMutation& _assembly,
		assembly::CodeGenerator::IdentifierContext _context
	) {
		auto it = std::find(_localVariables.begin(), _localVariables.end(), _identifier.name);
		if (it == _localVariables.end())
			return false;
		unsigned stackDepth = _localVariables.end() - it;
		int stackDiff = _assembly.deposit() - startStackHeight + stackDepth;
		if (stackDiff < 1 || stackDiff > 16)
			BOOST_THROW_EXCEPTION(
				CompilerError() <<
				errinfo_comment("Stack too deep, try removing local variables.")
			);
		if (_context == assembly::CodeGenerator::IdentifierContext::RValue)
			_assembly.append(dupInstruction(stackDiff));
		else
		{
			_assembly.append(swapInstruction(stackDiff));
			_assembly.append(Instruction::POP);
		}
		return true;
	};

	solAssert(assembly::InlineAssemblyStack().parseAndAssemble(*assembly, m_asm, identifierAccess), "");
}

void CompilerContext::injectVersionStampIntoSub(size_t _subIndex)
{
	m_asm.injectVersionStampIntoSub(_subIndex, binaryVersion());
}

eth::AssemblyItem CompilerContext::virtualFunctionEntryLabel(
	FunctionDefinition const& _function,
	vector<ContractDefinition const*>::const_iterator _searchStart
)
{
	string name = _function.name();
	FunctionType functionType(_function);
	auto it = _searchStart;
	for (; it != m_inheritanceHierarchy.end(); ++it)
		for (FunctionDefinition const* function: (*it)->definedFunctions())
			if (
				function->name() == name &&
				!function->isConstructor() &&
				FunctionType(*function).hasEqualArgumentTypes(functionType)
			)
				return functionEntryLabel(*function);
	solAssert(false, "Super function " + name + " not found.");
	return m_asm.newTag(); // not reached
}

vector<ContractDefinition const*>::const_iterator CompilerContext::superContract(ContractDefinition const& _contract) const
{
	solAssert(!m_inheritanceHierarchy.empty(), "No inheritance hierarchy set.");
	auto it = find(m_inheritanceHierarchy.begin(), m_inheritanceHierarchy.end(), &_contract);
	solAssert(it != m_inheritanceHierarchy.end(), "Base not found in inheritance hierarchy.");
	return ++it;
}

void CompilerContext::updateSourceLocation()
{
	m_asm.setSourceLocation(m_visitedNodes.empty() ? SourceLocation() : m_visitedNodes.top()->location());
}

eth::AssemblyItem CompilerContext::FunctionCompilationQueue::entryLabel(
	Declaration const& _declaration,
	CompilerContext& _context
)
{
	auto res = m_entryLabels.find(&_declaration);
	if (res == m_entryLabels.end())
	{
		eth::AssemblyItem tag(_context.newTag());
		m_entryLabels.insert(make_pair(&_declaration, tag));
		m_functionsToCompile.push(&_declaration);
		return tag.tag();
	}
	else
		return res->second.tag();

}

eth::AssemblyItem CompilerContext::FunctionCompilationQueue::entryLabelIfExists(Declaration const& _declaration) const
{
	auto res = m_entryLabels.find(&_declaration);
	return res == m_entryLabels.end() ? eth::AssemblyItem(eth::UndefinedItem) : res->second.tag();
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

bool CompilerContext::mutate() const
{
	return m_mutate;
}

void CompilerContext::appendCompareOperatorCode(BinaryOperation const& _binaryOperation)
{
	Type const& commonType = *_binaryOperation.annotation().commonType;
	Token::Value const c_op = _binaryOperation.getOperator();

	bool isSigned = false;
	if (auto type = dynamic_cast<IntegerType const*>(&commonType))
		isSigned = type->isSigned();

	switch (c_op)
	{
	case Token::GreaterThanOrEqual:
		*this <<
			(isSigned ? Instruction::SLT : Instruction::LT) <<
			Instruction::ISZERO;
		break;
	case Token::LessThanOrEqual:
		*this <<
			(isSigned ? Instruction::SGT : Instruction::GT) <<
			Instruction::ISZERO;
		break;
	case Token::GreaterThan:
		*this << (isSigned ? Instruction::SGT : Instruction::GT);
		break;
	case Token::LessThan:
		*this << (isSigned ? Instruction::SLT : Instruction::LT);
		break;
	default:
		BOOST_THROW_EXCEPTION(InternalCompilerError() << errinfo_comment("Unknown comparison operator."));
	}
}

void CompilerContext::mutateAdd(eth::Assembly const& _ordinary, SourceLocation const& _location)
{
	map<Token::Value, Instruction> mutations;
	mutations[Token::Sub] = Instruction::SUB;
	mutations[Token::Mul] = Instruction::MUL;
	mutations[Token::Exp] = Instruction::EXP;

	mutate(Token::Add, mutations, _ordinary, _location);
}

void CompilerContext::mutateSub(eth::Assembly const& _ordinary, SourceLocation const& _location)
{
	map<Token::Value, Instruction> mutations;
	mutations[Token::Add] = Instruction::ADD;
	mutations[Token::Mul] = Instruction::MUL;
	mutations[Token::Exp] = Instruction::EXP;

	mutate(Token::Sub, mutations, _ordinary, _location);
}

void CompilerContext::mutateMul(eth::Assembly const& _ordinary, SourceLocation const& _location)
{
	map<Token::Value, Instruction> mutations;
	mutations[Token::Add] = Instruction::ADD;
	mutations[Token::Sub] = Instruction::SUB;
	mutations[Token::Exp] = Instruction::EXP;

	mutate(Token::Mul, mutations, _ordinary, _location);
}

void CompilerContext::mutateExp(eth::Assembly const& _ordinary, SourceLocation const& _location)
{
	map<Token::Value, Instruction> mutations;
	mutations[Token::Add] = Instruction::ADD;
	mutations[Token::Sub] = Instruction::SUB;
	mutations[Token::Mul] = Instruction::MUL;

	mutate(Token::Exp, mutations, _ordinary, _location);
}

void CompilerContext::mutate(Token::Value _original, map<Token::Value, Instruction> const& _mutations, eth::Assembly const& _ordinary, SourceLocation const& _location)
{
	for (auto const& pair : _mutations) {
		eth::Assembly bud = _ordinary;
		bud << pair.second;
		addMutant(_original, pair.first, bud, _location);
	}
}

void CompilerContext::addMutant(
	Token::Value _original, 
	Token::Value _mutated, 
	eth::Assembly const& _bud, 
	SourceLocation const& _location)
{
	stringstream description;
	description
		<<  Token::toString(_original)
		<< " -> "
		<< Token::toString(_mutated);

	eth::AssemblyMutant* mutant = new eth::AssemblyMutant(_bud, description.str(), _location);
	m_asm.addMutant(*mutant);
}

}
}
