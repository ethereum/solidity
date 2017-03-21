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
	AsmAnalyzer::Scopes& _scopes,
	ErrorList& _errors,
	ExternalIdentifierAccess::Resolver const& _resolver
):
	m_resolver(_resolver), m_scopes(_scopes), m_errors(_errors)
{
}

bool AsmAnalyzer::analyze(Block const& _block)
{
	if (!(ScopeFiller(m_scopes, m_errors))(_block))
		return false;
	return (*this)(_block);
}

bool AsmAnalyzer::operator()(assembly::Literal const& _literal)
{
	if (!_literal.isNumber && _literal.value.size() > 32)
	{
		m_errors.push_back(make_shared<Error>(
			Error::Type::TypeError,
			"String literal too long (" + boost::lexical_cast<std::string>(_literal.value.size()) + " > 32)"
		));
		return false;
	}
	return true;
}

bool AsmAnalyzer::operator()(assembly::Identifier const& _identifier)
{
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
		},
		[&](Scope::Label const&) {},
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
	else if (!m_resolver || m_resolver(_identifier, IdentifierContext::RValue) == size_t(-1))
	{
		m_errors.push_back(make_shared<Error>(
			Error::Type::DeclarationError,
			"Identifier not found.",
			_identifier.location
		));
		success = false;
	}
	return success;
}

bool AsmAnalyzer::operator()(FunctionalInstruction const& _instr)
{
	bool success = true;
	for (auto const& arg: _instr.arguments | boost::adaptors::reversed)
		if (!boost::apply_visitor(*this, arg))
			success = false;
	if (!(*this)(_instr.instruction))
		success = false;
	return success;
}

bool AsmAnalyzer::operator()(assembly::Assignment const& _assignment)
{
	return checkAssignment(_assignment.variableName);
}

bool AsmAnalyzer::operator()(FunctionalAssignment const& _assignment)
{
	bool success = boost::apply_visitor(*this, *_assignment.value);
	if (!checkAssignment(_assignment.variableName))
		success = false;
	return success;
}

bool AsmAnalyzer::operator()(assembly::VariableDeclaration const& _varDecl)
{
	bool success = boost::apply_visitor(*this, *_varDecl.value);
	boost::get<Scope::Variable>(m_currentScope->identifiers.at(_varDecl.name)).active = true;
	return success;
}

bool AsmAnalyzer::operator()(assembly::FunctionDefinition const& _funDef)
{
	Scope& bodyScope = scope(&_funDef.body);
	for (auto const& var: _funDef.arguments + _funDef.returns)
		boost::get<Scope::Variable>(bodyScope.identifiers.at(var)).active = true;

	return (*this)(_funDef.body);
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
			arguments = _fun.arguments;
			returns = _fun.returns;
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
		//@todo check the number of returns - depends on context and should probably
		// be only done once we have stack height checks
	}
	for (auto const& arg: _funCall.arguments | boost::adaptors::reversed)
		if (!boost::apply_visitor(*this, arg))
			success = false;
	return success;
}

bool AsmAnalyzer::operator()(Block const& _block)
{
	bool success = true;
	m_currentScope = &scope(&_block);

	for (auto const& s: _block.statements)
		if (!boost::apply_visitor(*this, s))
			success = false;

	m_currentScope = m_currentScope->superScope;
	return success;
}

bool AsmAnalyzer::checkAssignment(assembly::Identifier const& _variable)
{
	if (!(*this)(_variable))
		return false;

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
			return false;
		}
	}
	else if (!m_resolver || m_resolver(_variable, IdentifierContext::LValue) == size_t(-1))
	{
		m_errors.push_back(make_shared<Error>(
			Error::Type::DeclarationError,
			"Variable not found.",
			_variable.location
		));
		return false;
	}
	return true;
}

Scope& AsmAnalyzer::scope(Block const* _block)
{
	auto scopePtr = m_scopes.at(_block);
	solAssert(scopePtr, "Scope requested but not present.");
	return *scopePtr;
}
