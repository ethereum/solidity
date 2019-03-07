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
	return dynamic_pointer_cast<SourceUnit>(m_substitutions.at(&_ast));
}

void ASTCopier::substitute(ASTNode const& _node, ASTPointer<ASTNode> _new)
{
	solAssert(!m_substitutions.count(&_node), "");
	m_substitutions[&_node] = _new;
}

bool ASTCopier::visit(SourceUnit const& /*_node*/)
{
	return true;
}

bool ASTCopier::visit(PragmaDirective const& /*_node*/)
{
	return true;
}

bool ASTCopier::visit(ImportDirective const& /*_node*/)
{
	return true;
}

bool ASTCopier::visit(ContractDefinition const& /*_node*/)
{
	return true;
}

bool ASTCopier::visit(InheritanceSpecifier const& /*_node*/)
{
	return true;
}

bool ASTCopier::visit(UsingForDirective const& /*_node*/)
{
	return true;
}

bool ASTCopier::visit(StructDefinition const& /*_node*/)
{
	return true;
}

bool ASTCopier::visit(EnumDefinition const& /*_node*/)
{
	return true;
}

bool ASTCopier::visit(EnumValue const& /*_node*/)
{
	return true;
}

bool ASTCopier::visit(ParameterList const& /*_node*/)
{
	return true;
}

bool ASTCopier::visit(FunctionDefinition const& /*_node*/)
{
	return true;
}

bool ASTCopier::visit(VariableDeclaration const& /*_node*/)
{
	return true;
}

bool ASTCopier::visit(ModifierDefinition const& /*_node*/)
{
	return true;
}

bool ASTCopier::visit(ModifierInvocation const& /*_node*/)
{
	return true;
}

bool ASTCopier::visit(EventDefinition const& /*_node*/)
{
	return true;
}

bool ASTCopier::visit(ElementaryTypeName const& /*_node*/)
{
	return true;
}

bool ASTCopier::visit(UserDefinedTypeName const& /*_node*/)
{
	return true;
}

bool ASTCopier::visit(FunctionTypeName const& /*_node*/)
{
	return true;
}

bool ASTCopier::visit(Mapping const& /*_node*/)
{
	return true;
}

bool ASTCopier::visit(ArrayTypeName const& /*_node*/)
{
	return true;
}

bool ASTCopier::visit(InlineAssembly const& /*_node*/)
{
	return true;
}

bool ASTCopier::visit(Block const& /*_node*/)
{
	return true;
}

bool ASTCopier::visit(PlaceholderStatement const& /*_node*/)
{
	return true;
}

bool ASTCopier::visit(IfStatement const& /*_node*/)
{
	return true;
}

bool ASTCopier::visit(WhileStatement const& /*_node*/)
{
	return true;
}

bool ASTCopier::visit(ForStatement const& /*_node*/)
{
	return true;
}

bool ASTCopier::visit(Continue const& /*_node*/)
{
	return true;
}

bool ASTCopier::visit(Break const& /*_node*/)
{
	return true;
}

bool ASTCopier::visit(Return const& /*_node*/)
{
	return true;
}

bool ASTCopier::visit(Throw const& /*_node*/)
{
	return true;
}

bool ASTCopier::visit(EmitStatement const& /*_node*/)
{
	return true;
}

bool ASTCopier::visit(VariableDeclarationStatement const& /*_node*/)
{
	return true;
}

bool ASTCopier::visit(ExpressionStatement const& /*_node*/)
{
	return true;
}

bool ASTCopier::visit(Conditional const& /*_node*/)
{
	return true;
}

bool ASTCopier::visit(Assignment const& /*_node*/)
{
	return true;
}

bool ASTCopier::visit(TupleExpression const& /*_node*/)
{
	return true;
}

bool ASTCopier::visit(UnaryOperation const& /*_node*/)
{
	return true;
}

bool ASTCopier::visit(BinaryOperation const& /*_node*/)
{
	return true;
}

bool ASTCopier::visit(FunctionCall const& /*_node*/)
{
	return true;
}

bool ASTCopier::visit(NewExpression const& /*_node*/)
{
	return true;
}

bool ASTCopier::visit(MemberAccess const& /*_node*/)
{
	return true;
}

bool ASTCopier::visit(IndexAccess const& /*_node*/)
{
	return true;
}

bool ASTCopier::visit(Identifier const& /*_node*/)
{
	return true;
}

bool ASTCopier::visit(ElementaryTypeNameExpression const& /*_node*/)
{
	return true;
}

bool ASTCopier::visit(Literal const& /*_node*/)
{
	return true;
}

void ASTCopier::endVisit(SourceUnit const& _sourceUnit)
{
	vector<ASTPointer<ASTNode>> nodes;
	for (auto const& node: _sourceUnit.nodes())
	{
		solAssert(m_substitutions.count(node.get()), "");
		nodes.push_back(m_substitutions.at(node.get()));
	}
	substitute(_sourceUnit, make_shared<SourceUnit>(
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
	substitute(_pragma, make_shared<PragmaDirective>(
		_pragma.location(),
		tokens,
		literals
	));
}

void ASTCopier::endVisit(ImportDirective const&)
{
}

void ASTCopier::endVisit(ContractDefinition const& _contract)
{
	vector<ASTPointer<InheritanceSpecifier>> inheritance;
	vector<ASTPointer<ASTNode>> subNodes;
	auto documentation = _contract.documentation() ?
		ASTString(*_contract.documentation()) :
		ASTString{}
	;
	substitute(_contract, make_shared<ContractDefinition>(
		_contract.location(),
		make_shared<ASTString>(_contract.name()),
		make_shared<ASTString>(documentation),
		inheritance,
		subNodes,
		_contract.contractKind()
	));
}

void ASTCopier::endVisit(InheritanceSpecifier const&)
{
}

void ASTCopier::endVisit(UsingForDirective const&)
{
}

void ASTCopier::endVisit(StructDefinition const&)
{
}

void ASTCopier::endVisit(EnumDefinition const&)
{
}

void ASTCopier::endVisit(EnumValue const&)
{
}

void ASTCopier::endVisit(ParameterList const&)
{
}

void ASTCopier::endVisit(FunctionDefinition const&)
{
}

void ASTCopier::endVisit(VariableDeclaration const&)
{
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

void ASTCopier::endVisit(ElementaryTypeName const&)
{
}

void ASTCopier::endVisit(UserDefinedTypeName const&)
{
}

void ASTCopier::endVisit(FunctionTypeName const&)
{
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

void ASTCopier::endVisit(Block const&)
{
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

void ASTCopier::endVisit(VariableDeclarationStatement const&)
{
}

void ASTCopier::endVisit(ExpressionStatement const&)
{
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

void ASTCopier::endVisit(FunctionCall const&)
{
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

void ASTCopier::endVisit(Identifier const&)
{
}

void ASTCopier::endVisit(ElementaryTypeNameExpression const&)
{
}

void ASTCopier::endVisit(Literal const&)
{
}

}
}
