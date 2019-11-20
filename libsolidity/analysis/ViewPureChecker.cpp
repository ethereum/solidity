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

#include <libsolidity/analysis/ViewPureChecker.h>
#include <libsolidity/ast/ExperimentalFeatures.h>
#include <libyul/AsmData.h>
#include <libyul/backends/evm/EVMDialect.h>
#include <liblangutil/ErrorReporter.h>
#include <libevmasm/SemanticInformation.h>

#include <functional>
#include <variant>

using namespace std;
using namespace dev;
using namespace langutil;
using namespace dev::solidity;

namespace
{

class AssemblyViewPureChecker
{
public:
	explicit AssemblyViewPureChecker(
		yul::Dialect const& _dialect,
		std::function<void(StateMutability, SourceLocation const&)> _reportMutability
	):
		m_dialect(_dialect),
		m_reportMutability(_reportMutability) {}

	void operator()(yul::Instruction const& _instruction)
	{
		checkInstruction(_instruction.location, _instruction.instruction);
	}
	void operator()(yul::Literal const&) {}
	void operator()(yul::Identifier const&) {}
	void operator()(yul::ExpressionStatement const& _expr)
	{
		std::visit(*this, _expr.expression);
	}
	void operator()(yul::Assignment const& _assignment)
	{
		std::visit(*this, *_assignment.value);
	}
	void operator()(yul::VariableDeclaration const& _varDecl)
	{
		if (_varDecl.value)
			std::visit(*this, *_varDecl.value);
	}
	void operator()(yul::FunctionDefinition const& _funDef)
	{
		(*this)(_funDef.body);
	}
	void operator()(yul::FunctionCall const& _funCall)
	{
		if (yul::EVMDialect const* dialect = dynamic_cast<decltype(dialect)>(&m_dialect))
			if (yul::BuiltinFunctionForEVM const* fun = dialect->builtin(_funCall.functionName.name))
				if (fun->instruction)
					checkInstruction(_funCall.location, *fun->instruction);

		for (auto const& arg: _funCall.arguments)
			std::visit(*this, arg);
	}
	void operator()(yul::If const& _if)
	{
		std::visit(*this, *_if.condition);
		(*this)(_if.body);
	}
	void operator()(yul::Switch const& _switch)
	{
		std::visit(*this, *_switch.expression);
		for (auto const& _case: _switch.cases)
		{
			if (_case.value)
				(*this)(*_case.value);
			(*this)(_case.body);
		}
	}
	void operator()(yul::ForLoop const& _for)
	{
		(*this)(_for.pre);
		std::visit(*this, *_for.condition);
		(*this)(_for.body);
		(*this)(_for.post);
	}
	void operator()(yul::Break const&)
	{
	}
	void operator()(yul::Continue const&)
	{
	}
	void operator()(yul::Leave const&)
	{
	}
	void operator()(yul::Block const& _block)
	{
		for (auto const& s: _block.statements)
			std::visit(*this, s);
	}

private:
	void checkInstruction(SourceLocation _location, dev::eth::Instruction _instruction)
	{
		if (eth::SemanticInformation::invalidInViewFunctions(_instruction))
			m_reportMutability(StateMutability::NonPayable, _location);
		else if (eth::SemanticInformation::invalidInPureFunctions(_instruction))
			m_reportMutability(StateMutability::View, _location);
	}

	yul::Dialect const& m_dialect;
	std::function<void(StateMutability, SourceLocation const&)> m_reportMutability;
};

}

bool ViewPureChecker::check()
{
	vector<ContractDefinition const*> contracts;

	for (auto const& node: m_ast)
	{
		SourceUnit const* source = dynamic_cast<SourceUnit const*>(node.get());
		solAssert(source, "");
		contracts += source->filteredNodes<ContractDefinition>(source->nodes());
	}

	// Check modifiers first to infer their state mutability.
	for (auto const& contract: contracts)
		for (ModifierDefinition const* mod: contract->functionModifiers())
			mod->accept(*this);

	for (auto const& contract: contracts)
		contract->accept(*this);

	return !m_errors;
}



bool ViewPureChecker::visit(FunctionDefinition const& _funDef)
{
	solAssert(!m_currentFunction, "");
	m_currentFunction = &_funDef;
	m_bestMutabilityAndLocation = {StateMutability::Pure, _funDef.location()};
	return true;
}

void ViewPureChecker::endVisit(FunctionDefinition const& _funDef)
{
	solAssert(m_currentFunction == &_funDef, "");
	if (
		m_bestMutabilityAndLocation.mutability < _funDef.stateMutability() &&
		_funDef.stateMutability() != StateMutability::Payable &&
		_funDef.isImplemented() &&
		!_funDef.body().statements().empty() &&
		!_funDef.isConstructor() &&
		!_funDef.isFallback() &&
		!_funDef.isReceive() &&
		!_funDef.annotation().superFunction
	)
		m_errorReporter.warning(
			_funDef.location(),
			"Function state mutability can be restricted to " + stateMutabilityToString(m_bestMutabilityAndLocation.mutability)
		);
	m_currentFunction = nullptr;
}

