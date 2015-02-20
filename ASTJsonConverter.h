/*
    This file is part of cpp-ethereum.

    cpp-ethereum is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    cpp-ethereum is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with cpp-ethereum.  If not, see <http://www.gnu.org/licenses/>.
*/
/**
 * @author Lefteris <lefteris@ethdev.com>
 * @date 2015
 * Converts the AST into json format
 */

#pragma once

#include <ostream>
#include <stack>
#include <libsolidity/ASTVisitor.h>
#include <libsolidity/Exceptions.h>
#include <libsolidity/Utils.h>
#include <json/json.h>

namespace dev
{
namespace solidity
{

/**
 * Converter of the AST into JSON format
 */
class ASTJsonConverter: public ASTConstVisitor
{
public:
	/// Create a converter to JSON for the given abstract syntax tree.
	ASTJsonConverter(ASTNode const& _ast);
	/// Output the json representation of the AST to _stream.
	void print(std::ostream& _stream);

	bool visit(ImportDirective const& _node) override;
	bool visit(ContractDefinition const& _node) override;
	bool visit(StructDefinition const& _node) override;
	bool visit(ParameterList const& _node) override;
	bool visit(FunctionDefinition const& _node) override;
	bool visit(VariableDeclaration const& _node) override;
	bool visit(TypeName const& _node) override;
	bool visit(ElementaryTypeName const& _node) override;
	bool visit(UserDefinedTypeName const& _node) override;
	bool visit(Mapping const& _node) override;
	bool visit(Statement const& _node) override;
	bool visit(Block const& _node) override;
	bool visit(IfStatement const& _node) override;
	bool visit(BreakableStatement const& _node) override;
	bool visit(WhileStatement const& _node) override;
	bool visit(ForStatement const& _node) override;
	bool visit(Continue const& _node) override;
	bool visit(Break const& _node) override;
	bool visit(Return const& _node) override;
	bool visit(VariableDefinition const& _node) override;
	bool visit(ExpressionStatement const& _node) override;
	bool visit(Expression const& _node) override;
	bool visit(Assignment const& _node) override;
	bool visit(UnaryOperation const& _node) override;
	bool visit(BinaryOperation const& _node) override;
	bool visit(FunctionCall const& _node) override;
	bool visit(NewExpression const& _node) override;
	bool visit(MemberAccess const& _node) override;
	bool visit(IndexAccess const& _node) override;
	bool visit(PrimaryExpression const& _node) override;
	bool visit(Identifier const& _node) override;
	bool visit(ElementaryTypeNameExpression const& _node) override;
	bool visit(Literal const& _node) override;

	void endVisit(ImportDirective const&) override;
	void endVisit(ContractDefinition const&) override;
	void endVisit(StructDefinition const&) override;
	void endVisit(ParameterList const&) override;
	void endVisit(FunctionDefinition const&) override;
	void endVisit(VariableDeclaration const&) override;
	void endVisit(TypeName const&) override;
	void endVisit(ElementaryTypeName const&) override;
	void endVisit(UserDefinedTypeName const&) override;
	void endVisit(Mapping const&) override;
	void endVisit(Statement const&) override;
	void endVisit(Block const&) override;
	void endVisit(IfStatement const&) override;
	void endVisit(BreakableStatement const&) override;
	void endVisit(WhileStatement const&) override;
	void endVisit(ForStatement const&) override;
	void endVisit(Continue const&) override;
	void endVisit(Break const&) override;
	void endVisit(Return const&) override;
	void endVisit(VariableDefinition const&) override;
	void endVisit(ExpressionStatement const&) override;
	void endVisit(Expression const&) override;
	void endVisit(Assignment const&) override;
	void endVisit(UnaryOperation const&) override;
	void endVisit(BinaryOperation const&) override;
	void endVisit(FunctionCall const&) override;
	void endVisit(NewExpression const&) override;
	void endVisit(MemberAccess const&) override;
	void endVisit(IndexAccess const&) override;
	void endVisit(PrimaryExpression const&) override;
	void endVisit(Identifier const&) override;
	void endVisit(ElementaryTypeNameExpression const&) override;
	void endVisit(Literal const&) override;

private:
	void addKeyValue(Json::Value& _obj, std::string const& _key, std::string const& _val);
	void addJsonNode(std::string const& _nodeName,
					 std::initializer_list<std::pair<std::string const, std::string const>> _list,
					 bool _hasChildren);
	std::string getType(Expression const& _expression);
	inline void goUp()
	{
		solAssert(!m_jsonNodePtrs.empty(), "Uneven json nodes stack. Internal error.");
		m_jsonNodePtrs.pop();
	};

	Json::Value m_astJson;
	std::stack<Json::Value*> m_jsonNodePtrs;
	std::string m_source;
	ASTNode const* m_ast;
};

}
}
