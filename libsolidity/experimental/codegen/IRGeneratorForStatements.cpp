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

#include <libsolidity/experimental/codegen/IRGeneratorForStatements.h>

#include <libsolidity/experimental/analysis/Analysis.h>
#include <libsolidity/experimental/analysis/TypeClassRegistration.h>
#include <libsolidity/experimental/analysis/TypeInference.h>
#include <libsolidity/experimental/analysis/TypeRegistration.h>

#include <libsolidity/experimental/ast/TypeSystemHelper.h>

#include <libyul/YulStack.h>
#include <libyul/AsmPrinter.h>
#include <libyul/AST.h>
#include <libyul/optimiser/ASTCopier.h>

#include <libsolidity/experimental/codegen/Common.h>

#include <range/v3/view/drop_last.hpp>
#include <range/v3/view/zip.hpp>

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


namespace
{

struct CopyTranslate: public yul::ASTCopier
{
	CopyTranslate(
		IRGenerationContext const& _context,
		yul::Dialect const& _dialect,
		std::map<yul::Identifier const*, InlineAssemblyAnnotation::ExternalIdentifierInfo> _references
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
		solAssert(std::holds_alternative<yul::Identifier>(translated));
		return std::get<yul::Identifier>(std::move(translated));
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
		std::string value = IRVariable{*varDecl, *type, IRGeneratorForStatements::stackSize(m_context, *type)}.name();
		return yul::Identifier{_identifier.debugData, yul::YulString{value}};
	}

	IRGenerationContext const& m_context;
	yul::Dialect const& m_dialect;
	std::map<yul::Identifier const*, InlineAssemblyAnnotation::ExternalIdentifierInfo> m_references;
};

}

std::size_t IRGeneratorForStatements::stackSize(IRGenerationContext const& _context, Type _type)
{
	TypeSystemHelpers helper{_context.analysis.typeSystem()};
	_type = _context.env->resolve(_type);
	solAssert(std::holds_alternative<TypeConstant>(_type), "No monomorphized type.");

	// type -> # stack slots
	// unit, itself -> 0
	// void, literals(integer), typeFunction -> error (maybe generate a revert)
	// word, bool, function -> 1
	// pair -> sum(stackSize(args))
	// user-defined -> stackSize(underlying type)
	TypeConstant typeConstant = std::get<TypeConstant>(_type);
	if (
		helper.isPrimitiveType(_type, PrimitiveType::Unit) ||
		helper.isPrimitiveType(_type, PrimitiveType::Itself)
	)
		return 0;
	else if (
		helper.isPrimitiveType(_type, PrimitiveType::Bool) ||
		helper.isPrimitiveType(_type, PrimitiveType::Word)
	)
	{
		solAssert(typeConstant.arguments.empty(), "Primitive type Bool or Word should have no arguments.");
		return 1;
	}
	else if (helper.isFunctionType(_type))
		return 1;
	else if (
		helper.isPrimitiveType(_type, PrimitiveType::Integer) ||
		helper.isPrimitiveType(_type, PrimitiveType::Void) ||
		helper.isPrimitiveType(_type, PrimitiveType::TypeFunction)
	)
		solAssert(false, "Attempted to query the stack size of a type without stack representation.");
	else if (helper.isPrimitiveType(_type, PrimitiveType::Pair))
	{
		solAssert(typeConstant.arguments.size() == 2);
		return stackSize(_context, typeConstant.arguments.front()) + stackSize(_context, typeConstant.arguments.back());
	}
	else
	{
		Type underlyingType = _context.env->resolve(
			_context.analysis.annotation<TypeInference>().underlyingTypes.at(typeConstant.constructor));
		if (helper.isTypeConstant(underlyingType))
			return stackSize(_context, underlyingType);

		TypeEnvironment env = _context.env->clone();
		Type genericFunctionType = helper.typeFunctionType(
			helper.tupleType(typeConstant.arguments),
			env.typeSystem().freshTypeVariable({}));
		solAssert(env.unify(genericFunctionType, underlyingType).empty());

		Type resolvedType = env.resolveRecursive(genericFunctionType);
		auto [argumentType, resultType] = helper.destTypeFunctionType(resolvedType);
		return stackSize(_context, resultType);
	}

	//TODO: sum types
	return 0;
}

bool IRGeneratorForStatements::visit(TupleExpression const& _tupleExpression)
{
	std::vector<std::string> components;
	for (auto const& component: _tupleExpression.components())
	{
		solUnimplementedAssert(component);
		component->accept(*this);
		components.emplace_back(var(*component).commaSeparatedList());
	}

	solUnimplementedAssert(false, "No support for tuples.");

	return false;
}

