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
 * Analyzer part of inline assembly.
 */

#include <libsolidity/inlineasm/AsmAnalysis.h>

#include <libsolidity/inlineasm/AsmData.h>
#include <libsolidity/inlineasm/AsmScopeFiller.h>
#include <libsolidity/inlineasm/AsmScope.h>
#include <libsolidity/inlineasm/AsmAnalysisInfo.h>

#include <libsolidity/interface/Exceptions.h>
#include <libsolidity/interface/Utils.h>

#include <boost/range/adaptor/reversed.hpp>

#include <memory>
#include <functional>

using namespace std;
using namespace dev;
using namespace dev::solidity;
using namespace dev::solidity::assembly;

AsmAnalyzer::AsmAnalyzer(
	AsmAnalysisInfo& _analysisInfo,
	ErrorList& _errors,
	ExternalIdentifierAccess::Resolver const& _resolver
):
	m_resolver(_resolver), m_info(_analysisInfo), m_errors(_errors)
{
}

bool AsmAnalyzer::analyze(Block const& _block)
{
	if (!(ScopeFiller(m_info.scopes, m_errors))(_block))
		return false;

	return (*this)(_block);
}

bool AsmAnalyzer::operator()(Label const& _label)
{
	m_info.stackHeightInfo[&_label] = m_stackHeight;
	return true;
}

bool AsmAnalyzer::operator()(assembly::Instruction const& _instruction)
{
	auto const& info = instructionInfo(_instruction.instruction);
	m_stackHeight += info.ret - info.args;
	m_info.stackHeightInfo[&_instruction] = m_stackHeight;
	return true;
}

bool AsmAnalyzer::operator()(assembly::Literal const& _literal)
{
	++m_stackHeight;
	if (_literal.kind == assembly::LiteralKind::String && _literal.value.size() > 32)
	{
		m_errors.push_back(make_shared<Error>(
			Error::Type::TypeError,
			"String literal too long (" + boost::lexical_cast<std::string>(_literal.value.size()) + " > 32)",
			_literal.location
		));
		return false;
	}
	m_info.stackHeightInfo[&_literal] = m_stackHeight;
	return true;
}

bool AsmAnalyzer::operator()(assembly::Identifier const& _identifier)
{
	size_t numErrorsBefore = m_errors.size();
	bool success = true;
	if (m_currentScope->lookup(_identifier.name, Scope::Visitor(
		[&](Scope::Variable const& _var)
		{
			if (!_var.active)
			{
				m_errors.push_back(make_shared<Error>(
					Error::Type::DeclarationError,
					"Variable " + _identifier.name + " used before it was declared.",
					_identifier.location
				));
				success = false;
			}
			++m_stackHeight;
		},
		[&](Scope::Label const&)
		{
			++m_stackHeight;
		},
		[&](Scope::Function const&)
		{
			m_errors.push_back(make_shared<Error>(
				Error::Type::TypeError,
				"Function " + _identifier.name + " used without being called.",
				_identifier.location
			));
			success = false;
		}
	)))
	{
	}
	else
	{
		size_t stackSize(-1);
		if (m_resolver)
			stackSize = m_resolver(_identifier, IdentifierContext::RValue);
		if (stackSize == size_t(-1))
		{
			// Only add an error message if the callback did not do it.
			if (numErrorsBefore == m_errors.size())
				m_errors.push_back(make_shared<Error>(
					Error::Type::DeclarationError,
					"Identifier not found.",
					_identifier.location
				));
			success = false;
		}
		m_stackHeight += stackSize == size_t(-1) ? 1 : stackSize;
	}
	m_info.stackHeightInfo[&_identifier] = m_stackHeight;
	return success;
}

bool AsmAnalyzer::operator()(FunctionalInstruction const& _instr)
{
	bool success = true;
	for (auto const& arg: _instr.arguments | boost::adaptors::reversed)
	{
		int const stackHeight = m_stackHeight;
		if (!boost::apply_visitor(*this, arg))
			success = false;
		if (!expectDeposit(1, stackHeight, locationOf(arg)))
			success = false;
	}
	// Parser already checks that the number of arguments is correct.
	solAssert(instructionInfo(_instr.instruction.instruction).args == int(_instr.arguments.size()), "");
	if (!(*this)(_instr.instruction))
		success = false;
	m_info.stackHeightInfo[&_instr] = m_stackHeight;
	return success;
}

