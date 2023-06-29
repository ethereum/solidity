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
#pragma once
#include <libsolidity/experimental/ast/AST.h>
#include <libsolidity/ast/ASTVisitor.h>

namespace solidity::langutil
{
class ErrorReporter;
}

namespace solidity::frontend::experimental
{
namespace legacy = solidity::frontend;

class Analysis;

class ASTTransform: public ASTConstVisitor
{
public:
	ASTTransform(Analysis& _analysis);

	std::unique_ptr<AST> ast() { return std::move(m_ast); }
private:
	bool visit(legacy::SourceUnit const&) override { return true; }
	bool visit(legacy::PragmaDirective const&) override { return true; }
	bool visit(legacy::ImportDirective const&) override { return true; }

	bool visit(legacy::TypeDefinition const& _typeDefinition) override;
	bool visit(legacy::TypeClassDefinition const& _typeClassDefinition) override;
	bool visit(legacy::TypeClassInstantiation const& _typeClassInstantiation) override;
	bool visit(legacy::FunctionDefinition const& _functionDefinition) override;
	bool visit(legacy::ContractDefinition const& _contractDefinition) override;
	bool visitNode(ASTNode const& _node) override;

	std::unique_ptr<Term> term(legacy::TypeClassDefinition const& _typeClassDefinition);
	std::unique_ptr<Term> term(legacy::TypeClassInstantiation const& _typeClassInstantiation);
	std::unique_ptr<Term> term(legacy::TypeDefinition const& _typeDefinition);
	std::unique_ptr<Term> term(legacy::ContractDefinition const& _contractDefinition);
	std::unique_ptr<Term> term(legacy::FunctionDefinition const& _functionDefinition);
	std::unique_ptr<Term> term(legacy::VariableDeclarationStatement const& _declaration);
	std::unique_ptr<Term> term(legacy::Assignment const& _assignment);
	std::unique_ptr<Term> term(legacy::Block const& _block);
	std::unique_ptr<Term> term(legacy::Statement const& _statements);
	std::unique_ptr<Term> term(legacy::TypeName const& _name);
	std::unique_ptr<Term> term(legacy::TypeClassName const& _typeClassName);
	std::unique_ptr<Term> term(legacy::ParameterList const& _parameterList);
	std::unique_ptr<Term> term(legacy::VariableDeclaration const& _variableDeclaration, std::unique_ptr<Term> _initialValue = {});
	std::unique_ptr<Term> term(legacy::InlineAssembly const& _assembly);
	std::unique_ptr<Term> term(legacy::Expression const& _expression);

	// Allows for easy uniform treatment in the variadic templates below.
	std::unique_ptr<Term> term(std::unique_ptr<Term> _term) { return _term; }

	std::unique_ptr<Term> namedFunctionList(std::vector<ASTPointer<ASTNode>> _nodes);
	std::unique_ptr<Term> binaryOperation(
		langutil::Token _operator,
		std::unique_ptr<Term> _leftHandSide,
		std::unique_ptr<Term> _rightHandSide
	)
	{
		return application(builtinBinaryOperator(_operator), std::move(_leftHandSide), std::move(_rightHandSide));
	}
	std::unique_ptr<Term> reference(legacy::Declaration const& _declaration);
	std::unique_ptr<Term> constant(BuiltinConstant _constant) { return makeTerm<Constant>(_constant); }
	std::unique_ptr<Term> constant(std::string _name) { return makeTerm<Constant>(_name); }
	template<typename T>
	std::unique_ptr<Term> termOrConstant(T const* _node, BuiltinConstant _constant)
	{
		return _node ? term(*_node) : constant(_constant);
	}


	std::unique_ptr<Term> pair(std::unique_ptr<Term> _first, std::unique_ptr<Term> _second)
	{
		// Note: BuiltinConstant::Pair has signature a -> (b -> (a, b))
		// This reduces n-ary functions to unary functions only as primitives.
		return application(
			application(
				BuiltinConstant::Pair,
				std::move(_first)
			),
			std::move(_second)
		);
	}
	std::unique_ptr<Term> tuple(std::list<std::unique_ptr<Term>> _components);
	std::unique_ptr<Term> constrain(std::unique_ptr<Term> _value, std::unique_ptr<Term> _constraint);
	std::unique_ptr<Term> builtinBinaryOperator(langutil::Token);
	std::unique_ptr<Term> builtinTypeClass(langutil::Token);

	template<typename... Args>
	std::unique_ptr<Term> tuple(Args&&... _args)
	{
		std::list<std::unique_ptr<Term>> components;
		(components.emplace_back(term(std::forward<Args>(_args))), ...);
		return tuple(std::move(components));
	}
	std::unique_ptr<Term> application(std::unique_ptr<Term> _function, std::list<std::unique_ptr<Term>> _arguments)
	{
		return makeTerm<Application>(std::move(_function), tuple(std::move(_arguments)));
	}
	template<typename... Args>
	std::unique_ptr<Term> application(std::unique_ptr<Term> _function, Args&&... _args)
	{
		std::list<std::unique_ptr<Term>> components;
		(components.emplace_back(term(std::forward<Args>(_args))), ...);
		return application(std::move(_function), std::move(components));
	}
	template<typename... Args>
	std::unique_ptr<Term> application(BuiltinConstant _function, Args&&... _args)
	{
		std::list<std::unique_ptr<Term>> components;
		(components.emplace_back(term(std::forward<Args>(_args))), ...);
		return application(constant(_function), std::move(components));
	}

	TermBase makeTermBase();
	template<typename TermKind, typename... Args>
	std::unique_ptr<Term> makeTerm(Args&&... _args)
	{
		return std::make_unique<Term>(TermKind{
			makeTermBase(),
			std::forward<Args>(_args)...
		});
	}

	Analysis& m_analysis;
	langutil::ErrorReporter& m_errorReporter;

	std::unique_ptr<AST> m_ast;

	struct SetNode {
		SetNode(ASTTransform& _parent, ASTNode const& _node):
		m_parent(_parent),
		m_previousNode(_parent.m_currentNode),
		m_previousLocation(_parent.m_currentLocation)
		{
			_parent.m_currentNode = &_node;
			_parent.m_currentLocation = _node.location();
		}
		SetNode(ASTTransform& _parent, langutil::SourceLocation const& _location):
		m_parent(_parent),
		m_previousNode(_parent.m_currentNode),
		m_previousLocation(_parent.m_currentLocation)
		{
			_parent.m_currentNode = nullptr;
			_parent.m_currentLocation = _location;
		}
		~SetNode() {
			m_parent.m_currentNode = m_previousNode;
			m_parent.m_currentLocation = m_previousLocation;
		}
	private:
		ASTTransform& m_parent;
		ASTNode const* m_previousNode = nullptr;
		langutil::SourceLocation m_previousLocation;
	};
	ASTNode const* m_currentNode = nullptr;
	langutil::SourceLocation m_currentLocation;
};

}
