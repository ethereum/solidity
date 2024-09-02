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
/**
 * @author Lefteris <lefteris@ethdev.com>
 * @date 2015
 * Converts the AST into json format
 */

#pragma once

#include <libsolidity/ast/ASTAnnotations.h>
#include <libsolidity/ast/ASTVisitor.h>
#include <libsolidity/interface/CompilerStack.h>
#include <liblangutil/Exceptions.h>

#include <libsolutil/JSON.h>

#include <algorithm>
#include <optional>
#include <ostream>
#include <stack>
#include <vector>

namespace solidity::langutil
{
struct SourceLocation;
}

namespace solidity::frontend
{

/**
 * Converter of the AST into JSON format
 */
class ASTCoqExporter: public ASTConstVisitor
{
public:
	/// Create a converter to JSON for the given abstract syntax tree.
	/// @a _stackState state of the compiler stack to avoid outputting incomplete data
	/// @a _sourceIndices is used to abbreviate source names in source locations.
	explicit ASTCoqExporter(
		CompilerStack::State _stackState,
		std::map<std::string, unsigned> _sourceIndices = std::map<std::string, unsigned>()
	);
	/// Output the string representation of the AST to _stream.
	void print(std::ostream& _stream, ASTNode const& _node);
	std::string toCoq(ASTNode const& _node);
	template <class T>
	std::string toCoqs(std::string const& separator, std::vector<ASTPointer<T>> const& _nodes)
	{
		std::string ret = "";
		bool isFirst = true;

		for (auto const& node: _nodes)
			if (node) {
				if (!isFirst)
					ret += separator;
				else
					isFirst = false;

				ret += toCoq(*node);
			}

		return ret;
	}
	std::string indent();
	std::string paren(std::string const& content);
	bool visit(SourceUnit const& _node) override;
	bool visit(PragmaDirective const& _node) override;
	bool visit(ImportDirective const& _node) override;
	bool visit(ContractDefinition const& _node) override;
	bool visit(IdentifierPath const& _node) override;
	bool visit(InheritanceSpecifier const& _node) override;
	bool visit(UsingForDirective const& _node) override;
	bool visit(StructDefinition const& _node) override;
	bool visit(EnumDefinition const& _node) override;
	bool visit(EnumValue const& _node) override;
	bool visit(UserDefinedValueTypeDefinition const& _node) override;
	bool visit(ParameterList const& _node) override;
	bool visit(OverrideSpecifier const& _node) override;
	bool visit(FunctionDefinition const& _node) override;
	bool visit(VariableDeclaration const& _node) override;
	bool visit(ModifierDefinition const& _node) override;
	bool visit(ModifierInvocation const& _node) override;
	bool visit(EventDefinition const& _node) override;
	bool visit(ErrorDefinition const& _node) override;
	bool visit(ElementaryTypeName const& _node) override;
	bool visit(UserDefinedTypeName const& _node) override;
	bool visit(FunctionTypeName const& _node) override;
	bool visit(Mapping const& _node) override;
	bool visit(ArrayTypeName const& _node) override;
	bool visit(InlineAssembly const& _node) override;
	bool visit(Block const& _node) override;
	bool visit(PlaceholderStatement const& _node) override;
	bool visit(IfStatement const& _node) override;
	bool visit(TryCatchClause const& _node) override;
	bool visit(TryStatement const& _node) override;
	bool visit(WhileStatement const& _node) override;
	bool visit(ForStatement const& _node) override;
	bool visit(Continue const& _node) override;
	bool visit(Break const& _node) override;
	bool visit(Return const& _node) override;
	bool visit(Throw const& _node) override;
	bool visit(EmitStatement const& _node) override;
	bool visit(RevertStatement const& _node) override;
	bool visit(VariableDeclarationStatement const& _node) override;
	bool visit(ExpressionStatement const& _node) override;
	bool visit(Conditional const& _node) override;
	bool visit(Assignment const& _node) override;
	bool visit(TupleExpression const& _node) override;
	bool visit(UnaryOperation const& _node) override;
	bool visit(BinaryOperation const& _node) override;
	bool visit(FunctionCall const& _node) override;
	bool visit(FunctionCallOptions const& _node) override;
	bool visit(NewExpression const& _node) override;
	bool visit(MemberAccess const& _node) override;
	bool visit(IndexAccess const& _node) override;
	bool visit(IndexRangeAccess const& _node) override;
	bool visit(Identifier const& _node) override;
	bool visit(ElementaryTypeNameExpression const& _node) override;
	bool visit(Literal const& _node) override;
	bool visit(StructuredDocumentation const& _node) override;

