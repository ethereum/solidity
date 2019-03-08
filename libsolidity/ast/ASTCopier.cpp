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

#include <libsolidity/ast/ASTCopier.h>

#include <libsolidity/ast/AST.h>

using namespace std;
using namespace langutil;

namespace dev
{
namespace solidity
{

ASTPointer<SourceUnit> ASTCopier::copy(ASTNode const& _ast)
{
	m_substitutions.clear();
	_ast.accept(*this);
	solAssert(m_substitutions.count(&_ast), "");
	return dynamic_pointer_cast<SourceUnit>(substitute(&_ast));
}

void ASTCopier::applySubstitution(ASTNode const& _node, ASTPointer<ASTNode> _new)
{
	solAssert(!m_substitutions.count(&_node), "");
	m_substitutions[&_node] = _new;
}

ASTPointer<ASTNode> ASTCopier::substitute(ASTNode const* _node)
{
	if (!_node)
		return nullptr;
	solAssert(m_substitutions.count(_node), "");
	return m_substitutions[_node];
}

ASTPointer<ASTString> ASTCopier::documentation(Documented const* _node)
{
	auto documentation = _node->documentation() ?
		ASTString(*_node->documentation()) :
		ASTString{}
	;
	return make_shared<ASTString>(documentation);
}

void ASTCopier::endVisit(SourceUnit const& _sourceUnit)
{
	vector<ASTPointer<ASTNode>> nodes;
	for (auto const& node: _sourceUnit.nodes())
		nodes.push_back(substitute(node.get()));
	applySubstitution(_sourceUnit, make_shared<SourceUnit>(
		_sourceUnit.location(),
		nodes
	));
}

void ASTCopier::endVisit(PragmaDirective const& _pragma)
{
	vector<Token> tokens;
	for (auto const& token: _pragma.tokens())
		tokens.emplace_back(token);
	vector<ASTString> literals;
	for (auto const& literal: _pragma.literals())
		literals.emplace_back(literal);
	applySubstitution(_pragma, make_shared<PragmaDirective>(
		_pragma.location(),
		tokens,
		literals
	));
}

void ASTCopier::endVisit(ImportDirective const& _import)
{
	vector<pair<ASTPointer<Identifier>, ASTPointer<ASTString>>> symbolAliases;
	for (auto const& alias: _import.symbolAliases())
	{
		ASTString newAlias = alias.second ? *alias.second : ASTString{};
		symbolAliases.emplace_back(
			dynamic_pointer_cast<Identifier>(substitute(alias.first.get())),
			make_shared<ASTString>(newAlias)
		);
	}
	applySubstitution(_import, make_shared<ImportDirective>(
		_import.location(),
		make_shared<ASTString>(_import.path()),
		make_shared<ASTString>(_import.name()),
		std::move(symbolAliases)
	));
}

void ASTCopier::endVisit(ContractDefinition const& _contract)
{
	vector<ASTPointer<InheritanceSpecifier>> inheritance;
	for (auto const& base: _contract.baseContracts())
		inheritance.push_back(dynamic_pointer_cast<InheritanceSpecifier>(substitute(base.get())));
	vector<ASTPointer<ASTNode>> subNodes;
	for (auto const& subNode: _contract.subNodes())
		subNodes.push_back(substitute(subNode.get()));
	applySubstitution(_contract, make_shared<ContractDefinition>(
		_contract.location(),
		make_shared<ASTString>(_contract.name()),
		documentation(&_contract),
		inheritance,
		subNodes,
		_contract.contractKind()
	));
}

void ASTCopier::endVisit(InheritanceSpecifier const& _inheritance)
{
	vector<ASTPointer<Expression>> arguments;
	for (auto const& arg: *_inheritance.arguments())
		arguments.push_back(dynamic_pointer_cast<Expression>(substitute(arg.get())));
	applySubstitution(_inheritance, make_shared<InheritanceSpecifier>(
		_inheritance.location(),
		dynamic_pointer_cast<UserDefinedTypeName>(substitute(&_inheritance.name())),
		make_unique<decltype(arguments)>(arguments)
	));
}

void ASTCopier::endVisit(UsingForDirective const& _using)
{
	applySubstitution(_using, make_shared<UsingForDirective>(
		_using.location(),
		dynamic_pointer_cast<UserDefinedTypeName>(substitute(&_using.libraryName())),
		dynamic_pointer_cast<TypeName>(substitute(_using.typeName()))
	));
}

void ASTCopier::endVisit(StructDefinition const& _struct)
{
	vector<ASTPointer<VariableDeclaration>> members;
	for (auto const& member: _struct.members())
		members.push_back(dynamic_pointer_cast<VariableDeclaration>(substitute(member.get())));
	applySubstitution(_struct, make_shared<StructDefinition>(
		_struct.location(),
		make_shared<ASTString>(_struct.name()),
		members
	));
}

void ASTCopier::endVisit(EnumDefinition const& _enum)
{
	vector<ASTPointer<EnumValue>> members;
	for (auto const& member: _enum.members())
		members.push_back(dynamic_pointer_cast<EnumValue>(substitute(member.get())));
	applySubstitution(_enum, make_shared<EnumDefinition>(
		_enum.location(),
		make_shared<ASTString>(_enum.name()),
		members
	));
}

void ASTCopier::endVisit(EnumValue const& _enum)
{
	applySubstitution(_enum, make_shared<EnumValue>(
		_enum.location(),
		make_shared<ASTString>(_enum.name())
	));
}

void ASTCopier::endVisit(ParameterList const& _list)
{
	vector<ASTPointer<VariableDeclaration>> parameters;
	for (auto const& param: _list.parameters())
		parameters.push_back(dynamic_pointer_cast<VariableDeclaration>(substitute(param.get())));
	applySubstitution(_list, make_shared<ParameterList>(
		_list.location(),
		parameters
	));
}

void ASTCopier::endVisit(FunctionDefinition const& _function)
{
	vector<ASTPointer<ModifierInvocation>> modifiers;
	for (auto const& modifier: _function.modifiers())
		modifiers.push_back(dynamic_pointer_cast<ModifierInvocation>(substitute(modifier.get())));
	applySubstitution(_function, make_shared<FunctionDefinition>(
		_function.location(),
		make_shared<ASTString>(_function.name()),
		_function.visibility(),
		_function.stateMutability(),
		_function.isConstructor(),
		documentation(&_function),
		dynamic_pointer_cast<ParameterList>(substitute(&_function.parameterList())),
		modifiers,
		dynamic_pointer_cast<ParameterList>(substitute(_function.returnParameterList().get())),
		dynamic_pointer_cast<Block>(substitute(&_function.body()))
	));
}

void ASTCopier::endVisit(VariableDeclaration const& _var)
{
	applySubstitution(_var, make_shared<VariableDeclaration>(
		_var.location(),
		dynamic_pointer_cast<TypeName>(substitute(_var.typeName())),
		make_shared<ASTString>(_var.name()),
		dynamic_pointer_cast<Expression>(substitute(_var.value().get())),
		_var.visibility(),
		_var.isStateVariable(),
		_var.isIndexed(),
		_var.isConstant(),
		_var.referenceLocation()
	));
}

void ASTCopier::endVisit(ModifierDefinition const&)
{
}

void ASTCopier::endVisit(ModifierInvocation const&)
{
}

void ASTCopier::endVisit(EventDefinition const&)
{
}

void ASTCopier::endVisit(ElementaryTypeName const& _typeName)
{
	applySubstitution(_typeName, make_shared<ElementaryTypeName>(
		_typeName.location(),
		_typeName.typeName(),
		_typeName.stateMutability()
	));
}

void ASTCopier::endVisit(UserDefinedTypeName const& _typeName)
{
	vector<ASTString> namePath;
	for (auto const& name: _typeName.namePath())
		namePath.push_back(name);
	applySubstitution(_typeName, make_shared<UserDefinedTypeName>(
		_typeName.location(),
		namePath
	));
}

void ASTCopier::endVisit(FunctionTypeName const& _typeName)
{
	applySubstitution(_typeName, make_shared<FunctionTypeName>(
		_typeName.location(),
		dynamic_pointer_cast<ParameterList>(substitute(_typeName.parameterTypeList().get())),
		dynamic_pointer_cast<ParameterList>(substitute(_typeName.returnParameterTypeList().get())),
		_typeName.visibility(),
		_typeName.stateMutability()
	));
}

void ASTCopier::endVisit(Mapping const&)
{
}

void ASTCopier::endVisit(ArrayTypeName const&)
{
}

void ASTCopier::endVisit(InlineAssembly const&)
{
}

void ASTCopier::endVisit(Block const& _block)
{
	vector<ASTPointer<Statement>> statements;
	for (auto const& statement: _block.statements())
		statements.push_back(dynamic_pointer_cast<Statement>(substitute(statement.get())));
	applySubstitution(_block, make_shared<Block>(
		_block.location(),
		documentation(&_block),
		statements
	));
}

void ASTCopier::endVisit(PlaceholderStatement const&)
{
}

void ASTCopier::endVisit(IfStatement const&)
{
}

void ASTCopier::endVisit(WhileStatement const&)
{
}

void ASTCopier::endVisit(ForStatement const&)
{
}

void ASTCopier::endVisit(Continue const&)
{
}

void ASTCopier::endVisit(Break const&)
{
}

void ASTCopier::endVisit(Return const&)
{
}

void ASTCopier::endVisit(Throw const&)
{
}

void ASTCopier::endVisit(EmitStatement const&)
{
}

void ASTCopier::endVisit(VariableDeclarationStatement const& _var)
{
	vector<ASTPointer<VariableDeclaration>> variables;
	for (auto const& var: _var.declarations())
		variables.push_back(dynamic_pointer_cast<VariableDeclaration>(substitute(var.get())));
	applySubstitution(_var, make_shared<VariableDeclarationStatement>(
		_var.location(),
		documentation(&_var),
		variables,
		dynamic_pointer_cast<Expression>(substitute(_var.initialValue()))
	));
}

void ASTCopier::endVisit(ExpressionStatement const& _expr)
{
	applySubstitution(_expr, make_shared<ExpressionStatement>(
		_expr.location(),
		documentation(&_expr),
		dynamic_pointer_cast<Expression>(substitute(&_expr.expression()))
	));
}

void ASTCopier::endVisit(Conditional const&)
{
}

void ASTCopier::endVisit(Assignment const&)
{
}

void ASTCopier::endVisit(TupleExpression const&)
{
}

void ASTCopier::endVisit(UnaryOperation const&)
{
}

void ASTCopier::endVisit(BinaryOperation const&)
{
}

void ASTCopier::endVisit(FunctionCall const& _function)
{
	vector<ASTPointer<Expression>> arguments;
	for (auto const& arg: _function.arguments())
		arguments.push_back(dynamic_pointer_cast<Expression>(substitute(arg.get())));
	vector<ASTPointer<ASTString>> names;
	for (auto const& name: _function.names())
		names.push_back(make_shared<ASTString>(*name));
	applySubstitution(_function, make_shared<FunctionCall>(
		_function.location(),
		dynamic_pointer_cast<Expression>(substitute(&_function.expression())),
		arguments,
		names
	));
}

void ASTCopier::endVisit(NewExpression const&)
{
}

void ASTCopier::endVisit(MemberAccess const&)
{
}

void ASTCopier::endVisit(IndexAccess const&)
{
}

void ASTCopier::endVisit(Identifier const& _identifier)
{
	applySubstitution(_identifier, make_shared<Identifier>(
		_identifier.location(),
		make_shared<ASTString>(_identifier.name())
	));
}

void ASTCopier::endVisit(ElementaryTypeNameExpression const&)
{
}

void ASTCopier::endVisit(Literal const&)
{
}

}
}