bool AsmAnalyzer::operator()(assembly::StackAssignment const& _assignment)
{
	bool success = checkAssignment(_assignment.variableName, size_t(-1));
	m_info.stackHeightInfo[&_assignment] = m_stackHeight;
	return success;
}


bool AsmAnalyzer::operator()(assembly::Assignment const& _assignment)
{
	int const stackHeight = m_stackHeight;
	bool success = boost::apply_visitor(*this, *_assignment.value);
	solAssert(m_stackHeight >= stackHeight, "Negative value size.");
	if (!checkAssignment(_assignment.variableName, m_stackHeight - stackHeight))
		success = false;
	m_info.stackHeightInfo[&_assignment] = m_stackHeight;
	return success;
}

bool AsmAnalyzer::operator()(assembly::VariableDeclaration const& _varDecl)
{
	int const expectedItems = _varDecl.variables.size();
	int const stackHeight = m_stackHeight;
	bool success = boost::apply_visitor(*this, *_varDecl.value);
	if ((m_stackHeight - stackHeight) != expectedItems)
	{
		m_errors.push_back(make_shared<Error>(
			Error::Type::DeclarationError,
			"Variable count mismatch.",
			_varDecl.location
		));
		return false;
	}

	for (auto const& variable: _varDecl.variables)
		boost::get<Scope::Variable>(m_currentScope->identifiers.at(variable.name)).active = true;
	m_info.stackHeightInfo[&_varDecl] = m_stackHeight;
	return success;
}

bool AsmAnalyzer::operator()(assembly::FunctionDefinition const& _funDef)
{
	Scope& bodyScope = scope(&_funDef.body);
	for (auto const& var: _funDef.arguments + _funDef.returns)
		boost::get<Scope::Variable>(bodyScope.identifiers.at(var.name)).active = true;

	int const stackHeight = m_stackHeight;
	m_stackHeight = _funDef.arguments.size() + _funDef.returns.size();
	m_virtualVariablesInNextBlock = m_stackHeight;

	bool success = (*this)(_funDef.body);

	m_stackHeight = stackHeight;
	m_info.stackHeightInfo[&_funDef] = m_stackHeight;
	return success;
}

bool AsmAnalyzer::operator()(assembly::FunctionCall const& _funCall)
{
	bool success = true;
	size_t arguments = 0;
	size_t returns = 0;
	if (!m_currentScope->lookup(_funCall.functionName.name, Scope::Visitor(
		[&](Scope::Variable const&)
		{
			m_errors.push_back(make_shared<Error>(
				Error::Type::TypeError,
				"Attempt to call variable instead of function.",
				_funCall.functionName.location
			));
			success = false;
		},
		[&](Scope::Label const&)
		{
			m_errors.push_back(make_shared<Error>(
				Error::Type::TypeError,
				"Attempt to call label instead of function.",
				_funCall.functionName.location
				));
			success = false;
		},
		[&](Scope::Function const& _fun)
		{
			/// TODO: compare types too
			arguments = _fun.arguments.size();
			returns = _fun.returns.size();
		}
	)))
	{
		m_errors.push_back(make_shared<Error>(
			Error::Type::DeclarationError,
			"Function not found.",
			_funCall.functionName.location
		));
		success = false;
	}
	if (success)
	{
		if (_funCall.arguments.size() != arguments)
		{
			m_errors.push_back(make_shared<Error>(
				Error::Type::TypeError,
				"Expected " +
				boost::lexical_cast<string>(arguments) +
				" arguments but got " +
				boost::lexical_cast<string>(_funCall.arguments.size()) +
				".",
				_funCall.functionName.location
				));
			success = false;
		}
	}
	for (auto const& arg: _funCall.arguments | boost::adaptors::reversed)
	{
		int const stackHeight = m_stackHeight;
		if (!boost::apply_visitor(*this, arg))
			success = false;
		if (!expectDeposit(1, stackHeight, locationOf(arg)))
			success = false;
	}
	m_stackHeight += int(returns) - int(arguments);
	m_info.stackHeightInfo[&_funCall] = m_stackHeight;
	return success;
}