bool ViewPureChecker::visit(ModifierDefinition const& _modifier)
{
	solAssert(m_currentFunction == nullptr, "");
	m_bestMutabilityAndLocation = {StateMutability::Pure, _modifier.location()};
	return true;
}

void ViewPureChecker::endVisit(ModifierDefinition const& _modifierDef)
{
	solAssert(m_currentFunction == nullptr, "");
	m_inferredMutability[&_modifierDef] = std::move(m_bestMutabilityAndLocation);
}

void ViewPureChecker::endVisit(Identifier const& _identifier)
{
	Declaration const* declaration = _identifier.annotation().referencedDeclaration;
	solAssert(declaration, "");

	StateMutability mutability = StateMutability::Pure;

	bool writes = _identifier.annotation().lValueRequested;
	if (VariableDeclaration const* varDecl = dynamic_cast<VariableDeclaration const*>(declaration))
	{
		if (varDecl->isStateVariable() && !varDecl->isConstant())
			mutability = writes ? StateMutability::NonPayable : StateMutability::View;
	}
	else if (MagicVariableDeclaration const* magicVar = dynamic_cast<MagicVariableDeclaration const*>(declaration))
	{
		switch (magicVar->type()->category())
		{
		case Type::Category::Contract:
			solAssert(_identifier.name() == "this" || _identifier.name() == "super", "");
			if (!dynamic_cast<ContractType const&>(*magicVar->type()).isSuper())
				// reads the address
				mutability = StateMutability::View;
			break;
		case Type::Category::Integer:
			solAssert(_identifier.name() == "now", "");
			mutability = StateMutability::View;
			break;
		default:
			break;
		}
	}

	reportMutability(mutability, _identifier.location());
}

void ViewPureChecker::endVisit(InlineAssembly const& _inlineAssembly)
{
	AssemblyViewPureChecker{
		_inlineAssembly.dialect(),
		[=](StateMutability _mutability, SourceLocation const& _location) { reportMutability(_mutability, _location); }
	}(_inlineAssembly.operations());
}

void ViewPureChecker::reportMutability(
	StateMutability _mutability,
	SourceLocation const& _location,
	std::optional<SourceLocation> const& _nestedLocation
)
{
	if (_mutability > m_bestMutabilityAndLocation.mutability)
		m_bestMutabilityAndLocation = MutabilityAndLocation{_mutability, _location};
	if (!m_currentFunction || _mutability <= m_currentFunction->stateMutability())
		return;

	// Check for payable here, because any occurrence of `msg.value`
	// will set mutability to payable.
	if (_mutability == StateMutability::View || (
		_mutability == StateMutability::Payable &&
		m_currentFunction->stateMutability() == StateMutability::Pure
	))
	{
		m_errorReporter.typeError(
			_location,
			"Function declared as pure, but this expression (potentially) reads from the "
			"environment or state and thus requires \"view\"."
		);
		m_errors = true;
	}
	else if (_mutability == StateMutability::NonPayable)
	{
		m_errorReporter.typeError(
			_location,
			"Function declared as " +
			stateMutabilityToString(m_currentFunction->stateMutability()) +
			", but this expression (potentially) modifies the state and thus "
			"requires non-payable (the default) or payable."
		);
		m_errors = true;
	}
	else if (_mutability == StateMutability::Payable)
	{
		// We do not warn for library functions because they cannot be payable anyway.
		// Also internal functions should be allowed to use `msg.value`.
		if (m_currentFunction->isPublic() && m_currentFunction->inContractKind() != ContractDefinition::ContractKind::Library)
		{
			if (_nestedLocation)
				m_errorReporter.typeError(
					_location,
					SecondarySourceLocation().append("\"msg.value\" or \"callvalue()\" appear here inside the modifier.", *_nestedLocation),
					"This modifier uses \"msg.value\" or \"callvalue()\" and thus the function has to be payable or internal."
				);
			else
				m_errorReporter.typeError(
					_location,
					"\"msg.value\" and \"callvalue()\" can only be used in payable public functions. Make the function "
					"\"payable\" or use an internal function to avoid this error."
				);
			m_errors = true;
		}
	}
	else
		solAssert(false, "");

	solAssert(
		m_currentFunction->stateMutability() == StateMutability::View ||
		m_currentFunction->stateMutability() == StateMutability::Pure ||
		m_currentFunction->stateMutability() == StateMutability::NonPayable,
		""
	);
}

void ViewPureChecker::endVisit(FunctionCall const& _functionCall)
{
	if (_functionCall.annotation().kind != FunctionCallKind::FunctionCall)
		return;

	StateMutability mutability = dynamic_cast<FunctionType const&>(*_functionCall.expression().annotation().type).stateMutability();
	// We only require "nonpayable" to call a payble function.
	if (mutability == StateMutability::Payable)
		mutability = StateMutability::NonPayable;
	reportMutability(mutability, _functionCall.location());
}

