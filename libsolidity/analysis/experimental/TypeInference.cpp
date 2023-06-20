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


#include <libsolidity/analysis/experimental/TypeInference.h>
#include <libsolidity/analysis/experimental/Analysis.h>
#include <liblangutil/Exceptions.h>

#include <libyul/AsmAnalysis.h>
#include <libyul/AsmAnalysisInfo.h>
#include <libyul/AST.h>

#include <range/v3/view/transform.hpp>

using namespace std;
using namespace solidity::frontend;
using namespace solidity::frontend::experimental;
using namespace solidity::langutil;

TypeInference::TypeInference(Analysis& _analysis):
m_analysis(_analysis),
m_errorReporter(_analysis.errorReporter())
{
	for (auto [type, name, arity]: std::initializer_list<std::tuple<BuiltinType, const char*, uint64_t>> {
			 {BuiltinType::Void, "void", 0},
			 {BuiltinType::Function, "fun", 2},
			 {BuiltinType::Unit, "unit", 0},
			 {BuiltinType::Pair, "pair", 2},
			 {BuiltinType::Word, "word", 0}
	})
		m_typeSystem.declareBuiltinType(type, name, arity);
	m_voidType = m_typeSystem.builtinType(BuiltinType::Void, {});
	m_wordType = m_typeSystem.builtinType(BuiltinType::Word, {});
	m_env = make_unique<TypeEnvironment>(m_typeSystem);

	m_typeAnnotations.resize(_analysis.maxAstId());
}

bool TypeInference::analyze(SourceUnit const& _sourceUnit)
{
	_sourceUnit.accept(*this);
	return !m_errorReporter.hasErrors();
}

bool TypeInference::visit(FunctionDefinition const& _functionDefinition)
{
	auto& functionAnnotation = annotation(_functionDefinition);
	if (functionAnnotation.type)
		return false;

	Type functionType;
	{
		_functionDefinition.parameterList().accept(*this);
		if (_functionDefinition.returnParameterList())
			_functionDefinition.returnParameterList()->accept(*this);

		_functionDefinition.body().accept(*this);

		auto typeFromParameterList = [&](ParameterList const* _list) {
			if (!_list)
				return m_typeSystem.builtinType(BuiltinType::Unit, {});
			return TypeSystemHelpers{m_typeSystem}.tupleType(_list->parameters() | ranges::view::transform([&](auto _param) {
				auto& argAnnotation = annotation(*_param);
				solAssert(argAnnotation.type);
				return *argAnnotation.type;
			}) | ranges::to<std::vector<Type>>);
		};

		Type argType = typeFromParameterList(&_functionDefinition.parameterList());
		Type resultType = typeFromParameterList(_functionDefinition.returnParameterList().get());

		functionType = m_typeSystem.builtinType(BuiltinType::Function, {argType, resultType});
	}
	functionAnnotation.type = functionType;

	m_errorReporter.warning(0000_error, _functionDefinition.location(), m_typeSystem.typeToString(m_typeSystem.resolve(functionType)));

	return false;
}

bool TypeInference::visit(ParameterList const&)
{
	return true;
}

void TypeInference::endVisit(ParameterList const& _parameterList)
{
	auto& listAnnotation = annotation(_parameterList);
	solAssert(!listAnnotation.type);
	std::vector<Type> argTypes;
	for(auto arg: _parameterList.parameters())
	{
		auto& argAnnotation = annotation(*arg);
		solAssert(argAnnotation.type);
		argTypes.emplace_back(*argAnnotation.type);
	}
	listAnnotation.type = TypeSystemHelpers{m_typeSystem}.tupleType(argTypes);
}

bool TypeInference::visitNode(ASTNode const& _node)
{
	m_errorReporter.typeError(0000_error, _node.location(), "Unsupported AST node during type inference.");
	return false;
}

experimental::Type TypeInference::fromTypeName(TypeName const& _typeName)
{
	if (auto const* elementaryTypeName = dynamic_cast<ElementaryTypeName const*>(&_typeName))
	{
		switch(elementaryTypeName->typeName().token())
		{
		case Token::Word:
			return m_wordType;
		case Token::Void:
			return m_voidType;
		default:
			m_errorReporter.typeError(0000_error, _typeName.location(), "Only elementary types are supported.");
			break;
		}
	}
	else
		m_errorReporter.typeError(0000_error, _typeName.location(), "Only elementary types are supported.");
	// TODO: free type?
	return m_typeSystem.freshTypeVariable();

}

