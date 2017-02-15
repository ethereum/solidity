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
 * @date 2016
 * Code-generating part of inline assembly.
 */

#include <libsolidity/inlineasm/AsmCodeGen.h>

#include <libsolidity/inlineasm/AsmParser.h>
#include <libsolidity/inlineasm/AsmData.h>
#include <libsolidity/inlineasm/AsmAnalysis.h>

#include <libevmasm/Assembly.h>
#include <libevmasm/SourceLocation.h>
#include <libevmasm/Instruction.h>

#include <libdevcore/CommonIO.h>

#include <boost/range/adaptor/reversed.hpp>

#include <memory>
#include <functional>

using namespace std;
using namespace dev;
using namespace dev::solidity;
using namespace dev::solidity::assembly;

struct GeneratorState
{
	GeneratorState(ErrorList& _errors, eth::Assembly& _assembly):
		errors(_errors), assembly(_assembly) {}

	void addError(Error::Type _type, std::string const& _description, SourceLocation const& _location = SourceLocation())
	{
		errors.push_back(make_shared<Error>(_type, _description, _location));
	}

	size_t newLabelId()
	{
		return assemblyTagToIdentifier(assembly.newTag());
	}

	size_t assemblyTagToIdentifier(eth::AssemblyItem const& _tag) const
	{
		u256 id = _tag.data();
		solAssert(id <= std::numeric_limits<size_t>::max(), "Tag id too large.");
		return size_t(id);
	}

	std::map<assembly::Block const*, shared_ptr<Scope>> scopes;
	ErrorList& errors;
	eth::Assembly& assembly;
};

class CodeTransform: public boost::static_visitor<>
{
public:
	/// Create the code transformer which appends assembly to _state.assembly when called
	/// with parsed assembly data.
	/// @param _identifierAccess used to resolve identifiers external to the inline assembly
	explicit CodeTransform(
		GeneratorState& _state,
		assembly::CodeGenerator::IdentifierAccess const& _identifierAccess = assembly::CodeGenerator::IdentifierAccess()
	):
		m_state(_state)
	{
		if (_identifierAccess)
			m_identifierAccess = _identifierAccess;
		else
			m_identifierAccess = [](assembly::Identifier const&, eth::Assembly&, CodeGenerator::IdentifierContext) { return false; };
		m_currentScope = m_state.scopes[nullptr].get();
	}

