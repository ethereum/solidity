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
// SPDX-License-Identifier: GPL-3.0

#include <libsolidity/analysis/ViewPureChecker.h>
#include <libsolidity/ast/ExperimentalFeatures.h>
#include <libyul/AST.h>
#include <libyul/backends/evm/EVMDialect.h>
#include <liblangutil/ErrorReporter.h>
#include <libevmasm/SemanticInformation.h>

#include <functional>
#include <utility>
#include <variant>

using namespace solidity;
using namespace solidity::langutil;
using namespace solidity::frontend;

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
		m_reportMutability(std::move(_reportMutability)) {}

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
					checkInstruction(nativeLocationOf(_funCall), *fun->instruction);

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
	void checkInstruction(SourceLocation _location, evmasm::Instruction _instruction)
	{
		if (evmasm::SemanticInformation::invalidInViewFunctions(_instruction))
			m_reportMutability(StateMutability::NonPayable, _location);
		else if (evmasm::SemanticInformation::invalidInPureFunctions(_instruction))
			m_reportMutability(StateMutability::View, _location);
	}

	yul::Dialect const& m_dialect;
	std::function<void(StateMutability, SourceLocation const&)> m_reportMutability;
};

}

bool ViewPureChecker::check()
{
	for (auto const& source: m_ast)
		source->accept(*this);

	return !m_errors;
}

