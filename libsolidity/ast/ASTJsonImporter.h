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
 * @author julius <djudju@protonmail.com>
 * @date 2019
 * Converts the AST from JSON format to ASTNode
 */

#pragma once

#include <vector>
#include <libsolidity/ast/AST.h>
#include <json/json.h>
#include <libsolidity/ast/ASTAnnotations.h>
#include <liblangutil/EVMVersion.h>
#include <liblangutil/Exceptions.h>
#include <liblangutil/SourceLocation.h>

namespace solidity::frontend
{

/**
 * Component that imports an AST from json format to the internal format
 */
class ASTJsonImporter
{
public:
	ASTJsonImporter(langutil::EVMVersion _evmVersion)
		:m_evmVersion(_evmVersion)
	{}

	/// Converts the AST from JSON-format to ASTPointer
	/// @a _sourceList used to provide source names for the ASTs
	/// @returns map of sourcenames to their respective ASTs
	std::map<std::string, ASTPointer<SourceUnit>> jsonToSourceUnit(std::map<std::string, Json::Value> const& _sourceList);

private:

	// =========== general creation functions ==============

	/// Sets the source location and nodeID
	/// @returns the ASTNode Object class of the respective JSON node,
	template <typename T, typename... Args>
	ASTPointer<T> createASTNode(Json::Value const& _node, Args&&... _args);
	/// @returns the sourceLocation-object created from the string in the JSON node
	langutil::SourceLocation const createSourceLocation(Json::Value const& _node);
	std::optional<std::vector<langutil::SourceLocation>> createSourceLocations(Json::Value const& _node) const;
	/// Creates an ASTNode for a given JSON-ast of unknown type
	/// @returns Pointer to a new created ASTNode
	ASTPointer<ASTNode> convertJsonToASTNode(Json::Value const& _ast);
	/// @returns a pointer to the more specific subclass of ASTNode
	/// as indicated by the nodeType field of the json
	template<class T>
	ASTPointer<T> convertJsonToASTNode(Json::Value const& _node);

	langutil::SourceLocation createNameSourceLocation(Json::Value const& _node);