bool TypeInference::visit(InlineAssembly const& _inlineAssembly)
{
	// External references have already been resolved in a prior stage and stored in the annotation.
	// We run the resolve step again regardless.
	yul::ExternalIdentifierAccess::Resolver identifierAccess = [&](
		yul::Identifier const& _identifier,
		yul::IdentifierContext _context,
		bool
	) -> bool
	{
		if (_context == yul::IdentifierContext::NonExternal)
		{
			// Hack until we can disallow any shadowing: If we found an internal reference,
			// clear the external references, so that codegen does not use it.
			_inlineAssembly.annotation().externalReferences.erase(& _identifier);
			return false;
		}
		auto ref = _inlineAssembly.annotation().externalReferences.find(&_identifier);
		if (ref == _inlineAssembly.annotation().externalReferences.end())
			return false;
		InlineAssemblyAnnotation::ExternalIdentifierInfo& identifierInfo = ref->second;
		Declaration const* declaration = identifierInfo.declaration;
		solAssert(!!declaration, "");

		auto& declarationAnnotation = annotation(*declaration);
		solAssert(declarationAnnotation.type);
		m_typeSystem.unify(*declarationAnnotation.type, m_wordType);
		identifierInfo.valueSize = 1;
		return true;
	};
	solAssert(!_inlineAssembly.annotation().analysisInfo, "");
	_inlineAssembly.annotation().analysisInfo = make_shared<yul::AsmAnalysisInfo>();
	yul::AsmAnalyzer analyzer(
		*_inlineAssembly.annotation().analysisInfo,
		m_errorReporter,
		_inlineAssembly.dialect(),
		identifierAccess
	);
	if (!analyzer.analyze(_inlineAssembly.operations()))
		solAssert(m_errorReporter.hasErrors());
	return false;
}

bool TypeInference::visit(VariableDeclaration const& _variableDeclaration)
{
	solAssert(!_variableDeclaration.value());
	auto& variableAnnotation = annotation(_variableDeclaration);
	solAssert(!variableAnnotation.type);
	variableAnnotation.type = [&] {
		if (_variableDeclaration.hasTypeName())
			return fromTypeName(_variableDeclaration.typeName());
		else
			return m_typeSystem.freshTypeVariable();
	}();
	return false;
}

bool TypeInference::visit(Assignment const&)
{
	return true;
}

void TypeInference::endVisit(Assignment const& _assignment)
{
	auto& assignmentAnnotation = annotation(_assignment);
	solAssert(!assignmentAnnotation.type);

	auto& lhsAnnotation = annotation(_assignment.leftHandSide());
	solAssert(lhsAnnotation.type);
	auto& rhsAnnotation = annotation(_assignment.rightHandSide());
	solAssert(rhsAnnotation.type);
	m_typeSystem.unify(*lhsAnnotation.type, *rhsAnnotation.type);
	assignmentAnnotation.type = m_typeSystem.resolve(*lhsAnnotation.type);
}

TypeInference::TypeAnnotation& TypeInference::annotation(ASTNode const& _node)
{
	auto& annotation = m_typeAnnotations.at(static_cast<size_t>(_node.id()));
	if (!annotation)
		annotation = make_unique<TypeAnnotation>();
	return *annotation;
}

bool TypeInference::visit(Identifier const& _identifier)
{
	auto& identifierAnnotation = annotation(_identifier);
	solAssert(!identifierAnnotation.type);

	auto const* referencedDeclaration = _identifier.annotation().referencedDeclaration;
	solAssert(referencedDeclaration);

	auto& declarationAnnotation = annotation(*referencedDeclaration);
	if (!declarationAnnotation.type)
		referencedDeclaration->accept(*this);

	solAssert(declarationAnnotation.type);
	identifierAnnotation.type = declarationAnnotation.type;

	return true;
}

bool TypeInference::visit(FunctionCall const&) { return true; }
void TypeInference::endVisit(FunctionCall const& _functionCall)
{
	auto& functionCallAnnotation = annotation(_functionCall);
	solAssert(!functionCallAnnotation.type);

	auto& expressionAnnotation = annotation(_functionCall.expression());
	solAssert(expressionAnnotation.type);

	Type functionType = m_typeSystem.fresh(*expressionAnnotation.type);

	std::vector<Type> argTypes;
	for(auto arg: _functionCall.arguments())
	{
		auto& argAnnotation = annotation(*arg);
		solAssert(argAnnotation.type);
		argTypes.emplace_back(*argAnnotation.type);
	}
	Type argTuple = TypeSystemHelpers{m_typeSystem}.tupleType(argTypes);
	Type genericFunctionType = TypeSystemHelpers{m_typeSystem}.functionType(argTuple, m_typeSystem.freshTypeVariable());
	m_typeSystem.unify(genericFunctionType, functionType);

	functionCallAnnotation.type = m_typeSystem.resolve(std::get<1>(TypeSystemHelpers{m_typeSystem}.destFunctionType(m_typeSystem.resolve(genericFunctionType))));
}
