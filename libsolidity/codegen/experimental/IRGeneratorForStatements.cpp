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
		type = m_context.env->resolve(*type);
		solAssert(*type == m_context.analysis.typeSystem().builtinType(BuiltinType::Word, {}));
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
	solAssert(_variableDeclarationStatement.declarations().size() == 1, "multi variable declarations not supported");
	solAssert(!_variableDeclarationStatement.initialValue(), "initial values not yet supported");
	VariableDeclaration const* variableDeclaration = _variableDeclarationStatement.declarations().front().get();
	solAssert(variableDeclaration);
	// TODO: check the type of the variable; register local variable; initialize
	m_code << "let " << IRNames::localVariable(*variableDeclaration) << "\n";
	return false;
}

bool IRGeneratorForStatements::visit(ExpressionStatement const&)
{
	return true;
}

bool IRGeneratorForStatements::visit(Identifier const& _identifier)
{
	auto const* rhsVar = dynamic_cast<VariableDeclaration const*>(_identifier.annotation().referencedDeclaration);
	solAssert(rhsVar, "Can only reference identifiers referring to variables.");
	m_code << "let " << IRNames::localVariable(_identifier) << " := " << IRNames::localVariable(*rhsVar) << "\n";
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

bool IRGeneratorForStatements::visit(FunctionCall const& _functionCall)
{
	for(auto arg: _functionCall.arguments())
		arg->accept(*this);

	FunctionDefinition const* functionDefinition = nullptr;
	if (auto const* identifier = dynamic_cast<Identifier const*>(&_functionCall.expression()))
	{
		functionDefinition = dynamic_cast<FunctionDefinition const*>(identifier->annotation().referencedDeclaration);
	}
	else if (auto const* memberAccess = dynamic_cast<MemberAccess const*>(&_functionCall.expression()))
	{
		auto const& expressionAnnotation = m_context.analysis.annotation<TypeInference>(memberAccess->expression());
		solAssert(expressionAnnotation.type);

		auto typeConstructor = std::get<0>(TypeSystemHelpers{m_context.analysis.typeSystem()}.destTypeExpression(
			m_context.analysis.typeSystem().env().resolve(*expressionAnnotation.type)
		));
		auto const* typeClass = dynamic_cast<Identifier const*>(&memberAccess->expression());
		solAssert(typeClass, "Function call to member access only supported for type classes.");
		auto const* typeClassDefinition = dynamic_cast<TypeClassDefinition const*>(typeClass->annotation().referencedDeclaration);
		solAssert(typeClassDefinition, "Function call to member access only supported for type classes.");
		auto const& classAnnotation = m_context.analysis.annotation<TypeRegistration>(*typeClassDefinition);
		TypeClassInstantiation const* instantiation = classAnnotation.instantiations.at(typeConstructor);
		for (auto const& node: instantiation->subNodes())
		{
			auto const* def = dynamic_cast<FunctionDefinition const*>(node.get());
			solAssert(def);
			if (def->name() == memberAccess->memberName())
			{
				functionDefinition = def;
				break;
			}
		}
	}
	else
		solAssert(false, "Complex function call expressions not supported.");

	solAssert(functionDefinition);
	auto functionType = m_context.analysis.annotation<TypeInference>(_functionCall).type;
	solAssert(functionType);
	functionType = m_context.analysis.typeSystem().env().resolve(*functionType);
	m_context.enqueueFunctionDefinition(functionDefinition, *functionType);
	m_code << "let " << IRNames::localVariable(_functionCall) << " := " << IRNames::function(*functionDefinition, *functionType) << "(";
	auto const& arguments = _functionCall.arguments();
	if (arguments.size() > 1)
		for (auto arg: arguments | ranges::views::drop_last(1))
			m_code << IRNames::localVariable(*arg) << ", ";
	if (!arguments.empty())
		m_code << IRNames::localVariable(*arguments.back());
	m_code << ")\n";
	return false;
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
