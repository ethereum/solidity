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
#include <boost/range/adaptor/map.hpp>
#include <boost/range/algorithm/count_if.hpp>

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

	AsmAnalyzer::Scopes scopes;
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
		assembly::Block const& _block,
		assembly::CodeGenerator::IdentifierAccess const& _identifierAccess = assembly::CodeGenerator::IdentifierAccess()
	):
		m_state(_state),
		m_scope(*m_state.scopes.at(&_block)),
		m_initialDeposit(m_state.assembly.deposit()),
		m_identifierAccess(_identifierAccess)
	{
		std::for_each(_block.statements.begin(), _block.statements.end(), boost::apply_visitor(*this));

		m_state.assembly.setSourceLocation(_block.location);

		// pop variables
		for (auto const& identifier: m_scope.identifiers)
			if (identifier.second.type() == typeid(Scope::Variable))
				m_state.assembly.append(solidity::Instruction::POP);

		int deposit = m_state.assembly.deposit() - m_initialDeposit;

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
			m_state.assembly.append(u256(h256(_literal.value, h256::FromBinary, h256::AlignLeft)));
		}
	}
	void operator()(assembly::Identifier const& _identifier)
	{
		m_state.assembly.setSourceLocation(_identifier.location);
		// First search internals, then externals.
		if (m_scope.lookup(_identifier.name, Scope::NonconstVisitor(
			[=](Scope::Variable& _var)
			{
				if (int heightDiff = variableHeightDiff(_var, _identifier.location, false))
					m_state.assembly.append(solidity::dupInstruction(heightDiff));
				else
					// Store something to balance the stack
					m_state.assembly.append(u256(0));
			},
			[=](Scope::Label& _label)
			{
				assignLabelIdIfUnset(_label);
				m_state.assembly.append(eth::AssemblyItem(eth::PushTag, _label.id));
			},
			[=](Scope::Function&)
			{
				solAssert(false, "Not yet implemented");
			}
		)))
		{
		}
		else if (!m_identifierAccess || !m_identifierAccess(_identifier, m_state.assembly, CodeGenerator::IdentifierContext::RValue))
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
		m_state.assembly.setSourceLocation(_label.location);
		solAssert(m_scope.identifiers.count(_label.name), "");
		Scope::Label& label = boost::get<Scope::Label>(m_scope.identifiers[_label.name]);
		assignLabelIdIfUnset(label);
		m_state.assembly.append(eth::AssemblyItem(eth::Tag, label.id));
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
		solAssert(m_scope.identifiers.count(_varDecl.name), "");
		auto& var = boost::get<Scope::Variable>(m_scope.identifiers[_varDecl.name]);
		var.stackHeight = height;
		var.active = true;
	}
	void operator()(assembly::Block const& _block)
	{
		CodeTransform(m_state, _block, m_identifierAccess);
	}
	void operator()(assembly::FunctionDefinition const&)
	{
		solAssert(false, "Function definition not removed during desugaring phase.");
	}

private:
	void generateAssignment(assembly::Identifier const& _variableName, SourceLocation const& _location)
	{
		if (m_scope.lookup(_variableName.name, Scope::Visitor(
			[=](Scope::Variable const& _var)
			{
				if (int heightDiff = variableHeightDiff(_var, _location, true))
					m_state.assembly.append(solidity::swapInstruction(heightDiff - 1));
				m_state.assembly.append(solidity::Instruction::POP);
			},
			[=](Scope::Label const&)
			{
				m_state.addError(
					Error::Type::DeclarationError,
					"Label \"" + string(_variableName.name) + "\" used as variable."
				);
			},
			[=](Scope::Function const&)
			{
				m_state.addError(
					Error::Type::DeclarationError,
					"Function \"" + string(_variableName.name) + "\" used as variable."
				);
			}
		)))
		{
		}
		else if (!m_identifierAccess || !m_identifierAccess(_variableName, m_state.assembly, CodeGenerator::IdentifierContext::LValue))
			m_state.addError(
				Error::Type::DeclarationError,
				"Identifier \"" + string(_variableName.name) + "\" not found, not unique or not lvalue."
			);
	}

	/// Determines the stack height difference to the given variables. Automatically generates
	/// errors if it is not yet in scope or the height difference is too large. Returns 0 on
	/// errors and the (positive) stack height difference otherwise.
	int variableHeightDiff(Scope::Variable const& _var, SourceLocation const& _location, bool _forSwap)
	{
		if (!_var.active)
		{
			m_state.addError( Error::Type::TypeError, "Variable used before it was declared", _location);
			return 0;
		}
		int heightDiff = m_state.assembly.deposit() - _var.stackHeight;
		if (heightDiff <= (_forSwap ? 1 : 0) || heightDiff > (_forSwap ? 17 : 16))
		{
			m_state.addError(
				Error::Type::TypeError,
				"Variable inaccessible, too deep inside stack (" + boost::lexical_cast<string>(heightDiff) + ")",
				_location
			);
			return 0;
		}
		else
			return heightDiff;
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

	/// Assigns the label's id to a value taken from eth::Assembly if it has not yet been set.
	void assignLabelIdIfUnset(Scope::Label& _label)
	{
		if (_label.id == Scope::Label::unassignedLabelId)
			_label.id = m_state.newLabelId();
		else if (_label.id == Scope::Label::errorLabelId)
			_label.id = size_t(m_state.assembly.errorTag().data());
	}


	GeneratorState& m_state;
	Scope& m_scope;
	int const m_initialDeposit;
	assembly::CodeGenerator::IdentifierAccess m_identifierAccess;
};

bool assembly::CodeGenerator::typeCheck(assembly::CodeGenerator::IdentifierAccess const& _identifierAccess)
{
	size_t initialErrorLen = m_errors.size();
	eth::Assembly assembly;
	GeneratorState state(m_errors, assembly);
	if (!(AsmAnalyzer(state.scopes, m_errors))(m_parsedData))
		return false;
	CodeTransform(state, m_parsedData, _identifierAccess);
	return m_errors.size() == initialErrorLen;
}

eth::Assembly assembly::CodeGenerator::assemble(assembly::CodeGenerator::IdentifierAccess const& _identifierAccess)
{
	eth::Assembly assembly;
	GeneratorState state(m_errors, assembly);
	if (!(AsmAnalyzer(state.scopes, m_errors))(m_parsedData))
		solAssert(false, "Assembly error");
	CodeTransform(state, m_parsedData, _identifierAccess);
	return assembly;
}

void assembly::CodeGenerator::assemble(eth::Assembly& _assembly, assembly::CodeGenerator::IdentifierAccess const& _identifierAccess)
{
	GeneratorState state(m_errors, _assembly);
	if (!(AsmAnalyzer(state.scopes, m_errors))(m_parsedData))
		solAssert(false, "Assembly error");
	CodeTransform(state, m_parsedData, _identifierAccess);
}
