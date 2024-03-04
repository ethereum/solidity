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
/**
 * @author Federico Bond <federicobond@gmail.com>
 * @date 2016
 * Static analyzer and checker.
 */

#include <libsolidity/analysis/StaticAnalyzer.h>

#include <libsolidity/analysis/ConstantEvaluator.h>
#include <libsolidity/ast/AST.h>
#include <liblangutil/ErrorReporter.h>
#include <memory>

using namespace solidity;
using namespace solidity::langutil;
using namespace solidity::frontend;

/**
 * Helper class that determines whether a contract's constructor uses inline assembly.
 */
class solidity::frontend::ConstructorUsesAssembly
{
public:
	/// @returns true if and only if the contract's or any of its bases' constructors
	/// use inline assembly.
	bool check(ContractDefinition const& _contract)
	{
		for (auto const* base: _contract.annotation().linearizedBaseContracts)
			if (checkInternal(*base))
				return true;
		return false;
	}


private:
	class Checker: public ASTConstVisitor
	{
	public:
		Checker(FunctionDefinition const& _f) { _f.accept(*this); }
		bool visit(InlineAssembly const&) override { assemblySeen = true; return false; }
		bool assemblySeen = false;
	};

	bool checkInternal(ContractDefinition const& _contract)
	{
		if (!m_usesAssembly.count(&_contract))
		{
			bool usesAssembly = false;
			if (_contract.constructor())
				usesAssembly = Checker{*_contract.constructor()}.assemblySeen;
			m_usesAssembly[&_contract] = usesAssembly;
		}
		return m_usesAssembly[&_contract];
	}

	std::map<ContractDefinition const*, bool> m_usesAssembly;
};

StaticAnalyzer::StaticAnalyzer(ErrorReporter& _errorReporter):
	m_errorReporter(_errorReporter)
{
}

StaticAnalyzer::~StaticAnalyzer()
{
}

bool StaticAnalyzer::analyze(SourceUnit const& _sourceUnit)
{
	_sourceUnit.accept(*this);
	return !Error::containsErrors(m_errorReporter.errors());
}

bool StaticAnalyzer::visit(ContractDefinition const& _contract)
{
	m_library = _contract.isLibrary();
	m_currentContract = &_contract;
	return true;
}

void StaticAnalyzer::endVisit(ContractDefinition const&)
{
	m_library = false;
	m_currentContract = nullptr;
}

bool StaticAnalyzer::visit(FunctionDefinition const& _function)
{
	if (_function.isImplemented())
		m_currentFunction = &_function;
	else
		solAssert(!m_currentFunction, "");
	solAssert(m_localVarUseCount.empty(), "");
	m_constructor = _function.isConstructor();
	return true;
}

void StaticAnalyzer::endVisit(FunctionDefinition const&)
{
	if (m_currentFunction && !m_currentFunction->body().statements().empty())
		for (auto const& var: m_localVarUseCount)
			if (var.second == 0)
			{
				if (var.first.second->isCallableOrCatchParameter())
					m_errorReporter.warning(
						5667_error,
						var.first.second->location(),
						"Unused " +
						std::string(var.first.second->isTryCatchParameter() ? "try/catch" : "function") +
						" parameter. Remove or comment out the variable name to silence this warning."
					);
				else
					m_errorReporter.warning(2072_error, var.first.second->location(), "Unused local variable.");
			}
	m_localVarUseCount.clear();
	m_constructor = false;
	m_currentFunction = nullptr;
}

bool StaticAnalyzer::visit(Identifier const& _identifier)
{
	if (m_currentFunction)
		if (auto var = dynamic_cast<VariableDeclaration const*>(_identifier.annotation().referencedDeclaration))
		{
			solAssert(!var->name().empty(), "");
			if (var->isLocalVariable())
				m_localVarUseCount[std::make_pair(var->id(), var)] += 1;
		}
	return true;
}

bool StaticAnalyzer::visit(VariableDeclaration const& _variable)
{
	if (m_currentFunction)
	{
		solAssert(_variable.isLocalVariable(), "");
		if (_variable.name() != "")
			// This is not a no-op, the entry might pre-exist.
			m_localVarUseCount[std::make_pair(_variable.id(), &_variable)] += 0;
	}

	if (_variable.isStateVariable() || _variable.referenceLocation() == VariableDeclaration::Location::Storage)
		if (auto varType = dynamic_cast<CompositeType const*>(_variable.annotation().type))
			for (Type const* type: varType->fullDecomposition())
				if (type->storageSizeUpperBound() >= (bigint(1) << 64))
				{
					std::string message = "Type " + type->toString(true) +
						" covers a large part of storage and thus makes collisions likely."
						" Either use mappings or dynamic arrays and allow their size to be increased only"
						" in small quantities per transaction.";
					m_errorReporter.warning(7325_error, _variable.typeName().location(), message);
				}

	return true;
}

bool StaticAnalyzer::visit(Return const& _return)
{
	// If the return has an expression, it counts as
	// a "use" of the return parameters.
	if (m_currentFunction && _return.expression())
		for (auto const& var: m_currentFunction->returnParameters())
			if (!var->name().empty())
				m_localVarUseCount[std::make_pair(var->id(), var.get())] += 1;
	return true;
}