	/// \defgroup nodeCreators JSON to AST-Nodes
	///@{
	ASTPointer<SourceUnit> createSourceUnit(Json::Value const& _node, std::string const& _srcName);
	ASTPointer<PragmaDirective> createPragmaDirective(Json::Value const& _node);
	ASTPointer<ImportDirective> createImportDirective(Json::Value const& _node);
	ASTPointer<ContractDefinition> createContractDefinition(Json::Value const& _node);
	ASTPointer<IdentifierPath> createIdentifierPath(Json::Value const& _node);
	ASTPointer<InheritanceSpecifier> createInheritanceSpecifier(Json::Value const& _node);
	ASTPointer<UsingForDirective> createUsingForDirective(Json::Value const& _node);
	ASTPointer<ASTNode> createStructDefinition(Json::Value const& _node);
	ASTPointer<EnumDefinition> createEnumDefinition(Json::Value const& _node);
	ASTPointer<EnumValue> createEnumValue(Json::Value const& _node);
	ASTPointer<UserDefinedValueTypeDefinition> createUserDefinedValueTypeDefinition(Json::Value const& _node);
	ASTPointer<ParameterList> createParameterList(Json::Value const& _node);
	ASTPointer<OverrideSpecifier> createOverrideSpecifier(Json::Value const& _node);
	ASTPointer<FunctionDefinition> createFunctionDefinition(Json::Value const& _node);
	ASTPointer<VariableDeclaration> createVariableDeclaration(Json::Value const& _node);
	ASTPointer<ModifierDefinition> createModifierDefinition(Json::Value const& _node);
	ASTPointer<ModifierInvocation> createModifierInvocation(Json::Value const& _node);
	ASTPointer<EventDefinition> createEventDefinition(Json::Value const& _node);
	ASTPointer<ErrorDefinition> createErrorDefinition(Json::Value const& _node);
	ASTPointer<ElementaryTypeName> createElementaryTypeName(Json::Value const& _node);
	ASTPointer<UserDefinedTypeName> createUserDefinedTypeName(Json::Value const& _node);
	ASTPointer<FunctionTypeName> createFunctionTypeName(Json::Value const& _node);
	ASTPointer<Mapping> createMapping(Json::Value const& _node);
	ASTPointer<ArrayTypeName> createArrayTypeName(Json::Value const& _node);
	ASTPointer<InlineAssembly> createInlineAssembly(Json::Value const& _node);
	ASTPointer<Block> createBlock(Json::Value const& _node, bool _unchecked);
	ASTPointer<PlaceholderStatement> createPlaceholderStatement(Json::Value const& _node);
	ASTPointer<IfStatement> createIfStatement(Json::Value const& _node);
	ASTPointer<TryCatchClause> createTryCatchClause(Json::Value const& _node);
	ASTPointer<TryStatement> createTryStatement(Json::Value const& _node);
	ASTPointer<WhileStatement> createWhileStatement(Json::Value const& _node, bool _isDoWhile);
	ASTPointer<ForStatement> createForStatement(Json::Value const& _node);
	ASTPointer<Continue> createContinue(Json::Value const& _node);
	ASTPointer<Break> createBreak(Json::Value const& _node);
	ASTPointer<Return> createReturn(Json::Value const& _node);
	ASTPointer<Throw> createThrow(Json::Value const& _node);
	ASTPointer<EmitStatement> createEmitStatement(Json::Value const& _node);
	ASTPointer<RevertStatement> createRevertStatement(Json::Value const& _node);
	ASTPointer<VariableDeclarationStatement> createVariableDeclarationStatement(Json::Value const& _node);
	ASTPointer<ExpressionStatement> createExpressionStatement(Json::Value const& _node);
	ASTPointer<Conditional> createConditional(Json::Value const& _node);
	ASTPointer<Assignment> createAssignment(Json::Value const& _node);
	ASTPointer<TupleExpression> createTupleExpression(Json::Value const& _node);
	ASTPointer<UnaryOperation> createUnaryOperation(Json::Value const& _node);
	ASTPointer<BinaryOperation> createBinaryOperation(Json::Value const& _node);
	ASTPointer<FunctionCall> createFunctionCall(Json::Value const& _node);
	ASTPointer<FunctionCallOptions> createFunctionCallOptions(Json::Value const& _node);
	ASTPointer<NewExpression> createNewExpression(Json::Value const& _node);
	ASTPointer<MemberAccess> createMemberAccess(Json::Value const& _node);
	ASTPointer<IndexAccess> createIndexAccess(Json::Value const& _node);
	ASTPointer<IndexRangeAccess> createIndexRangeAccess(Json::Value const& _node);
	ASTPointer<Identifier> createIdentifier(Json::Value const& _node);
	ASTPointer<ElementaryTypeNameExpression> createElementaryTypeNameExpression(Json::Value const& _node);
	ASTPointer<ASTNode> createLiteral(Json::Value const& _node);
	ASTPointer<StructuredDocumentation> createDocumentation(Json::Value const& _node);
	///@}

	// =============== general helper functions ===================
	/// @returns the member of a given JSON object, throws if member does not exist
	Json::Value member(Json::Value const& _node, std::string const& _name);
	/// @returns the appropriate TokenObject used in parsed Strings (pragma directive or operator)
	Token scanSingleToken(Json::Value const& _node);
	template<class T>
	///@returns nullptr or an ASTPointer cast to a specific Class
	ASTPointer<T> nullOrCast(Json::Value const& _json);
	/// @returns nullptr or ASTString, given an JSON string or an empty field
	ASTPointer<ASTString> nullOrASTString(Json::Value const& _json, std::string const& _name);

	// ============== JSON to definition helpers ===============
	/// \defgroup typeHelpers Json to ast-datatype helpers
	/// {@
	ASTPointer<ASTString> memberAsASTString(Json::Value const& _node, std::string const& _name);
	bool memberAsBool(Json::Value const& _node, std::string const& _name);
	Visibility visibility(Json::Value const& _node);
	StateMutability stateMutability(Json::Value const& _node);
	VariableDeclaration::Location location(Json::Value const& _node);
	ContractKind contractKind(Json::Value const& _node);
	Token literalTokenKind(Json::Value const& _node);
	Literal::SubDenomination subdenomination(Json::Value const& _node);
	///@}

	// =========== member variables ===============
	/// list of source names, order by source index
	std::vector<std::shared_ptr<std::string const>> m_sourceNames;
	/// filepath to AST
	std::map<std::string, ASTPointer<SourceUnit>> m_sourceUnits;
	/// IDs already used by the nodes
	std::set<int64_t> m_usedIDs;
	/// Configured EVM version
	langutil::EVMVersion m_evmVersion;
};

}
