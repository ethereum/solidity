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
#include <libsolidity/inlineasm/AsmScope.h>
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
	GeneratorState(ErrorList& _errors, AsmAnalyzer::Scopes& _scopes, eth::Assembly& _assembly):
		errors(_errors), scopes(_scopes), assembly(_assembly) {}

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

	ErrorList& errors;
	AsmAnalyzer::Scopes scopes;
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
		assembly::ExternalIdentifierAccess const& _identifierAccess = assembly::ExternalIdentifierAccess()
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

		solAssert(deposit == 0, "Invalid stack height at end of block.");
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
				solAssert(false, "Function not removed during desugaring.");
			}
		)))
		{
			return;
		}
		solAssert(
			m_identifierAccess.generateCode,
			"Identifier not found and no external access available."
		);
		m_identifierAccess.generateCode(_identifier, IdentifierContext::RValue, m_state.assembly);
	}
	void operator()(FunctionalInstruction const& _instr)
	{
		for (auto it = _instr.arguments.rbegin(); it != _instr.arguments.rend(); ++it)
		{
			int height = m_state.assembly.deposit();
			boost::apply_visitor(*this, *it);
			expectDeposit(1, height);
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
		Scope::Label& label = boost::get<Scope::Label>(m_scope.identifiers.at(_label.name));
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
		expectDeposit(1, height);
		m_state.assembly.setSourceLocation(_assignment.location);
		generateAssignment(_assignment.variableName, _assignment.location);
	}
	void operator()(assembly::VariableDeclaration const& _varDecl)
	{
		int height = m_state.assembly.deposit();
		boost::apply_visitor(*this, *_varDecl.value);
		expectDeposit(1, height);
		auto& var = boost::get<Scope::Variable>(m_scope.identifiers.at(_varDecl.name));
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
		auto var = m_scope.lookup(_variableName.name);
		if (var)
		{
			Scope::Variable const& _var = boost::get<Scope::Variable>(*var);
			if (int heightDiff = variableHeightDiff(_var, _location, true))
				m_state.assembly.append(solidity::swapInstruction(heightDiff - 1));
			m_state.assembly.append(solidity::Instruction::POP);
		}
		else
		{
			solAssert(
				m_identifierAccess.generateCode,
				"Identifier not found and no external access available."
			);
			m_identifierAccess.generateCode(_variableName, IdentifierContext::LValue, m_state.assembly);
		}
	}

	/// Determines the stack height difference to the given variables. Automatically generates
	/// errors if it is not yet in scope or the height difference is too large. Returns 0 on
	/// errors and the (positive) stack height difference otherwise.
	int variableHeightDiff(Scope::Variable const& _var, SourceLocation const& _location, bool _forSwap)
	{
		int heightDiff = m_state.assembly.deposit() - _var.stackHeight;
		if (heightDiff <= (_forSwap ? 1 : 0) || heightDiff > (_forSwap ? 17 : 16))
		{
			//@TODO move this to analysis phase.
			m_state.errors.push_back(make_shared<Error>(
				Error::Type::TypeError,
				"Variable inaccessible, too deep inside stack (" + boost::lexical_cast<string>(heightDiff) + ")",
				_location
			));
			return 0;
		}
		else
			return heightDiff;
	}

	void expectDeposit(int _deposit, int _oldHeight)
	{
		solAssert(m_state.assembly.deposit() == _oldHeight + _deposit, "Invalid stack deposit.");
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
	ExternalIdentifierAccess m_identifierAccess;
};

eth::Assembly assembly::CodeGenerator::assemble(
	Block const& _parsedData,
	AsmAnalyzer::Scopes& _scopes,
	ExternalIdentifierAccess const& _identifierAccess
)
{
	eth::Assembly assembly;
	GeneratorState state(m_errors, _scopes, assembly);
	CodeTransform(state, _parsedData, _identifierAccess);
	return assembly;
}

void assembly::CodeGenerator::assemble(
	Block const& _parsedData,
	AsmAnalyzer::Scopes& _scopes,
	eth::Assembly& _assembly,
	ExternalIdentifierAccess const& _identifierAccess
)
{
	GeneratorState state(m_errors, _scopes, _assembly);
	CodeTransform(state, _parsedData, _identifierAccess);
}