bool StaticAnalyzer::visit(ExpressionStatement const& _statement)
{
	if (*_statement.expression().annotation().isPure)
		m_errorReporter.warning(
			6133_error,
			_statement.location(),
			"Statement has no effect."
		);

	return true;
}

bool StaticAnalyzer::visit(MemberAccess const& _memberAccess)
{
	if (MagicType const* type = dynamic_cast<MagicType const*>(_memberAccess.expression().annotation().type))
	{
		if (type->kind() == MagicType::Kind::Message && _memberAccess.memberName() == "gas")
			m_errorReporter.typeError(
				1400_error,
				_memberAccess.location(),
				"\"msg.gas\" has been deprecated in favor of \"gasleft()\""
			);
		else if (type->kind() == MagicType::Kind::Block && _memberAccess.memberName() == "blockhash")
			m_errorReporter.typeError(
				8113_error,
				_memberAccess.location(),
				"\"block.blockhash()\" has been deprecated in favor of \"blockhash()\""
			);
		else if (type->kind() == MagicType::Kind::MetaType && _memberAccess.memberName() == "runtimeCode")
		{
			if (!m_constructorUsesAssembly)
				m_constructorUsesAssembly = std::make_unique<ConstructorUsesAssembly>();
			ContractType const& contract = dynamic_cast<ContractType const&>(*type->typeArgument());
			if (m_constructorUsesAssembly->check(contract.contractDefinition()))
				m_errorReporter.warning(
					6417_error,
					_memberAccess.location(),
					"The constructor of the contract (or its base) uses inline assembly. "
					"Because of that, it might be that the deployed bytecode is different from type(...).runtimeCode."
				);
		}
		else if (
			m_currentFunction &&
			m_currentFunction->isReceive() &&
			type->kind() == MagicType::Kind::Message &&
			_memberAccess.memberName() == "data"
		)
			m_errorReporter.typeError(
				7139_error,
				_memberAccess.location(),
				R"("msg.data" cannot be used inside of "receive" function.)"
			);
	}

	if (_memberAccess.memberName() == "callcode")
		if (auto const* type = dynamic_cast<FunctionType const*>(_memberAccess.annotation().type))
			if (type->kind() == FunctionType::Kind::BareCallCode)
				m_errorReporter.typeError(
					2256_error,
					_memberAccess.location(),
					"\"callcode\" has been deprecated in favour of \"delegatecall\"."
				);

	if (m_constructor)
	{
		auto const* expr = &_memberAccess.expression();
		while (expr)
		{
			if (auto id = dynamic_cast<Identifier const*>(expr))
			{
				if (id->name() == "this")
					m_errorReporter.warning(
						5805_error,
						id->location(),
						"\"this\" used in constructor. "
						"Note that external functions of a contract "
						"cannot be called while it is being constructed.");
				break;
			}
			else if (auto tuple = dynamic_cast<TupleExpression const*>(expr))
			{
				if (tuple->components().size() == 1)
					expr = tuple->components().front().get();
				else
					break;
			}
			else
				break;
		}
	}

	return true;
}

bool StaticAnalyzer::visit(InlineAssembly const& _inlineAssembly)
{
	if (!m_currentFunction)
		return true;

	for (auto const& ref: _inlineAssembly.annotation().externalReferences)
	{
		if (auto var = dynamic_cast<VariableDeclaration const*>(ref.second.declaration))
		{
			solAssert(!var->name().empty(), "");
			if (var->isLocalVariable())
				m_localVarUseCount[std::make_pair(var->id(), var)] += 1;
		}
	}

	return true;
}

bool StaticAnalyzer::visit(BinaryOperation const& _operation)
{
	if (
		*_operation.rightExpression().annotation().isPure &&
		(_operation.getOperator() == Token::Div || _operation.getOperator() == Token::Mod)
	)
		if (auto rhs = ConstantEvaluator::evaluate(m_errorReporter, _operation.rightExpression()))
			if (rhs->value == 0)
				m_errorReporter.typeError(
					1211_error,
					_operation.location(),
					(_operation.getOperator() == Token::Div) ? "Division by zero." : "Modulo zero."
				);

	return true;
}

bool StaticAnalyzer::visit(FunctionCall const& _functionCall)
{
	if (*_functionCall.annotation().kind == FunctionCallKind::FunctionCall)
	{
		auto functionType = dynamic_cast<FunctionType const*>(_functionCall.expression().annotation().type);
		solAssert(functionType, "");
		if (functionType->kind() == FunctionType::Kind::AddMod || functionType->kind() == FunctionType::Kind::MulMod)
		{
			solAssert(_functionCall.arguments().size() == 3, "");
			if (*_functionCall.arguments()[2]->annotation().isPure)
				if (auto lastArg = ConstantEvaluator::evaluate(m_errorReporter, *(_functionCall.arguments())[2]))
					if (lastArg->value == 0)
						m_errorReporter.typeError(
							4195_error,
							_functionCall.location(),
							"Arithmetic modulo zero."
						);
		}
		if (
			m_currentContract &&
			m_currentContract->isLibrary() &&
			functionType->kind() == FunctionType::Kind::DelegateCall &&
			functionType->declaration().scope() == m_currentContract
		)
			m_errorReporter.typeError(
				6700_error,
				_functionCall.location(),
				SecondarySourceLocation().append(
					"The function declaration is here:",
					functionType->declaration().scope()->location()
				),
				"Libraries cannot call their own functions externally."
			);
	}
	return true;
}