bool IRGeneratorForStatements::visit(InlineAssembly const& _assembly)
{
	CopyTranslate bodyCopier{m_context, _assembly.dialect(), _assembly.annotation().externalReferences};
	yul::Statement modified = bodyCopier(_assembly.operations());
	solAssert(std::holds_alternative<yul::Block>(modified));
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
	if (_variableDeclarationStatement.initialValue())
		define(var(*variableDeclaration), var(*_variableDeclarationStatement.initialValue()));
	else
		declare(var(*variableDeclaration));

	return false;
}

bool IRGeneratorForStatements::visit(ExpressionStatement const&)
{
	return true;
}

bool IRGeneratorForStatements::visit(Identifier const& _identifier)
{
	if (auto const* variable = dynamic_cast<VariableDeclaration const*>(_identifier.annotation().referencedDeclaration))
		define(var(_identifier), var(*variable));
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
		solAssert(_return.annotation().function, "Invalid return.");
		solAssert(_return.annotation().function->experimentalReturnExpression(), "Invalid return.");
		auto returnExpression = _return.annotation().function->experimentalReturnExpression();
		assign(var(*returnExpression), var(*value));
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
	auto [typeClass, memberName] = m_context.analysis.annotation<TypeInference>().operators.at(_binaryOperation.getOperator());
	auto const& functionDefinition = resolveTypeClassFunction(typeClass, memberName, functionType);
	std::string result = var(_binaryOperation).commaSeparatedList();
	if (!result.empty())
		m_code << "let " << result << " := ";
	m_code << buildFunctionCall(functionDefinition, functionType, _binaryOperation.arguments());
}

std::string IRGeneratorForStatements::buildFunctionCall(FunctionDefinition const& _functionDefinition, Type _functionType, std::vector<ASTPointer<Expression const>> const& _arguments)
{
	// Ensure type is resolved
	// TODO: get around resolveRecursive by passing the environment further down?
	Type resolvedFunctionType = m_context.env->resolveRecursive(_functionType);
	m_context.enqueueFunctionDefinition(&_functionDefinition, resolvedFunctionType);

	std::ostringstream output;
	output << IRNames::function(*m_context.env, _functionDefinition, resolvedFunctionType) << "(";
	if (_arguments.size() == 1)
		output << var(*_arguments.back()).commaSeparatedList();
	else if (_arguments.size() > 1)
	{
		for (auto arg: _arguments | ranges::views::drop_last(1))
			output << var(*arg).commaSeparatedList();
		output << var(*_arguments.back()).commaSeparatedListPrefixed();
	}
	output << ")\n";
	return output.str();
}

void IRGeneratorForStatements::assign(IRVariable const& _lhs, IRVariable const& _rhs, bool _declare)
{
	solAssert(stackSize(m_context, _lhs.type()) == stackSize(m_context, _rhs.type()));
	for (auto&& [lhsSlot, rhsSlot]: ranges::zip_view(_lhs.stackSlots(), _rhs.stackSlots()))
		m_code << (_declare ? "let " : "") << lhsSlot << " := " <<  rhsSlot << "\n";
}

void IRGeneratorForStatements::declare(IRVariable const& _var)
{
	if (_var.stackSize() > 0)
		m_code << "let " << _var.commaSeparatedList() << "\n";
}

namespace
{
TypeRegistration::TypeClassInstantiations const& typeClassInstantiations(IRGenerationContext const& _context, TypeClass _class)
{
	auto const* typeClassDeclaration = _context.analysis.typeSystem().typeClassDeclaration(_class);
	if (typeClassDeclaration)
		return _context.analysis.annotation<TypeRegistration>(*typeClassDeclaration).instantiations;
	// TODO: better mechanism than fetching by name.
	auto& instantiations = _context.analysis.annotation<TypeRegistration>().builtinClassInstantiations;
	auto& builtinClassesByName = _context.analysis.annotation<TypeInference>().builtinClassesByName;
	return instantiations.at(builtinClassesByName.at(_context.analysis.typeSystem().typeClassName(_class)));
}
}

