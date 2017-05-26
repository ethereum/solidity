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
#include <libsolidity/inlineasm/AsmAnalysisInfo.h>

#include <libevmasm/Assembly.h>
#include <libevmasm/SourceLocation.h>
#include <libevmasm/Instruction.h>

#include <libjulia/backends/AbstractAssembly.h>

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
	GeneratorState(ErrorList& _errors, AsmAnalysisInfo& _analysisInfo):
		errors(_errors), info(_analysisInfo) {}

	ErrorList& errors;
	AsmAnalysisInfo info;
};

class EthAssemblyAdapter: public julia::AbstractAssembly
{
public:
	EthAssemblyAdapter(eth::Assembly& _assembly):
		m_assembly(_assembly)
	{
	}
	virtual void setSourceLocation(SourceLocation const& _location) override
	{
		m_assembly.setSourceLocation(_location);
	}
	virtual int stackHeight() const override { return m_assembly.deposit(); }
	virtual void appendInstruction(solidity::Instruction _instruction) override
	{
		m_assembly.append(_instruction);
	}
	virtual void appendConstant(u256 const& _constant) override
	{
		m_assembly.append(_constant);
	}
	/// Append a label.
	virtual void appendLabel(size_t _labelId) override
	{
		m_assembly.append(eth::AssemblyItem(eth::Tag, _labelId));
	}
	/// Append a label reference.
	virtual void appendLabelReference(size_t _labelId) override
	{
		m_assembly.append(eth::AssemblyItem(eth::PushTag, _labelId));
	}
	virtual size_t newLabelId() override
	{
		return assemblyTagToIdentifier(m_assembly.newTag());
	}
	virtual void appendLinkerSymbol(std::string const& _linkerSymbol) override
	{
		m_assembly.appendLibraryAddress(_linkerSymbol);
	}

private:
	size_t assemblyTagToIdentifier(eth::AssemblyItem const& _tag) const
	{
		u256 id = _tag.data();
		solAssert(id <= std::numeric_limits<size_t>::max(), "Tag id too large.");
		return size_t(id);
	}

	eth::Assembly& m_assembly;
};