bool AsmAnalyzer::operator()(Switch const& _switch)
{
	bool success = true;

	int const initialStackHeight = m_stackHeight;
	if (!boost::apply_visitor(*this, *_switch.expression))
		success = false;
	expectDeposit(1, initialStackHeight, locationOf(*_switch.expression));

	set<tuple<LiteralKind, string>> cases;
	for (auto const& _case: _switch.cases)
	{
		if (_case.value)
		{
			int const initialStackHeight = m_stackHeight;
			if (!(*this)(*_case.value))
				success = false;
			expectDeposit(1, initialStackHeight, _case.value->location);
			m_stackHeight--;

			/// Note: the parser ensures there is only one default case
			auto val = make_tuple(_case.value->kind, _case.value->value);
			if (!cases.insert(val).second)
			{
				m_errors.push_back(make_shared<Error>(
					Error::Type::DeclarationError,
					"Duplicate case defined",
					_case.location
				));
				success = false;
			}
		}

		if (!(*this)(_case.body))
			success = false;
	}

	m_stackHeight--;

	return success;
}

bool AsmAnalyzer::operator()(Block const& _block)
{
	bool success = true;
	m_currentScope = &scope(&_block);

	int const initialStackHeight = m_stackHeight - m_virtualVariablesInNextBlock;
	m_virtualVariablesInNextBlock = 0;

	for (auto const& s: _block.statements)
		if (!boost::apply_visitor(*this, s))
			success = false;

	for (auto const& identifier: scope(&_block).identifiers)
		if (identifier.second.type() == typeid(Scope::Variable))
			--m_stackHeight;

	int const stackDiff = m_stackHeight - initialStackHeight;
	if (stackDiff != 0)
	{
		m_errors.push_back(make_shared<Error>(
			Error::Type::DeclarationError,
			"Unbalanced stack at the end of a block: " +
			(
				stackDiff > 0 ?
				to_string(stackDiff) + string(" surplus item(s).") :
				to_string(-stackDiff) + string(" missing item(s).")
			),
			_block.location
		));
		success = false;
	}

	m_currentScope = m_currentScope->superScope;
	m_info.stackHeightInfo[&_block] = m_stackHeight;
	return success;
}

bool AsmAnalyzer::checkAssignment(assembly::Identifier const& _variable, size_t _valueSize)
{
	bool success = true;
	size_t numErrorsBefore = m_errors.size();
	size_t variableSize(-1);
	if (Scope::Identifier const* var = m_currentScope->lookup(_variable.name))
	{
		// Check that it is a variable
		if (var->type() != typeid(Scope::Variable))
		{
			m_errors.push_back(make_shared<Error>(
				Error::Type::TypeError,
				"Assignment requires variable.",
				_variable.location
			));
			success = false;
		}
		else if (!boost::get<Scope::Variable>(*var).active)
		{
			m_errors.push_back(make_shared<Error>(
				Error::Type::DeclarationError,
				"Variable " + _variable.name + " used before it was declared.",
				_variable.location
			));
			success = false;
		}
		variableSize = 1;
	}
	else if (m_resolver)
		variableSize = m_resolver(_variable, IdentifierContext::LValue);
	if (variableSize == size_t(-1))
	{
		// Only add message if the callback did not.
		if (numErrorsBefore == m_errors.size())
			m_errors.push_back(make_shared<Error>(
				Error::Type::DeclarationError,
				"Variable not found or variable not lvalue.",
				_variable.location
			));
		success = false;
	}
	if (_valueSize == size_t(-1))
		_valueSize = variableSize == size_t(-1) ? 1 : variableSize;

	m_stackHeight -= _valueSize;

	if (_valueSize != variableSize && variableSize != size_t(-1))
	{
		m_errors.push_back(make_shared<Error>(
			Error::Type::TypeError,
			"Variable size (" +
			to_string(variableSize) +
			") and value size (" +
			to_string(_valueSize) +
			") do not match.",
			_variable.location
		));
		success = false;
	}
	return success;
}

bool AsmAnalyzer::expectDeposit(int const _deposit, int const _oldHeight, SourceLocation const& _location)
{
	int stackDiff = m_stackHeight - _oldHeight;
	if (stackDiff != _deposit)
	{
		m_errors.push_back(make_shared<Error>(
			Error::Type::TypeError,
			"Expected instruction(s) to deposit " +
			boost::lexical_cast<string>(_deposit) +
			" item(s) to the stack, but did deposit " +
			boost::lexical_cast<string>(stackDiff) +
			" item(s).",
			_location
		));
		return false;
	}
	else
		return true;
}

Scope& AsmAnalyzer::scope(Block const* _block)
{
	solAssert(m_info.scopes.count(_block) == 1, "Scope requested but not present.");
	auto scopePtr = m_info.scopes.at(_block);
	solAssert(scopePtr, "Scope requested but not present.");
	return *scopePtr;
}
