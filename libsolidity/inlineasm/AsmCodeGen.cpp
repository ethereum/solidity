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
#include <memory>
#include <functional>
#include <libdevcore/CommonIO.h>
#include <libevmasm/Assembly.h>
#include <libevmasm/SourceLocation.h>
#include <libevmasm/Instruction.h>
#include <libsolidity/inlineasm/AsmParser.h>
#include <libsolidity/inlineasm/AsmData.h>

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
		auto err = make_shared<Error>(_type);
		if (!_location.isEmpty())
			*err << errinfo_sourceLocation(_location);
		*err << errinfo_comment(_description);
		errors.push_back(err);
	}

	int const* findVariable(string const& _variableName) const
	{
		auto localVariable = find_if(
			variables.rbegin(),
			variables.rend(),
			[&](pair<string, int> const& _var) { return _var.first == _variableName; }
		);
		return localVariable != variables.rend() ? &localVariable->second : nullptr;
	}
	eth::AssemblyItem const* findLabel(string const& _labelName) const
	{
		auto label = find_if(
			labels.begin(),
			labels.end(),
			[&](pair<string, eth::AssemblyItem> const& _label) { return _label.first == _labelName; }
		);
		return label != labels.end() ? &label->second : nullptr;
	}

	map<string, eth::AssemblyItem> labels;
	vector<pair<string, int>> variables; ///< name plus stack height
	ErrorList& errors;
	eth::Assembly& assembly;
};

/**
 * Scans the inline assembly data for labels, creates tags in the assembly and searches for
 * duplicate labels.
 */
class LabelOrganizer: public boost::static_visitor<>
{
public:
	LabelOrganizer(GeneratorState& _state): m_state(_state)
	{
		// Make the Solidity ErrorTag available to inline assembly
		m_state.labels.insert(make_pair("invalidJumpLabel", m_state.assembly.errorTag()));
	}

	template <class T>
	void operator()(T const& /*_item*/) { }
	void operator()(Label const& _item)
	{
		if (m_state.labels.count(_item.name))
			//@TODO secondary location
			m_state.addError(
				Error::Type::DeclarationError,
				"Label " + _item.name + " declared twice.",
				_item.location
			);
		m_state.labels.insert(make_pair(_item.name, m_state.assembly.newTag()));
	}
	void operator()(assembly::Block const& _block)
	{
		std::for_each(_block.statements.begin(), _block.statements.end(), boost::apply_visitor(*this));
	}

private:
	GeneratorState& m_state;
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
		else if (_literal.value.size() > 32)
		{
			m_state.addError(
				Error::Type::TypeError,
				"String literal too long (" + boost::lexical_cast<string>(_literal.value.size()) + " > 32)"
			);
			m_state.assembly.append(u256(0));
		}
		else
			m_state.assembly.append(_literal.value);
	}
	void operator()(assembly::Identifier const& _identifier)
	{
		m_state.assembly.setSourceLocation(_identifier.location);
		// First search local variables, then labels, then externals.
		if (int const* stackHeight = m_state.findVariable(_identifier.name))
		{
			int heightDiff = m_state.assembly.deposit() - *stackHeight;
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
			return;
		}
		else if (eth::AssemblyItem const* label = m_state.findLabel(_identifier.name))
			m_state.assembly.append(label->pushTag());
		else if (!m_identifierAccess(_identifier, m_state.assembly, CodeGenerator::IdentifierContext::RValue))
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
	void operator()(Label const& _label)
	{
		m_state.assembly.setSourceLocation(_label.location);
		m_state.assembly.append(m_state.labels.at(_label.name));
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
		m_state.variables.push_back(make_pair(_varDecl.name, height));
	}
	void operator()(assembly::Block const& _block)
	{
		size_t numVariables = m_state.variables.size();
		int deposit = m_state.assembly.deposit();
		std::for_each(_block.statements.begin(), _block.statements.end(), boost::apply_visitor(*this));

		// pop variables
		while (m_state.variables.size() > numVariables)
		{
			m_state.assembly.append(solidity::Instruction::POP);
			m_state.variables.pop_back();
		}

		m_state.assembly.setSourceLocation(_block.location);

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

	}

private:
	void generateAssignment(assembly::Identifier const& _variableName, SourceLocation const& _location)
	{
		if (int const* stackHeight = m_state.findVariable(_variableName.name))
		{
			int heightDiff = m_state.assembly.deposit() - *stackHeight - 1;
			if (heightDiff <= 0 || heightDiff > 16)
				m_state.addError(
					Error::Type::TypeError,
					"Variable inaccessible, too deep inside stack (" + boost::lexical_cast<string>(heightDiff) + ")",
					_location
				);
			else
				m_state.assembly.append(solidity::swapInstruction(heightDiff));
			m_state.assembly.append(solidity::Instruction::POP);
			return;
		}
		else if (!m_identifierAccess(_variableName, m_state.assembly, CodeGenerator::IdentifierContext::LValue))
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
	assembly::CodeGenerator::IdentifierAccess m_identifierAccess;
};

bool assembly::CodeGenerator::typeCheck(assembly::CodeGenerator::IdentifierAccess const& _identifierAccess)
{
	size_t initialErrorLen = m_errors.size();
	eth::Assembly assembly;
	GeneratorState state(m_errors, assembly);
	(LabelOrganizer(state))(m_parsedData);
	(CodeTransform(state, _identifierAccess))(m_parsedData);
	return m_errors.size() == initialErrorLen;
}

eth::Assembly assembly::CodeGenerator::assemble(assembly::CodeGenerator::IdentifierAccess const& _identifierAccess)
{
	eth::Assembly assembly;
	GeneratorState state(m_errors, assembly);
	(LabelOrganizer(state))(m_parsedData);
	(CodeTransform(state, _identifierAccess))(m_parsedData);
	return assembly;
}

void assembly::CodeGenerator::assemble(eth::Assembly& _assembly, assembly::CodeGenerator::IdentifierAccess const& _identifierAccess)
{
	GeneratorState state(m_errors, _assembly);
	(LabelOrganizer(state))(m_parsedData);
	(CodeTransform(state, _identifierAccess))(m_parsedData);
}

