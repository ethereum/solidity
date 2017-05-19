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
 * @author Lefteris <lefteris@ethdev.com>
 * @date 2015
 * Converts the AST into json format
 */

#pragma once

#include <ostream>
#include <stack>
#include <libsolidity/ast/ASTVisitor.h>
#include <libsolidity/interface/Exceptions.h>
#include <libsolidity/interface/Utils.h>
#include <libsolidity/ast/ASTAnnotations.h>
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
	/// @a _legacy if true, use legacy format
	/// @a _sourceIndices is used to abbreviate source names in source locations.
	explicit ASTJsonConverter(
		bool _legacy,
		std::map<std::string, unsigned> _sourceIndices = std::map<std::string, unsigned>()
	);
	/// Output the json representation of the AST to _stream.
	void print(std::ostream& _stream, ASTNode const& _node);
	Json::Value toJson(ASTNode const& _node);
	template <class T>
	Json::Value toJson(std::vector<ASTPointer<T>> const& _nodes)
	{
		Json::Value ret(Json::arrayValue);
		for (auto const& n: _nodes)
			ret.append(n ? toJson(*n) : Json::nullValue);
		return ret;
	}
	bool visit(SourceUnit const& _node) override;
	bool visit(PragmaDirective const& _node) override;
	bool visit(ImportDirective const& _node) override;
	bool visit(ContractDefinition const& _node) override;
	bool visit(InheritanceSpecifier const& _node) override;
	bool visit(UsingForDirective const& _node) override;
	bool visit(StructDefinition const& _node) override;
	bool visit(EnumDefinition const& _node) override;
	bool visit(EnumValue const& _node) override;
	bool visit(ParameterList const& _node) override;
	bool visit(FunctionDefinition const& _node) override;
	bool visit(VariableDeclaration const& _node) override;
	bool visit(ModifierDefinition const& _node) override;
	bool visit(ModifierInvocation const& _node) override;
	bool visit(EventDefinition const& _node) override;
	bool visit(TypeName const& _node) override;
	bool visit(ElementaryTypeName const& _node) override;
	bool visit(UserDefinedTypeName const& _node) override;
	bool visit(FunctionTypeName const& _node) override;
	bool visit(Mapping const& _node) override;
	bool visit(ArrayTypeName const& _node) override;
	bool visit(InlineAssembly const& _node) override;
	bool visit(Block const& _node) override;
	bool visit(PlaceholderStatement const& _node) override;
	bool visit(IfStatement const& _node) override;
	bool visit(WhileStatement const& _node) override;
	bool visit(ForStatement const& _node) override;
	bool visit(Continue const& _node) override;
	bool visit(Break const& _node) override;
	bool visit(Return const& _node) override;
	bool visit(Throw const& _node) override;
	bool visit(VariableDeclarationStatement const& _node) override;
	bool visit(ExpressionStatement const& _node) override;
	bool visit(Conditional const& _node) override;
	bool visit(Assignment const& _node) override;
	bool visit(TupleExpression const& _node) override;
	bool visit(UnaryOperation const& _node) override;
	bool visit(BinaryOperation const& _node) override;
	bool visit(FunctionCall const& _node) override;
	bool visit(NewExpression const& _node) override;
	bool visit(MemberAccess const& _node) override;
	bool visit(IndexAccess const& _node) override;
	bool visit(Identifier const& _node) override;
	bool visit(ElementaryTypeNameExpression const& _node) override;
	bool visit(Literal const& _node) override;

	void endVisit(EventDefinition const&) override;

private:
	void setJsonNode(
		ASTNode const& _node,
		std::string const& _nodeName,
		std::initializer_list<std::pair<std::string, Json::Value>>&& _attributes
	);
	void setJsonNode(
		ASTNode const& _node,
		std::string const& _nodeName,
		std::vector<std::pair<std::string, Json::Value>>&& _attributes
	);
	std::string sourceLocationToString(SourceLocation const& _location) const;
	std::string namePathToString(std::vector<ASTString> const& _namePath) const;
	Json::Value idOrNull(ASTNode const* _pt)
	{
		return _pt ? Json::Value(nodeId(*_pt)) : Json::nullValue;
	}
	Json::Value toJsonOrNull(ASTNode const* _node)
	{
		return _node ? toJson(*_node) : Json::nullValue;
	}
	Json::Value inlineAssemblyIdentifierToJson(std::pair<assembly::Identifier const* , InlineAssemblyAnnotation::ExternalIdentifierInfo> _info);
	std::string visibility(Declaration::Visibility const& _visibility);
	std::string location(VariableDeclaration::Location _location);
	std::string contractKind(ContractDefinition::ContractKind _kind);
	std::string functionCallKind(FunctionCallKind _kind);
	std::string type(Expression const& _expression);
	std::string type(VariableDeclaration const& _varDecl);
	int nodeId(ASTNode const& _node)
	{
		return _node.id();
	}
	template<class Container>
	Json::Value getContainerIds(Container const& container)
	{
		Json::Value tmp(Json::arrayValue);
		for (auto const& element: container)
		{
			solAssert(element, "");
			tmp.append(nodeId(*element));
		}
		return tmp;
	}
	Json::Value typePointerToJson(TypePointer _tp);
	Json::Value typePointerToJson(std::shared_ptr<std::vector<TypePointer>> _tps);
	void appendExpressionAttributes(
		std::vector<std::pair<std::string, Json::Value>> &_attributes,
		ExpressionAnnotation const& _annotation
	);
	bool m_legacy = false; ///< if true, use legacy format
	bool m_inEvent = false; ///< whether we are currently inside an event or not
	Json::Value m_currentValue;
	std::map<std::string, unsigned> m_sourceIndices;
};

}
}
