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
 * @author julius <djudju@protonmail.com>
 * @date 2017
 * Converts the AST from JSON sformat to ASTNode
 */

#pragma once

#include <vector>
#include <libsolidity/ast/AST.h>
#include <json/json.h>
#include <libsolidity/ast/ASTAnnotations.h>
#include <libevmasm/SourceLocation.h>

namespace dev
{
namespace solidity
{

/**
 * Converter of the AST from JSON sformat to ASTNode
 */
class ASTJsonImporter
{
public:
	/// Create an importer to import a given abstract syntax tree in Json format to an ASTNode
	/// @a _sourceList used to provide source names for the ASTs
	ASTJsonImporter(std::map<std::string, Json::Value const*> _sourceList);

	/// converts the
	std::map<std::string, ASTPointer<SourceUnit>> jsonToSourceUnit();

private:
	template <typename T, typename... Args>
	ASTPointer<T> createASTNode(Json::Value const& _node, Args&&... _args);
	SourceLocation const createSourceLocation(Json::Value const& _node);
	///function to be called when the type of the Json-node is unknown
	ASTPointer<ASTNode> convertJsonToASTNode(Json::Value const& _ast);

	//instantiate the AST-Nodes with the information from the Json-nodes
	ASTPointer<SourceUnit> createSourceUnit(Json::Value const& _node, std::string const& _srcName);
	ASTPointer<PragmaDirective> createPragmaDirective(Json::Value const& _node);
	ASTPointer<ImportDirective> createImportDirective(Json::Value const& _node);
	ASTPointer<ContractDefinition> createContractDefinition(Json::Value const& _node);
	ASTPointer<InheritanceSpecifier> createInheritanceSpecifier(Json::Value const& _node);
	ASTPointer<UsingForDirective> createUsingForDirective(Json::Value const& _node);
	ASTPointer<ASTNode> createStructDefinition(Json::Value const& _node);
	ASTPointer<EnumDefinition> createEnumDefinition(Json::Value const& _node);
	ASTPointer<EnumValue> createEnumValue(Json::Value const& _node);
	ASTPointer<ParameterList> createParameterList(Json::Value const& _node);
	ASTPointer<FunctionDefinition> createFunctionDefinition(Json::Value const& _node);
	ASTPointer<VariableDeclaration> createVariableDeclaration(Json::Value const& _node);
	ASTPointer<ModifierDefinition> createModifierDefinition(Json::Value const& _node);
	ASTPointer<ModifierInvocation> createModifierInvocation(Json::Value const& _node);
	ASTPointer<EventDefinition> createEventDefinition(Json::Value const& _node);
	ASTPointer<ElementaryTypeName> createElementaryTypeName(Json::Value const& _node);
	ASTPointer<UserDefinedTypeName> createUserDefinedTypeName(Json::Value const& _node);
	ASTPointer<FunctionTypeName> createFunctionTypeName(Json::Value const& _node);
	ASTPointer<Mapping> createMapping(Json::Value const& _node);
	ASTPointer<ArrayTypeName> createArrayTypeName(Json::Value const& _node);
	ASTPointer<InlineAssembly> createInlineAssembly(Json::Value const& _node);
	ASTPointer<Block> createBlock(Json::Value const& _node);
	ASTPointer<PlaceholderStatement> createPlaceholderStatement(Json::Value const& _node);
	ASTPointer<IfStatement> createIfStatement(Json::Value const& _node);
	ASTPointer<WhileStatement> createWhileStatement(Json::Value const& _node, bool _isDoWhile);
	ASTPointer<ForStatement> createForStatement(Json::Value const& _node);
	ASTPointer<Continue> createContinue(Json::Value const& _node);
	ASTPointer<Break> createBreak(Json::Value const& _node);
	ASTPointer<Return> createReturn(Json::Value const& _node);
	ASTPointer<Throw> createThrow(Json::Value const& _node);
	ASTPointer<VariableDeclarationStatement> createVariableDeclarationStatement(Json::Value const& _node);
	ASTPointer<ExpressionStatement> createExpressionStatement(Json::Value const& _node);
	ASTPointer<Conditional> createConditional(Json::Value const& _node);
	ASTPointer<Assignment> createAssignment(Json::Value const& _node);
	ASTPointer<TupleExpression> createTupleExpression(Json::Value const& _node);
	ASTPointer<UnaryOperation> createUnaryOperation(Json::Value const& _node);
	ASTPointer<BinaryOperation> createBinaryOperation(Json::Value const& _node);
	ASTPointer<FunctionCall> createFunctionCall(Json::Value const& _node);
	ASTPointer<NewExpression> createNewExpression(Json::Value const& _node);
	ASTPointer<MemberAccess> createMemberAccess(Json::Value const& _node);
	ASTPointer<IndexAccess> createIndexAccess(Json::Value const& _node);
	ASTPointer<Identifier> createIdentifier(Json::Value const& _node, std::string const& name);
	ASTPointer<ElementaryTypeNameExpression> createElementaryTypeNameExpression(Json::Value const& _node);
	ASTPointer<ASTNode> createLiteral(Json::Value const& _node);

	template<class T>
	ASTPointer<T> nullOrCast(Json::Value _json);
	Declaration::Visibility visibility(Json::Value const& _node);
	VariableDeclaration::Location location(Json::Value const& _node);
	ContractDefinition::ContractKind contractKind(Json::Value const& _node);
	Literal::SubDenomination subdenomination(Json::Value const& _node);
	Token::Value literalTokenKind(Json::Value const& _node);
	Token::Value scanSingleToken(Json::Value _node);
	std::map<std::string, Json::Value const*> m_sourceList;
	std::vector<std::shared_ptr<std::string const>> m_sourceLocations;
	std::map<std::string, ASTPointer<SourceUnit>> m_sourceUnits;


};

}
}

