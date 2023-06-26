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

#include <libsolidity/codegen/experimental/IRGeneratorForStatements.h>

#include <libsolidity/analysis/experimental/Analysis.h>
#include <libsolidity/analysis/experimental/TypeInference.h>
#include <libsolidity/analysis/experimental/TypeRegistration.h>

#include <libsolidity/ast/experimental/TypeSystemHelper.h>

#include <libyul/YulStack.h>
#include <libyul/AsmPrinter.h>
#include <libyul/AST.h>
#include <libyul/optimiser/ASTCopier.h>

#include <libsolidity/codegen/experimental/Common.h>

#include <range/v3/view/drop_last.hpp>

using namespace std;
using namespace solidity;
using namespace solidity::util;
using namespace solidity::frontend;
using namespace solidity::frontend::experimental;
using namespace std::string_literals;

std::string IRGeneratorForStatements::generate(ASTNode const& _node)
{
	_node.accept(*this);
	return m_code.str();
}


namespace {

struct CopyTranslate: public yul::ASTCopier
{
	CopyTranslate(
		IRGenerationContext const& _context,
		yul::Dialect const& _dialect,
		map<yul::Identifier const*, InlineAssemblyAnnotation::ExternalIdentifierInfo> _references
	): m_context(_context), m_dialect(_dialect), m_references(std::move(_references)) {}

	using ASTCopier::operator();

	yul::Expression operator()(yul::Identifier const& _identifier) override
	{
		// The operator() function is only called in lvalue context. In rvalue context,
		// only translate(yul::Identifier) is called.
		if (m_references.count(&_identifier))
			return translateReference(_identifier);
		else
			return ASTCopier::operator()(_identifier);
	}

	yul::YulString translateIdentifier(yul::YulString _name) override
	{
		if (m_dialect.builtin(_name))
			return _name;
		else
			return yul::YulString{"usr$" + _name.str()};
	}

	yul::Identifier translate(yul::Identifier const& _identifier) override
	{
		if (!m_references.count(&_identifier))
			return ASTCopier::translate(_identifier);

		yul::Expression translated = translateReference(_identifier);
		solAssert(holds_alternative<yul::Identifier>(translated));
		return get<yul::Identifier>(std::move(translated));
	}

private:

	/// Translates a reference to a local variable, potentially including
	/// a suffix. Might return a literal, which causes this to be invalid in
	/// lvalue-context.
	yul::Expression translateReference(yul::Identifier const& _identifier)
	{
		auto const& reference = m_references.at(&_identifier);
		auto const varDecl = dynamic_cast<VariableDeclaration const*>(reference.declaration);
		solAssert(varDecl, "External reference in inline assembly to something that is not a variable declaration.");
		auto type = m_context.analysis.annotation<TypeInference>(*varDecl).type;
		solAssert(type);
		solAssert(m_context.env->typeEquals(*type, m_context.analysis.typeSystem().type(PrimitiveType::Word, {})));
		string value = IRNames::localVariable(*varDecl);
		return yul::Identifier{_identifier.debugData, yul::YulString{value}};
	}

	IRGenerationContext const& m_context;
	yul::Dialect const& m_dialect;
	map<yul::Identifier const*, InlineAssemblyAnnotation::ExternalIdentifierInfo> m_references;
};

}

bool IRGeneratorForStatements::visit(InlineAssembly const& _assembly)
{
	CopyTranslate bodyCopier{m_context, _assembly.dialect(), _assembly.annotation().externalReferences};
	yul::Statement modified = bodyCopier(_assembly.operations());
	solAssert(holds_alternative<yul::Block>(modified));
	m_code << yul::AsmPrinter()(std::get<yul::Block>(modified)) << "\n";
	return false;
}

bool IRGeneratorForStatements::visit(VariableDeclarationStatement const& _variableDeclarationStatement)
{
	if (_variableDeclarationStatement.initialValue())
		_variableDeclarationStatement.initialValue()->accept(*this);
	solAssert(_variableDeclarationStatement.declarations().size() == 1, "multi variable declarations not supported");
	VariableDeclaration const* variableDeclaration = _variableDeclarationStatement.declarations().front().get();
	solAssert(variableDeclaration);
	// TODO: check the type of the variable; register local variable; initialize
	m_code << "let " << IRNames::localVariable(*variableDeclaration);
	if (_variableDeclarationStatement.initialValue())
		m_code << " := " << IRNames::localVariable(*_variableDeclarationStatement.initialValue());
	m_code << "\n";
	return false;
}