	void endVisit(EventDefinition const&) override;

	bool visitNode(ASTNode const& _node) override;
private:
	void setCoqNode(
		ASTNode const& _node,
		std::string const& _nodeName,
		std::initializer_list<std::pair<std::string, std::string>>&& _attributes
	);
	void setCoqNode(
		ASTNode const& _node,
		std::string const& _nodeName,
		std::vector<std::pair<std::string, std::string>>&& _attributes
	);
	/// Maps source location to an index, if source is valid and a mapping does exist, otherwise returns std::nullopt.
	std::optional<size_t> sourceIndexFromLocation(langutil::SourceLocation const& _location) const;
	std::string sourceLocationToString(langutil::SourceLocation const& _location) const;
	std::string sourceLocationsToCoq(std::vector<langutil::SourceLocation> const& _sourceLocations) const;
	static std::string namePathToString(std::vector<ASTString> const& _namePath);
	static std::string idOrNull(ASTNode const* _pt)
	{
		return _pt ? std::to_string(nodeId(*_pt)) : "null";
	}
	std::string toCoqOrNull(ASTNode const* _node)
	{
		return _node ? toCoq(*_node) : "";
	}
	std::string toCoqOrUnit(ASTNode const* _node)
	{
		return _node ? toCoq(*_node) : paren("Value.Tuple []");
	}
	std::string toCoqOption(ASTNode const* _node)
	{
		if (_node) {
			m_withParens.push(true);
			std::string content = toCoq(*_node);
			m_withParens.pop();

			return paren("Some " + content);
		}

		return "None";
	}
	std::string inlineAssemblyIdentifierToCoq(std::pair<yul::Identifier const* , InlineAssemblyAnnotation::ExternalIdentifierInfo> _info) const;
	static std::string location(VariableDeclaration::Location _location);
	static std::string contractKind(ContractKind _kind);
	static std::string functionCallKind(FunctionCallKind _kind);
	static std::string literalTokenKind(Token _token);
	static std::string type(Expression const& _expression);
	static std::string type(VariableDeclaration const& _varDecl);
	static int64_t nodeId(ASTNode const& _node)
	{
		return _node.id();
	}
	template<class Container>
	static std::string getContainerIds(Container const& _container, bool _order = false)
	{
		std::vector<int64_t> tmp;

		for (auto const& element: _container)
		{
			solAssert(element, "");
			tmp.push_back(nodeId(*element));
		}
		if (_order)
			std::sort(tmp.begin(), tmp.end());

		std::string output = "[";
		for (int64_t val: tmp)
			output += val + ",";
		return output + "]";
	}
	static std::string typePointerToCoq(Type const* _tp, bool _withoutDataLocation = false);
	static std::string typePointerToCoq(std::optional<FuncCallArguments> const& _tps);
	void appendExpressionAttributes(
		std::vector<std::pair<std::string, std::string>> &_attributes,
		ExpressionAnnotation const& _annotation
	);
	static void appendMove(Json& _array, Json&& _value)
	{
		solAssert(_array.is_array(), "");
		_array.emplace_back(std::move(_value));
	}

	CompilerStack::State m_stackState = CompilerStack::State::Empty; ///< Used to only access information that already exists
	bool m_inEvent = false; ///< whether we are currently inside an event or not
	std::string m_currentValue;
	std::map<std::string, unsigned> m_sourceIndices;
	
	std::stack<bool> m_withParens = std::stack<bool>({false}); /// < whether to print the expression with parens or not
	uint64_t m_indent = 0; ///< current indentation level
};

}
