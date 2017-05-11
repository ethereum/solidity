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
 * Common code generator for translating Julia / inline assembly to EVM and EVM1.5.
 */

#include <libjulia/backends/evm/EVMCodeTransform.h>

#include <libsolidity/inlineasm/AsmAnalysisInfo.h>
#include <libsolidity/inlineasm/AsmData.h>

#include <libsolidity/interface/Utils.h>

using namespace std;
using namespace dev;
using namespace dev::julia;
using namespace dev::solidity;
using namespace dev::solidity::assembly;

CodeTransform::CodeTransform(
	ErrorReporter& _errorReporter,
	AbstractAssembly& _assembly,
	Block const& _block,
	AsmAnalysisInfo& _analysisInfo,
	ExternalIdentifierAccess const& _identifierAccess,
	int _initialStackHeight
):
	m_errorReporter(_errorReporter),
	m_assembly(_assembly),
	m_info(_analysisInfo),
	m_scope(*_analysisInfo.scopes.at(&_block)),
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

void CodeTransform::operator()(const FunctionDefinition&)
{
	solAssert(false, "Function definition not removed during desugaring phase.");
}

void CodeTransform::generateAssignment(Identifier const& _variableName, SourceLocation const& _location)
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

int CodeTransform::variableHeightDiff(solidity::assembly::Scope::Variable const& _var, SourceLocation const& _location, bool _forSwap)
{
	int heightDiff = m_assembly.stackHeight() - _var.stackHeight;
	if (heightDiff <= (_forSwap ? 1 : 0) || heightDiff > (_forSwap ? 17 : 16))
	{
		//@TODO move this to analysis phase.
		m_errorReporter.typeError(
			_location,
			"Variable inaccessible, too deep inside stack (" + boost::lexical_cast<string>(heightDiff) + ")"
		);
		return 0;
	}
	else
		return heightDiff;
}

void CodeTransform::expectDeposit(int _deposit, int _oldHeight)
{
	solAssert(m_assembly.stackHeight() == _oldHeight + _deposit, "Invalid stack deposit.");
}

void CodeTransform::checkStackHeight(void const* _astElement)
{
	solAssert(m_info.stackHeightInfo.count(_astElement), "Stack height for AST element not found.");
	solAssert(
		m_info.stackHeightInfo.at(_astElement) == m_assembly.stackHeight() - m_initialStackHeight,
		"Stack height mismatch between analysis and code generation phase."
	);
}

void CodeTransform::assignLabelIdIfUnset(Scope::Label& _label)
{
	if (!_label.id)
		_label.id.reset(m_assembly.newLabelId());
}

void CodeTransform::operator()(Block const& _block)
{
	CodeTransform(m_errorReporter, m_assembly, _block, m_info, m_identifierAccess, m_initialStackHeight);
	checkStackHeight(&_block);
}

void CodeTransform::operator()(Switch const&)
{
	solAssert(false, "Switch not removed during desugaring phase.");
}

void CodeTransform::operator()(VariableDeclaration const& _varDecl)
{
	int expectedItems = _varDecl.variables.size();
	int height = m_assembly.stackHeight();
	boost::apply_visitor(*this, *_varDecl.value);
	expectDeposit(expectedItems, height);
	for (auto const& variable: _varDecl.variables)
	{
		auto& var = boost::get<Scope::Variable>(m_scope.identifiers.at(variable.name));
		var.stackHeight = height++;
		var.active = true;
	}
}

void CodeTransform::operator()(Assignment const& _assignment)
{
	int height = m_assembly.stackHeight();
	boost::apply_visitor(*this, *_assignment.value);
	expectDeposit(1, height);
	m_assembly.setSourceLocation(_assignment.location);
	generateAssignment(_assignment.variableName, _assignment.location);
	checkStackHeight(&_assignment);
}

void CodeTransform::operator()(StackAssignment const& _assignment)
{
	m_assembly.setSourceLocation(_assignment.location);
	generateAssignment(_assignment.variableName, _assignment.location);
	checkStackHeight(&_assignment);
}

void CodeTransform::operator()(Label const& _label)
{
	m_assembly.setSourceLocation(_label.location);
	solAssert(m_scope.identifiers.count(_label.name), "");
	Scope::Label& label = boost::get<Scope::Label>(m_scope.identifiers.at(_label.name));
	assignLabelIdIfUnset(label);
	m_assembly.appendLabel(*label.id);
	checkStackHeight(&_label);
}

void CodeTransform::operator()(FunctionCall const&)
{
	solAssert(false, "Function call not removed during desugaring phase.");
}

void CodeTransform::operator()(FunctionalInstruction const& _instr)
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

void CodeTransform::operator()(assembly::Identifier const& _identifier)
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

void CodeTransform::operator()(assembly::Literal const& _literal)
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

void CodeTransform::operator()(assembly::Instruction const& _instruction)
{
	m_assembly.setSourceLocation(_instruction.location);
	m_assembly.appendInstruction(_instruction.instruction);
	checkStackHeight(&_instruction);
}
