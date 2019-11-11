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
 * @author Federico Bond <federicobond@gmail.com>
 * @date 2016
 * Static analyzer and checker.
 */

#include <libsolidity/analysis/StaticAnalyzer.h>

#include <libsolidity/analysis/ConstantEvaluator.h>
#include <libsolidity/ast/AST.h>
#include <liblangutil/ErrorReporter.h>
#include <memory>

using namespace std;
using namespace dev;
using namespace langutil;
using namespace dev::solidity;

/**
 * Helper class that determines whether a contract's constructor uses inline assembly.
 */
class dev::solidity::ConstructorUsesAssembly
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

	map<ContractDefinition const*, bool> m_usesAssembly;
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
	return Error::containsOnlyWarnings(m_errorReporter.errors());
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
				if (var.first.second->isCallableParameter())
					m_errorReporter.warning(
						var.first.second->location(),
						"Unused function parameter. Remove or comment out the variable name to silence this warning."
					);
				else
					m_errorReporter.warning(var.first.second->location(), "Unused local variable.");
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
				m_localVarUseCount[make_pair(var->id(), var)] += 1;
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
			m_localVarUseCount[make_pair(_variable.id(), &_variable)] += 0;
	}
	else if (_variable.isStateVariable())
	{
		set<StructDefinition const*> structsSeen;
		if (structureSizeEstimate(*_variable.type(), structsSeen) >= bigint(1) << 64)
			m_errorReporter.warning(
				_variable.location(),
				"Variable covers a large part of storage and thus makes collisions likely. "
				"Either use mappings or dynamic arrays and allow their size to be increased only "
				"in small quantities per transaction."
			);
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
				m_localVarUseCount[make_pair(var->id(), var.get())] += 1;
	return true;
}

bool StaticAnalyzer::visit(ExpressionStatement const& _statement)
{
	if (_statement.expression().annotation().isPure)
		m_errorReporter.warning(
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
				_memberAccess.location(),
				"\"msg.gas\" has been deprecated in favor of \"gasleft()\""
			);
		else if (type->kind() == MagicType::Kind::Block && _memberAccess.memberName() == "blockhash")
			m_errorReporter.typeError(
				_memberAccess.location(),
				"\"block.blockhash()\" has been deprecated in favor of \"blockhash()\""
			);
		else if (type->kind() == MagicType::Kind::MetaType && _memberAccess.memberName() == "runtimeCode")
		{
			if (!m_constructorUsesAssembly)
				m_constructorUsesAssembly = make_unique<ConstructorUsesAssembly>();
			ContractType const& contract = dynamic_cast<ContractType const&>(*type->typeArgument());
			if (m_constructorUsesAssembly->check(contract.contractDefinition()))
				m_errorReporter.warning(
					_memberAccess.location(),
					"The constructor of the contract (or its base) uses inline assembly. "
					"Because of that, it might be that the deployed bytecode is different from type(...).runtimeCode."
				);
		}
	}

	if (_memberAccess.memberName() == "callcode")
		if (auto const* type = dynamic_cast<FunctionType const*>(_memberAccess.annotation().type))
			if (type->kind() == FunctionType::Kind::BareCallCode)
				m_errorReporter.typeError(
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
				m_localVarUseCount[make_pair(var->id(), var)] += 1;
		}
	}

	return true;
}

bool StaticAnalyzer::visit(BinaryOperation const& _operation)
{
	if (
		_operation.rightExpression().annotation().isPure &&
		(_operation.getOperator() == Token::Div || _operation.getOperator() == Token::Mod)
	)
		if (auto rhs = dynamic_cast<RationalNumberType const*>(
			ConstantEvaluator(m_errorReporter).evaluate(_operation.rightExpression())
		))
			if (rhs->isZero())
				m_errorReporter.typeError(
					_operation.location(),
					(_operation.getOperator() == Token::Div) ? "Division by zero." : "Modulo zero."
				);

	return true;
}

bool StaticAnalyzer::visit(FunctionCall const& _functionCall)
{
	if (_functionCall.annotation().kind == FunctionCallKind::FunctionCall)
	{
		auto functionType = dynamic_cast<FunctionType const*>(_functionCall.expression().annotation().type);
		solAssert(functionType, "");
		if (functionType->kind() == FunctionType::Kind::AddMod || functionType->kind() == FunctionType::Kind::MulMod)
		{
			solAssert(_functionCall.arguments().size() == 3, "");
			if (_functionCall.arguments()[2]->annotation().isPure)
				if (auto lastArg = dynamic_cast<RationalNumberType const*>(
					ConstantEvaluator(m_errorReporter).evaluate(*(_functionCall.arguments())[2])
				))
					if (lastArg->isZero())
						m_errorReporter.typeError(
							_functionCall.location(),
							"Arithmetic modulo zero."
						);
		}
		if (
			m_currentContract->isLibrary() &&
			functionType->kind() == FunctionType::Kind::DelegateCall &&
			functionType->declaration().scope() == m_currentContract
		)
			m_errorReporter.typeError(
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

bigint StaticAnalyzer::structureSizeEstimate(Type const& _type, set<StructDefinition const*>& _structsSeen)
{
	switch (_type.category())
	{
	case Type::Category::Array:
	{
		auto const& t = dynamic_cast<ArrayType const&>(_type);
		return structureSizeEstimate(*t.baseType(), _structsSeen) * (t.isDynamicallySized() ? 1 : t.length());
	}
	case Type::Category::Struct:
	{
		auto const& t = dynamic_cast<StructType const&>(_type);
		bigint size = 1;
		if (!_structsSeen.count(&t.structDefinition()))
		{
			_structsSeen.insert(&t.structDefinition());
			for (auto const& m: t.members(nullptr))
				size += structureSizeEstimate(*m.type, _structsSeen);
		}
		return size;
	}
	case Type::Category::Mapping:
	{
		return structureSizeEstimate(*dynamic_cast<MappingType const&>(_type).valueType(), _structsSeen);
	}
	default:
		break;
	}
	return bigint(1);
}