bool IRGeneratorForStatements::visit(ExpressionStatement const&)
{
	return true;
}

bool IRGeneratorForStatements::visit(Identifier const& _identifier)
{
	if (auto const* var = dynamic_cast<VariableDeclaration const*>(_identifier.annotation().referencedDeclaration))
	{
		m_code << "let " << IRNames::localVariable(_identifier) << " := " << IRNames::localVariable(*var) << "\n";
	}
	else if (auto const* function = dynamic_cast<FunctionDefinition const*>(_identifier.annotation().referencedDeclaration))
		solAssert(m_expressionDeclaration.emplace(&_identifier, function).second);
	else if (auto const* typeClass = dynamic_cast<TypeClassDefinition const*>(_identifier.annotation().referencedDeclaration))
		solAssert(m_expressionDeclaration.emplace(&_identifier, typeClass).second);
	else if (auto const* typeDefinition = dynamic_cast<TypeDefinition const*>(_identifier.annotation().referencedDeclaration))
		solAssert(m_expressionDeclaration.emplace(&_identifier, typeDefinition).second);
	else
		solAssert(false, "Unsupported Identifier");
	return false;
}

void IRGeneratorForStatements::endVisit(Return const& _return)
{
	if (Expression const* value = _return.expression())
	{
		solAssert(_return.annotation().functionReturnParameters, "Invalid return parameters pointer.");
		vector<ASTPointer<VariableDeclaration>> const& returnParameters =
			_return.annotation().functionReturnParameters->parameters();
		solAssert(returnParameters.size() == 1, "Returning tuples not yet supported.");
		m_code << IRNames::localVariable(*returnParameters.front()) << " := " << IRNames::localVariable(*value) << "\n";
	}

	m_code << "leave\n";
}

experimental::Type IRGeneratorForStatements::type(ASTNode const& _node) const
{
	auto type = m_context.analysis.annotation<TypeInference>(_node).type;
	solAssert(type);
	return *type;
}

void IRGeneratorForStatements::endVisit(BinaryOperation const& _binaryOperation)
{
	TypeSystemHelpers helper{m_context.analysis.typeSystem()};
	Type leftType = type(_binaryOperation.leftExpression());
	Type rightType = type(_binaryOperation.rightExpression());
	Type resultType = type(_binaryOperation);
	Type functionType = helper.functionType(helper.tupleType({leftType, rightType}), resultType);
	auto [typeClass, memberName] = m_context.analysis.annotation<TypeRegistration>().operators.at(_binaryOperation.getOperator());
	auto const& functionDefinition = resolveTypeClassFunction(typeClass, memberName, functionType);
	// TODO: deduplicate with FunctionCall
	// TODO: get around resolveRecursive by passing the environment further down?
	functionType = m_context.env->resolveRecursive(functionType);
	m_context.enqueueFunctionDefinition(&functionDefinition, functionType);
	// TODO: account for return stack size
	m_code << "let " << IRNames::localVariable(_binaryOperation) << " := " << IRNames::function(*m_context.env, functionDefinition, functionType) << "("
		<< IRNames::localVariable(_binaryOperation.leftExpression()) << ", " << IRNames::localVariable(_binaryOperation.rightExpression()) << ")\n";
}

namespace
{
TypeRegistration::TypeClassInstantiations const& typeClassInstantiations(IRGenerationContext const& _context, TypeClass _class)
{
	auto const* typeClassDeclaration = _context.analysis.typeSystem().typeClassDeclaration(_class);
	if (typeClassDeclaration)
		return _context.analysis.annotation<TypeRegistration>(*typeClassDeclaration).instantiations;
	// TODO: better mechanism than fetching by name.
	auto& annotation = _context.analysis.annotation<TypeRegistration>();
	return annotation.builtinClassInstantiations.at(annotation.builtinClassesByName.at(_context.analysis.typeSystem().typeClassName(_class)));
}
}