	void operator()(assembly::Instruction const& _instruction)
	{
		m_state.assembly.setSourceLocation(_instruction.location);
		m_state.assembly.append(_instruction.instruction);
	}
	void operator()(assembly::Literal const& _literal)
	{
		m_state.assembly.setSourceLocation(_literal.location);
		if (_literal.isNumber)
			m_state.assembly.append(u256(_literal.value));
		else
		{
			solAssert(_literal.value.size() <= 32, "");
			m_state.assembly.append(_literal.value);
		}
	}
	void operator()(assembly::Identifier const& _identifier)
	{
		m_state.assembly.setSourceLocation(_identifier.location);
		// First search internals, then externals.
		if (!m_currentScope->lookup(_identifier.name, Scope::NonconstVisitor(
			[=](Scope::Variable& _var)
			{
				if (!_var.active)
				{
					m_state.addError(
						Error::Type::TypeError,
						"Variable used before it was declared",
						_identifier.location
					);
					m_state.assembly.append(u256(0));
					return;
				}
				int heightDiff = m_state.assembly.deposit() - _var.stackHeight;
				if (heightDiff <= 0 || heightDiff > 16)
				{
					m_state.addError(
						Error::Type::TypeError,
						"Variable inaccessible, too deep inside stack (" + boost::lexical_cast<string>(heightDiff) + ")",
						_identifier.location
					);
					m_state.assembly.append(u256(0));
				}
				else
					m_state.assembly.append(solidity::dupInstruction(heightDiff));
			},
			[=](Scope::Label& _label)
			{
				if (_label.id == Scope::Label::unassignedLabelId)
					_label.id = m_state.newLabelId();
				m_state.assembly.append(eth::AssemblyItem(eth::PushTag, _label.id));
			}
		)))
		{
			m_state.addError(
				Error::Type::DeclarationError,
				"Identifier not found or not unique",
				_identifier.location
			);
			m_state.assembly.append(u256(0));
		}
	}
	void operator()(FunctionalInstruction const& _instr)
	{
		for (auto it = _instr.arguments.rbegin(); it != _instr.arguments.rend(); ++it)
		{
			int height = m_state.assembly.deposit();
			boost::apply_visitor(*this, *it);
			expectDeposit(1, height, locationOf(*it));
		}
		(*this)(_instr.instruction);
	}
	void operator()(assembly::FunctionCall const&)
	{
		solAssert(false, "Function call not removed during desugaring phase.");
	}
	void operator()(Label const& _label)
	{
		solAssert(_label.stackInfo.empty(), "Labels with stack info not yet supported.");
		m_state.assembly.setSourceLocation(_label.location);
		solAssert(m_currentScope->identifiers.count(_label.name), "");
		Scope::Label& label = boost::get<Scope::Label>(m_currentScope->identifiers[_label.name]);
		if (label.id == Scope::Label::unassignedLabelId)
			label.id = m_state.newLabelId();
		m_state.assembly.append(eth::AssemblyItem(eth::PushTag, label.id));
	}
	void operator()(assembly::Assignment const& _assignment)
	{
		m_state.assembly.setSourceLocation(_assignment.location);
		generateAssignment(_assignment.variableName, _assignment.location);
	}
	void operator()(FunctionalAssignment const& _assignment)
	{
		int height = m_state.assembly.deposit();
		boost::apply_visitor(*this, *_assignment.value);
		expectDeposit(1, height, locationOf(*_assignment.value));
		m_state.assembly.setSourceLocation(_assignment.location);
		generateAssignment(_assignment.variableName, _assignment.location);
	}
	void operator()(assembly::VariableDeclaration const& _varDecl)
	{
		int height = m_state.assembly.deposit();
		boost::apply_visitor(*this, *_varDecl.value);
		expectDeposit(1, height, locationOf(*_varDecl.value));
		solAssert(m_currentScope && m_currentScope->identifiers.count(_varDecl.name), "");
		auto& var = boost::get<Scope::Variable>(m_currentScope->identifiers[_varDecl.name]);
		var.stackHeight = height;
		var.active = true;
	}
	void operator()(assembly::Block const& _block)
	{
		solAssert(m_state.scopes.count(&_block), "Scope for block not defined.");
		m_currentScope = m_state.scopes[&_block].get();
		int deposit = m_state.assembly.deposit();
		std::for_each(_block.statements.begin(), _block.statements.end(), boost::apply_visitor(*this));

		m_state.assembly.setSourceLocation(_block.location);

		// pop variables
		for (auto const& identifier: m_currentScope->identifiers)
			if (identifier.second.type() == typeid(Scope::Variable))
				m_state.assembly.append(solidity::Instruction::POP);

		deposit = m_state.assembly.deposit() - deposit;

		// issue warnings for stack height discrepancies
		if (deposit < 0)
		{
			m_state.addError(
				Error::Type::Warning,
				"Inline assembly block is not balanced. It takes " + toString(-deposit) + " item(s) from the stack.",
				_block.location
			);
		}
		else if (deposit > 0)
		{
			m_state.addError(
				Error::Type::Warning,
				"Inline assembly block is not balanced. It leaves " + toString(deposit) + " item(s) on the stack.",
				_block.location
			);
		}

		m_currentScope = m_currentScope->superScope;
	}
	void operator()(assembly::FunctionDefinition const&)
	{
		solAssert(false, "Function definition not removed during desugaring phase.");
	}

private:
	void generateAssignment(assembly::Identifier const& _variableName, SourceLocation const& _location)
	{
		if (!m_currentScope->lookup(_variableName.name, Scope::Visitor(
			[=](Scope::Variable const& _var)
			{
				if (!_var.active)
				{
					m_state.addError(
						Error::Type::TypeError,
						"Variable used before it was declared",
						_location
					);
					m_state.assembly.append(u256(0));
					return;
				}
				int heightDiff = m_state.assembly.deposit() - _var.stackHeight - 1;
				if (heightDiff <= 0 || heightDiff > 16)
					m_state.addError(
						Error::Type::TypeError,
						"Variable inaccessible, too deep inside stack (" + boost::lexical_cast<string>(heightDiff) + ")",
						_location
					);
				else
					m_state.assembly.append(solidity::swapInstruction(heightDiff));
				m_state.assembly.append(solidity::Instruction::POP);
			},
			[=](Scope::Label const&)
			{
				m_state.addError(
					Error::Type::DeclarationError,
					"Label \"" + string(_variableName.name) + "\" used as variable."
				);
			}
		)) && !m_identifierAccess(_variableName, m_state.assembly, CodeGenerator::IdentifierContext::LValue))
			m_state.addError(
				Error::Type::DeclarationError,
				"Identifier \"" + string(_variableName.name) + "\" not found, not unique or not lvalue."
			);
	}

	void expectDeposit(int _deposit, int _oldHeight, SourceLocation const& _location)
	{
		if (m_state.assembly.deposit() != _oldHeight + 1)
			m_state.addError(Error::Type::TypeError,
				"Expected instruction(s) to deposit " +
				boost::lexical_cast<string>(_deposit) +
				" item(s) to the stack, but did deposit " +
				boost::lexical_cast<string>(m_state.assembly.deposit() - _oldHeight) +
				" item(s).",
				_location
			);
	}

	GeneratorState& m_state;
	Scope* m_currentScope;
	assembly::CodeGenerator::IdentifierAccess m_identifierAccess;
};

bool assembly::CodeGenerator::typeCheck(assembly::CodeGenerator::IdentifierAccess const& _identifierAccess)
{
	size_t initialErrorLen = m_errors.size();
	eth::Assembly assembly;
	GeneratorState state(m_errors, assembly);
	if (!(AsmAnalyzer(state.scopes, m_errors))(m_parsedData))
		return false;
	(CodeTransform(state, _identifierAccess))(m_parsedData);
	return m_errors.size() == initialErrorLen;
}

eth::Assembly assembly::CodeGenerator::assemble(assembly::CodeGenerator::IdentifierAccess const& _identifierAccess)
{
	size_t initialErrorLen = m_errors.size();
	eth::Assembly assembly;
	GeneratorState state(m_errors, assembly);
	if (!(AsmAnalyzer(state.scopes, m_errors))(m_parsedData))
		solAssert(false, "Assembly error");
	(CodeTransform(state, _identifierAccess))(m_parsedData);
	solAssert(m_errors.size() == initialErrorLen, "Assembly error");
	return assembly;
}

void assembly::CodeGenerator::assemble(eth::Assembly& _assembly, assembly::CodeGenerator::IdentifierAccess const& _identifierAccess)
{
	size_t initialErrorLen = m_errors.size();
	GeneratorState state(m_errors, _assembly);
	if (!(AsmAnalyzer(state.scopes, m_errors))(m_parsedData))
		solAssert(false, "Assembly error");
	(CodeTransform(state, _identifierAccess))(m_parsedData);
	solAssert(m_errors.size() == initialErrorLen, "Assembly error");
}