bool ViewPureChecker::visit(MemberAccess const& _memberAccess)
{
	// Catch the special case of `this.f.selector` which is a pure expression.
	ASTString const& member = _memberAccess.memberName();
	if (
		_memberAccess.expression().annotation().type->category() == Type::Category::Function &&
		member == "selector"
	)
		if (auto const* expr = dynamic_cast<MemberAccess const*>(&_memberAccess.expression()))
			if (auto const* exprInt = dynamic_cast<Identifier const*>(&expr->expression()))
				if (exprInt->name() == "this")
					// Do not continue visiting.
					return false;
	return true;
}

void ViewPureChecker::endVisit(MemberAccess const& _memberAccess)
{
	StateMutability mutability = StateMutability::Pure;
	bool writes = _memberAccess.annotation().lValueRequested;

	ASTString const& member = _memberAccess.memberName();
	switch (_memberAccess.expression().annotation().type->category())
	{
	case Type::Category::Address:
		if (member == "balance")
			mutability = StateMutability::View;
		break;
	case Type::Category::Magic:
	{
		using MagicMember = pair<MagicType::Kind, string>;
		set<MagicMember> static const pureMembers{
			{MagicType::Kind::ABI, "decode"},
			{MagicType::Kind::ABI, "encode"},
			{MagicType::Kind::ABI, "encodePacked"},
			{MagicType::Kind::ABI, "encodeWithSelector"},
			{MagicType::Kind::ABI, "encodeWithSignature"},
			{MagicType::Kind::Block, "blockhash"},
			{MagicType::Kind::Message, "data"},
			{MagicType::Kind::Message, "sig"},
			{MagicType::Kind::MetaType, "creationCode"},
			{MagicType::Kind::MetaType, "runtimeCode"},
			{MagicType::Kind::MetaType, "name"},
		};
		set<MagicMember> static const payableMembers{
			{MagicType::Kind::Message, "value"}
		};

		auto const& type = dynamic_cast<MagicType const&>(*_memberAccess.expression().annotation().type);
		MagicMember magicMember(type.kind(), member);

		if (!pureMembers.count(magicMember))
			mutability = StateMutability::View;
		if (payableMembers.count(magicMember))
			mutability = StateMutability::Payable;
		break;
	}
	case Type::Category::Struct:
	{
		if (_memberAccess.expression().annotation().type->dataStoredIn(DataLocation::Storage))
			mutability = writes ? StateMutability::NonPayable : StateMutability::View;
		break;
	}
	case Type::Category::Array:
	{
		auto const& type = dynamic_cast<ArrayType const&>(*_memberAccess.expression().annotation().type);
		if (member == "length" && type.isDynamicallySized() && type.dataStoredIn(DataLocation::Storage))
			mutability = writes ? StateMutability::NonPayable : StateMutability::View;
		break;
	}
	default:
	{
		if (VariableDeclaration const* varDecl = dynamic_cast<VariableDeclaration const*>(
			_memberAccess.annotation().referencedDeclaration
		))
			if (varDecl->isStateVariable() && !varDecl->isConstant())
				mutability = writes ? StateMutability::NonPayable : StateMutability::View;
		break;
	}
	}
	reportMutability(mutability, _memberAccess.location());
}

void ViewPureChecker::endVisit(IndexAccess const& _indexAccess)
{
	if (!_indexAccess.indexExpression())
		solAssert(_indexAccess.annotation().type->category() == Type::Category::TypeType, "");
	else
	{
		bool writes = _indexAccess.annotation().lValueRequested;
		if (_indexAccess.baseExpression().annotation().type->dataStoredIn(DataLocation::Storage))
			reportMutability(writes ? StateMutability::NonPayable : StateMutability::View, _indexAccess.location());
	}
}

void ViewPureChecker::endVisit(IndexRangeAccess const& _indexRangeAccess)
{
	bool writes = _indexRangeAccess.annotation().lValueRequested;
	if (_indexRangeAccess.baseExpression().annotation().type->dataStoredIn(DataLocation::Storage))
		reportMutability(writes ? StateMutability::NonPayable : StateMutability::View, _indexRangeAccess.location());
}

void ViewPureChecker::endVisit(ModifierInvocation const& _modifier)
{
	solAssert(_modifier.name(), "");
	if (ModifierDefinition const* mod = dynamic_cast<decltype(mod)>(_modifier.name()->annotation().referencedDeclaration))
	{
		solAssert(m_inferredMutability.count(mod), "");
		auto const& mutAndLocation = m_inferredMutability.at(mod);
		reportMutability(mutAndLocation.mutability, _modifier.location(), mutAndLocation.location);
	}
	else
		solAssert(dynamic_cast<ContractDefinition const*>(_modifier.name()->annotation().referencedDeclaration), "");
}