bool ViewPureChecker::visit(ImportDirective const&)
{
	return false;
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
		!_funDef.virtualSemantics()
	)
		m_errorReporter.warning(
			2018_error,
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

	bool writes = _identifier.annotation().willBeWrittenTo;
	if (VariableDeclaration const* varDecl = dynamic_cast<VariableDeclaration const*>(declaration))
	{
		if (varDecl->immutable())
		{
			// Immutables that are assigned literals are pure.
			if (!(varDecl->value() && varDecl->value()->annotation().type->category() == Type::Category::RationalNumber))
				mutability = StateMutability::View;
		}
		else if (varDecl->isStateVariable() && !varDecl->isConstant())
			mutability = writes ? StateMutability::NonPayable : StateMutability::View;
	}
	else if (MagicVariableDeclaration const* magicVar = dynamic_cast<MagicVariableDeclaration const*>(declaration))
	{
		switch (magicVar->type()->category())
		{
		case Type::Category::Contract:
			solAssert(_identifier.name() == "this", "");
			if (dynamic_cast<ContractType const*>(magicVar->type()))
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
		[&](StateMutability _mutability, SourceLocation const& _location) { reportMutability(_mutability, _location); }
	}(_inlineAssembly.operations().root());
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
			2527_error,
			_location,
			"Function declared as pure, but this expression (potentially) reads from the "
			"environment or state and thus requires \"view\"."
		);
		m_errors = true;
	}
	else if (_mutability == StateMutability::NonPayable)
	{
		m_errorReporter.typeError(
			8961_error,
			_location,
			"Function cannot be declared as " +
			stateMutabilityToString(m_currentFunction->stateMutability()) +
			" because this expression (potentially) modifies the state."
		);
		m_errors = true;
	}
	else if (_mutability == StateMutability::Payable)
	{
		// We do not warn for library functions because they cannot be payable anyway.
		// Also internal functions should be allowed to use `msg.value`.
		if ((m_currentFunction->isConstructor() || m_currentFunction->isPublic()) && !m_currentFunction->libraryFunction())
		{
			if (_nestedLocation)
				m_errorReporter.typeError(
					4006_error,
					_location,
					SecondarySourceLocation().append("\"msg.value\" or \"callvalue()\" appear here inside the modifier.", *_nestedLocation),
					m_currentFunction->isConstructor()  ?
						"This modifier uses \"msg.value\" or \"callvalue()\" and thus the constructor has to be payable."
						: "This modifier uses \"msg.value\" or \"callvalue()\" and thus the function has to be payable or internal."
				);
			else
				m_errorReporter.typeError(
					5887_error,
					_location,
					m_currentFunction->isConstructor()  ?
						"\"msg.value\" and \"callvalue()\" can only be used in payable constructors. Make the constructor \"payable\" to avoid this error."
						: "\"msg.value\" and \"callvalue()\" can only be used in payable public functions. Make the function \"payable\" or use an internal function to avoid this error."
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

ViewPureChecker::MutabilityAndLocation const& ViewPureChecker::modifierMutability(
	ModifierDefinition const& _modifier
)
{
	if (!m_inferredMutability.count(&_modifier))
	{
		MutabilityAndLocation bestMutabilityAndLocation{};
		FunctionDefinition const* currentFunction = nullptr;
		std::swap(bestMutabilityAndLocation, m_bestMutabilityAndLocation);
		std::swap(currentFunction, m_currentFunction);

		_modifier.accept(*this);

		std::swap(bestMutabilityAndLocation, m_bestMutabilityAndLocation);
		std::swap(currentFunction, m_currentFunction);
	}
	return m_inferredMutability.at(&_modifier);
}

void ViewPureChecker::reportFunctionCallMutability(StateMutability _mutability, langutil::SourceLocation const& _location)
{
	// We only require "nonpayable" to call a payable function.
	if (_mutability == StateMutability::Payable)
		_mutability = StateMutability::NonPayable;
	reportMutability(_mutability, _location);
}

void ViewPureChecker::endVisit(BinaryOperation const& _binaryOperation)
{
	if (*_binaryOperation.annotation().userDefinedFunction != nullptr)
		reportFunctionCallMutability((*_binaryOperation.annotation().userDefinedFunction)->stateMutability(), _binaryOperation.location());
}

void ViewPureChecker::endVisit(UnaryOperation const& _unaryOperation)
{
	if (*_unaryOperation.annotation().userDefinedFunction != nullptr)
		reportFunctionCallMutability((*_unaryOperation.annotation().userDefinedFunction)->stateMutability(), _unaryOperation.location());
}

void ViewPureChecker::endVisit(FunctionCall const& _functionCall)
{
	if (*_functionCall.annotation().kind != FunctionCallKind::FunctionCall)
		return;

	reportFunctionCallMutability(
		dynamic_cast<FunctionType const&>(*_functionCall.expression().annotation().type).stateMutability(),
		_functionCall.location()
	);
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
	bool writes = _memberAccess.annotation().willBeWrittenTo;

	ASTString const& member = _memberAccess.memberName();
	switch (_memberAccess.expression().annotation().type->category())
	{
	case Type::Category::Address:
		if (member == "balance" || member == "code" || member == "codehash")
			mutability = StateMutability::View;
		break;
	case Type::Category::Magic:
	{
		using MagicMember = std::pair<MagicType::Kind, std::string>;
		std::set<MagicMember> static const pureMembers{
			{MagicType::Kind::ABI, "decode"},
			{MagicType::Kind::ABI, "encode"},
			{MagicType::Kind::ABI, "encodePacked"},
			{MagicType::Kind::ABI, "encodeWithSelector"},
			{MagicType::Kind::ABI, "encodeCall"},
			{MagicType::Kind::ABI, "encodeWithSignature"},
			{MagicType::Kind::Message, "data"},
			{MagicType::Kind::Message, "sig"},
			{MagicType::Kind::MetaType, "creationCode"},
			{MagicType::Kind::MetaType, "runtimeCode"},
			{MagicType::Kind::MetaType, "name"},
			{MagicType::Kind::MetaType, "interfaceId"},
			{MagicType::Kind::MetaType, "typehash"},
			{MagicType::Kind::MetaType, "min"},
			{MagicType::Kind::MetaType, "max"},
		};
		std::set<MagicMember> static const payableMembers{
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
			mutability = StateMutability::View;
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
		bool writes = _indexAccess.annotation().willBeWrittenTo;
		if (_indexAccess.baseExpression().annotation().type->dataStoredIn(DataLocation::Storage))
			reportMutability(writes ? StateMutability::NonPayable : StateMutability::View, _indexAccess.location());
	}
}

void ViewPureChecker::endVisit(IndexRangeAccess const& _indexRangeAccess)
{
	bool writes = _indexRangeAccess.annotation().willBeWrittenTo;
	if (_indexRangeAccess.baseExpression().annotation().type->dataStoredIn(DataLocation::Storage))
		reportMutability(writes ? StateMutability::NonPayable : StateMutability::View, _indexRangeAccess.location());
}

void ViewPureChecker::endVisit(ModifierInvocation const& _modifier)
{
	if (ModifierDefinition const* mod = dynamic_cast<decltype(mod)>(_modifier.name().annotation().referencedDeclaration))
	{
		MutabilityAndLocation const& mutAndLocation = modifierMutability(*mod);
		reportMutability(mutAndLocation.mutability, _modifier.location(), mutAndLocation.location);
	}
	else
		solAssert(dynamic_cast<ContractDefinition const*>(_modifier.name().annotation().referencedDeclaration), "");
}