FunctionDefinition const& IRGeneratorForStatements::resolveTypeClassFunction(TypeClass _class, std::string _name, Type _type)
{
	TypeSystemHelpers helper{m_context.analysis.typeSystem()};

	TypeEnvironment env = m_context.env->clone();
	Type genericFunctionType = env.fresh(m_context.analysis.annotation<TypeInference>().typeClassFunctions.at(_class).at(_name));
	auto typeVars = TypeEnvironmentHelpers{env}.typeVars(genericFunctionType);
	solAssert(typeVars.size() == 1);
	solAssert(env.unify(genericFunctionType, _type).empty());
	auto typeClassInstantiation = std::get<0>(helper.destTypeConstant(env.resolve(typeVars.front())));

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
	// TODO: avoid resolve?
	auto expressionType = m_context.env->resolve(type(_memberAccess.expression()));
	auto constructor = std::get<0>(helper.destTypeConstant(expressionType));
	auto memberAccessType = type(_memberAccess);
	// TODO: better mechanism
	if (constructor == m_context.analysis.typeSystem().constructor(PrimitiveType::Bool))
	{
		if (_memberAccess.memberName() == "abs")
			solAssert(m_expressionDeclaration.emplace(&_memberAccess, Builtins::ToBool).second);
		else if (_memberAccess.memberName() == "rep")
			solAssert(m_expressionDeclaration.emplace(&_memberAccess, Builtins::FromBool).second);
		return;
	}
	auto const* declaration = m_context.analysis.typeSystem().constructorInfo(constructor).typeDeclaration;
	solAssert(declaration);
	if (auto const* typeClassDefinition = dynamic_cast<TypeClassDefinition const*>(declaration))
	{
		solAssert(m_context.analysis.annotation<TypeClassRegistration>(*typeClassDefinition).typeClass.has_value());
		TypeClass typeClass = m_context.analysis.annotation<TypeClassRegistration>(*typeClassDefinition).typeClass.value();
		solAssert(m_expressionDeclaration.emplace(
			&_memberAccess,
			&resolveTypeClassFunction(typeClass, _memberAccess.memberName(), memberAccessType)
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

bool IRGeneratorForStatements::visit(ElementaryTypeNameExpression const&)
{
	// TODO: is this always a no-op?
	return false;
}

void IRGeneratorForStatements::endVisit(FunctionCall const& _functionCall)
{
	Type functionType = type(_functionCall.expression());
	solUnimplementedAssert(m_expressionDeclaration.count(&_functionCall.expression()) != 0, "No support for calling functions pointers yet.");
	auto declaration = m_expressionDeclaration.at(&_functionCall.expression());
	if (auto builtin = std::get_if<Builtins>(&declaration))
	{
		switch (*builtin)
		{
		case Builtins::FromBool:
		case Builtins::Identity:
			solAssert(_functionCall.arguments().size() == 1);
			define(var(_functionCall), var(*_functionCall.arguments().front()));
			return;
		case Builtins::ToBool:
			solAssert(_functionCall.arguments().size() == 1);
			m_code << "let " << var(_functionCall).name() << " := iszero(iszero(" << var(*_functionCall.arguments().front()).name() << "))\n";
			return;
		}
		solAssert(false);
	}
	FunctionDefinition const* functionDefinition = dynamic_cast<FunctionDefinition const*>(std::get<Declaration const*>(declaration));
	solAssert(functionDefinition);
	// TODO: account for return stack size
	solAssert(!functionDefinition->returnParameterList());
	std::string result = var(_functionCall).commaSeparatedList();
	if (!result.empty())
		m_code << "let " << result << " := ";
	m_code << buildFunctionCall(*functionDefinition, functionType, _functionCall.arguments());
}

bool IRGeneratorForStatements::visit(FunctionCall const&)
{
	return true;
}

bool IRGeneratorForStatements::visit(Block const& _block)
{
	m_code << "{\n";
	solAssert(!_block.unchecked());
	for (auto const& statement: _block.statements())
		statement->accept(*this);
	m_code << "}\n";
	return false;
}

bool IRGeneratorForStatements::visit(IfStatement const& _ifStatement)
{
	_ifStatement.condition().accept(*this);
	if (_ifStatement.falseStatement())
	{
		m_code << "switch " << var(_ifStatement.condition()).name() << " {\n";
		m_code << "case 0 {\n";
		_ifStatement.falseStatement()->accept(*this);
		m_code << "}\n";
		m_code << "default {\n";
		_ifStatement.trueStatement().accept(*this);
		m_code << "}\n";
	}
	else
	{
		m_code << "if " << var(_ifStatement.condition()).name() << " {\n";
		_ifStatement.trueStatement().accept(*this);
		m_code << "}\n";
	}
	return false;
}

bool IRGeneratorForStatements::visit(Assignment const& _assignment)
{
	_assignment.rightHandSide().accept(*this);
	auto const* lhs = dynamic_cast<Identifier const*>(&_assignment.leftHandSide());
	solAssert(lhs, "Can only assign to identifiers.");
	auto const* lhsVar = dynamic_cast<VariableDeclaration const*>(lhs->annotation().referencedDeclaration);
	solAssert(lhsVar, "Can only assign to identifiers referring to variables.");
	assign(var(*lhsVar), var(_assignment.rightHandSide()));
	define(var(_assignment), var(*lhsVar));
	return false;
}


bool IRGeneratorForStatements::visitNode(ASTNode const&)
{
	solAssert(false, "Unsupported AST node during statement code generation.");
}