class CodeTransform: public boost::static_visitor<>
{
public:
	/// Create the code transformer which appends assembly to _state.assembly when called
	/// with parsed assembly data.
	/// @param _identifierAccess used to resolve identifiers external to the inline assembly
	explicit CodeTransform(
		GeneratorState& _state,
		julia::AbstractAssembly& _assembly,
		assembly::Block const& _block,
		assembly::ExternalIdentifierAccess const& _identifierAccess = assembly::ExternalIdentifierAccess()
	): CodeTransform(_state, _assembly, _block, _identifierAccess, _assembly.stackHeight())
	{
	}

private:
	CodeTransform(
		GeneratorState& _state,
		julia::AbstractAssembly& _assembly,
		assembly::Block const& _block,
		assembly::ExternalIdentifierAccess const& _identifierAccess,
		int _initialStackHeight
	):
		m_state(_state),
		m_assembly(_assembly),
		m_scope(*m_state.info.scopes.at(&_block)),
		m_identifierAccess(_identifierAccess),
		m_initialStackHeight(_initialStackHeight)
	{
		int blockStartStackHeight = m_assembly.stackHeight();
		std::for_each(_block.statements.begin(), _block.statements.end(), boost::apply_visitor(*this));

		m_assembly.setSourceLocation(_block.location);

		// pop variables
		for (auto const& identifier: m_scope.identifiers)
			if (identifier.second.type() == typeid(Scope::Variable))
				m_assembly.appendInstruction(solidity::Instruction::POP);

		int deposit = m_assembly.stackHeight() - blockStartStackHeight;
		solAssert(deposit == 0, "Invalid stack height at end of block.");
	}

public:
	void operator()(assembly::Instruction const& _instruction)
	{
		m_assembly.setSourceLocation(_instruction.location);
		m_assembly.appendInstruction(_instruction.instruction);
		checkStackHeight(&_instruction);
	}
	void operator()(assembly::Literal const& _literal)
	{
		m_assembly.setSourceLocation(_literal.location);
		if (_literal.kind == assembly::LiteralKind::Number)
			m_assembly.appendConstant(u256(_literal.value));
		else if (_literal.kind == assembly::LiteralKind::Boolean)
		{
			if (_literal.value == "true")
				m_assembly.appendConstant(u256(1));
			else
				m_assembly.appendConstant(u256(0));
		}
		else
		{
			solAssert(_literal.value.size() <= 32, "");
			m_assembly.appendConstant(u256(h256(_literal.value, h256::FromBinary, h256::AlignLeft)));
		}
		checkStackHeight(&_literal);
	}
	void operator()(assembly::Identifier const& _identifier)
	{
		m_assembly.setSourceLocation(_identifier.location);
		// First search internals, then externals.
		if (m_scope.lookup(_identifier.name, Scope::NonconstVisitor(
			[=](Scope::Variable& _var)
			{
				if (int heightDiff = variableHeightDiff(_var, _identifier.location, false))
					m_assembly.appendInstruction(solidity::dupInstruction(heightDiff));
				else
					// Store something to balance the stack
					m_assembly.appendConstant(u256(0));
			},
			[=](Scope::Label& _label)
			{
				assignLabelIdIfUnset(_label);
				m_assembly.appendLabelReference(*_label.id);
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
		m_identifierAccess.generateCode(_identifier, IdentifierContext::RValue, m_assembly);
		checkStackHeight(&_identifier);
	}
	void operator()(FunctionalInstruction const& _instr)
	{
		for (auto it = _instr.arguments.rbegin(); it != _instr.arguments.rend(); ++it)
		{
			int height = m_assembly.stackHeight();
			boost::apply_visitor(*this, *it);
			expectDeposit(1, height);
		}
		(*this)(_instr.instruction);
		checkStackHeight(&_instr);
	}
	void operator()(assembly::FunctionCall const&)
	{
		solAssert(false, "Function call not removed during desugaring phase.");
	}
	void operator()(Label const& _label)
	{
		m_assembly.setSourceLocation(_label.location);
		solAssert(m_scope.identifiers.count(_label.name), "");
		Scope::Label& label = boost::get<Scope::Label>(m_scope.identifiers.at(_label.name));
		assignLabelIdIfUnset(label);
		m_assembly.appendLabel(*label.id);
		checkStackHeight(&_label);
	}
	void operator()(assembly::StackAssignment const& _assignment)
	{
		m_assembly.setSourceLocation(_assignment.location);
		generateAssignment(_assignment.variableName, _assignment.location);
		checkStackHeight(&_assignment);
	}
	void operator()(assembly::Assignment const& _assignment)
	{
		int height = m_assembly.stackHeight();
		boost::apply_visitor(*this, *_assignment.value);
		expectDeposit(1, height);
		m_assembly.setSourceLocation(_assignment.location);
		generateAssignment(_assignment.variableName, _assignment.location);
		checkStackHeight(&_assignment);
	}
	void operator()(assembly::VariableDeclaration const& _varDecl)
	{
		int height = m_assembly.stackHeight();
		int expectedItems = _varDecl.variables.size();
		boost::apply_visitor(*this, *_varDecl.value);
		expectDeposit(expectedItems, height);
		for (auto const& variable: _varDecl.variables)
		{
			auto& var = boost::get<Scope::Variable>(m_scope.identifiers.at(variable.name));
			var.stackHeight = height++;
			var.active = true;
		}
	}
	void operator()(assembly::Block const& _block)
	{
		CodeTransform(m_state, m_assembly, _block, m_identifierAccess, m_initialStackHeight);
		checkStackHeight(&_block);
	}
	void operator()(assembly::Switch const&)
	{
		solAssert(false, "Switch not removed during desugaring phase.");
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
				m_assembly.appendInstruction(solidity::swapInstruction(heightDiff - 1));
			m_assembly.appendInstruction(solidity::Instruction::POP);
		}
		else
		{
			solAssert(
				m_identifierAccess.generateCode,
				"Identifier not found and no external access available."
			);
			m_identifierAccess.generateCode(_variableName, IdentifierContext::LValue, m_assembly);
		}
	}

	/// Determines the stack height difference to the given variables. Automatically generates
	/// errors if it is not yet in scope or the height difference is too large. Returns 0 on
	/// errors and the (positive) stack height difference otherwise.
	int variableHeightDiff(Scope::Variable const& _var, SourceLocation const& _location, bool _forSwap)
	{
		int heightDiff = m_assembly.stackHeight() - _var.stackHeight;
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
		solAssert(m_assembly.stackHeight() == _oldHeight + _deposit, "Invalid stack deposit.");
	}

	void checkStackHeight(void const* _astElement)
	{
		solAssert(m_state.info.stackHeightInfo.count(_astElement), "Stack height for AST element not found.");
		solAssert(
			m_state.info.stackHeightInfo.at(_astElement) == m_assembly.stackHeight() - m_initialStackHeight,
			"Stack height mismatch between analysis and code generation phase."
		);
	}

	/// Assigns the label's id to a value taken from eth::Assembly if it has not yet been set.
	void assignLabelIdIfUnset(Scope::Label& _label)
	{
		if (!_label.id)
			_label.id.reset(m_assembly.newLabelId());
	}


	GeneratorState& m_state;
	julia::AbstractAssembly& m_assembly;
	Scope& m_scope;
	ExternalIdentifierAccess m_identifierAccess;
	int const m_initialStackHeight;
};

eth::Assembly assembly::CodeGenerator::assemble(
	Block const& _parsedData,
	AsmAnalysisInfo& _analysisInfo,
	ExternalIdentifierAccess const& _identifierAccess
)
{
	eth::Assembly assembly;
	GeneratorState state(m_errors, _analysisInfo);
	EthAssemblyAdapter assemblyAdapter(assembly);
	CodeTransform(state, assemblyAdapter, _parsedData, _identifierAccess);
	return assembly;
}

void assembly::CodeGenerator::assemble(
	Block const& _parsedData,
	AsmAnalysisInfo& _analysisInfo,
	eth::Assembly& _assembly,
	ExternalIdentifierAccess const& _identifierAccess
)
{
	GeneratorState state(m_errors, _analysisInfo);
	EthAssemblyAdapter assemblyAdapter(_assembly);
	CodeTransform(state, assemblyAdapter, _parsedData, _identifierAccess);
}