FunctionDefinition const& IRGeneratorForStatements::resolveTypeClassFunction(TypeClass _class, string _name, Type _type)
{
	TypeSystemHelpers helper{m_context.analysis.typeSystem()};

	TypeEnvironment env = m_context.env->clone();
	std::optional<Type> genericFunctionType = env.typeClassFunction(_class, _name);
	solAssert(genericFunctionType);
	auto typeVars = TypeEnvironmentHelpers{env}.typeVars(*genericFunctionType);
	solAssert(typeVars.size() == 1);
	solAssert(env.unify(*genericFunctionType, _type).empty());
	auto typeClassInstantiation = get<0>(helper.destTypeConstant(env.resolve(typeVars.front())));

	auto const& instantiations = typeClassInstantiations(m_context, _class);
	TypeClassInstantiation const* instantiation = instantiations.at(typeClassInstantiation);
	FunctionDefinition const* functionDefinition = nullptr;
	for (auto const& node: instantiation->subNodes())
	{
		auto const* def = dynamic_cast<FunctionDefinition const*>(node.get());
		solAssert(def);
		if (def->name() == _name)
		{
			functionDefinition = def;
			break;
		}
	}
	solAssert(functionDefinition);
	return *functionDefinition;
}

void IRGeneratorForStatements::endVisit(MemberAccess const& _memberAccess)
{
	TypeSystemHelpers helper{m_context.analysis.typeSystem()};
	auto expressionType = type(_memberAccess.expression());
	auto constructor = std::get<0>(helper.destTypeConstant(expressionType));
	auto memberAccessType = type(_memberAccess);
	auto const* declaration = m_context.analysis.typeSystem().constructorInfo(constructor).typeDeclaration;
	solAssert(declaration);
	if (auto const* typeClassDefinition = dynamic_cast<TypeClassDefinition const*>(declaration))
	{
		optional<TypeClass> typeClass = m_context.analysis.annotation<TypeInference>(*typeClassDefinition).typeClass;
		solAssert(typeClass);
		solAssert(m_expressionDeclaration.emplace(
			&_memberAccess,
			&resolveTypeClassFunction(*typeClass, _memberAccess.memberName(), memberAccessType)
		).second);
	}
	else if (dynamic_cast<TypeDefinition const*>(declaration))
	{
		if (_memberAccess.memberName() == "abs" || _memberAccess.memberName() == "rep")
			solAssert(m_expressionDeclaration.emplace(&_memberAccess, Builtins::Identity).second);
		else
			solAssert(false);
	}
	else
		solAssert(false);
}

void IRGeneratorForStatements::endVisit(FunctionCall const& _functionCall)
{
	Type functionType = type(_functionCall.expression());
	auto declaration = m_expressionDeclaration.at(&_functionCall.expression());
	if (auto builtin = get_if<Builtins>(&declaration))
	{
		switch(*builtin)
		{
		case Builtins::Identity:
			solAssert(_functionCall.arguments().size() == 1);
			m_code << "let " << IRNames::localVariable(_functionCall) << " := " << IRNames::localVariable(*_functionCall.arguments().front()) << "\n";
			return;
		}
		solAssert(false);
	}
	FunctionDefinition const* functionDefinition = dynamic_cast<FunctionDefinition const*>(get<Declaration const*>(declaration));
	solAssert(functionDefinition);
	// TODO: get around resolveRecursive by passing the environment further down?
	functionType = m_context.env->resolveRecursive(functionType);
	m_context.enqueueFunctionDefinition(functionDefinition, functionType);
	// TODO: account for return stack size
	m_code << "let " << IRNames::localVariable(_functionCall) << " := " << IRNames::function(*m_context.env, *functionDefinition, functionType) << "(";
	auto const& arguments = _functionCall.arguments();
	if (arguments.size() > 1)
		for (auto arg: arguments | ranges::views::drop_last(1))
			m_code << IRNames::localVariable(*arg) << ", ";
	if (!arguments.empty())
		m_code << IRNames::localVariable(*arguments.back());
	m_code << ")\n";
}

bool IRGeneratorForStatements::visit(FunctionCall const&)
{
	return true;
}

bool IRGeneratorForStatements::visit(Assignment const& _assignment)
{
	_assignment.rightHandSide().accept(*this);
	auto const* lhs = dynamic_cast<Identifier const*>(&_assignment.leftHandSide());
	solAssert(lhs, "Can only assign to identifiers.");
	auto const* lhsVar = dynamic_cast<VariableDeclaration const*>(lhs->annotation().referencedDeclaration);
	solAssert(lhsVar, "Can only assign to identifiers referring to variables.");
	m_code << IRNames::localVariable(*lhsVar) << " := " << IRNames::localVariable(_assignment.rightHandSide()) << "\n";

	m_code << "let " << IRNames::localVariable(_assignment) << " := " << IRNames::localVariable(*lhsVar) << "\n";
	return false;
}


bool IRGeneratorForStatements::visitNode(ASTNode const&)
{
	solAssert(false, "Unsupported AST node during statement code generation.");
}
