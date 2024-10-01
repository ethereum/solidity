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
#include <libsolutil/JSON.h>
#include <libsolidity/ast/AST.h>
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
	ASTJsonImporter(langutil::EVMVersion _evmVersion, std::optional<uint8_t> _eofVersion)
		:m_evmVersion(_evmVersion), m_eofVersion(_eofVersion)
	{}

	/// Converts the AST from JSON-format to ASTPointer
	/// @a _sourceList used to provide source names for the ASTs
	/// @returns map of sourcenames to their respective ASTs
	std::map<std::string, ASTPointer<SourceUnit>> jsonToSourceUnit(std::map<std::string, Json> const& _sourceList);

private:

	// =========== general creation functions ==============

	/// Sets the source location and nodeID
	/// @returns the ASTNode Object class of the respective JSON node,
	template <typename T, typename... Args>
	ASTPointer<T> createASTNode(Json const& _node, Args&&... _args);
	/// @returns the sourceLocation-object created from the string in the JSON node
	langutil::SourceLocation const createSourceLocation(Json const& _node);
	std::optional<std::vector<langutil::SourceLocation>> createSourceLocations(Json const& _node) const;
	/// Creates an ASTNode for a given JSON-ast of unknown type
	/// @returns Pointer to a new created ASTNode
	ASTPointer<ASTNode> convertJsonToASTNode(Json const& _ast);
	/// @returns a pointer to the more specific subclass of ASTNode
	/// as indicated by the nodeType field of the json
	template<class T>
	ASTPointer<T> convertJsonToASTNode(Json const& _node);

	langutil::SourceLocation createNameSourceLocation(Json const& _node);
	/// @returns source location of a mapping key name
	langutil::SourceLocation createKeyNameSourceLocation(Json const& _node);
	/// @returns source location of a mapping value name
	langutil::SourceLocation createValueNameSourceLocation(Json const& _node);

	/// \defgroup nodeCreators JSON to AST-Nodes
	///@{
	ASTPointer<SourceUnit> createSourceUnit(Json const& _node, std::string const& _srcName);
	ASTPointer<PragmaDirective> createPragmaDirective(Json const& _node);
	ASTPointer<ImportDirective> createImportDirective(Json const& _node);
	ASTPointer<ContractDefinition> createContractDefinition(Json const& _node);
	ASTPointer<IdentifierPath> createIdentifierPath(Json const& _node);
	ASTPointer<InheritanceSpecifier> createInheritanceSpecifier(Json const& _node);
	ASTPointer<UsingForDirective> createUsingForDirective(Json const& _node);
	ASTPointer<ASTNode> createStructDefinition(Json const& _node);
	ASTPointer<EnumDefinition> createEnumDefinition(Json const& _node);
	ASTPointer<EnumValue> createEnumValue(Json const& _node);
	ASTPointer<UserDefinedValueTypeDefinition> createUserDefinedValueTypeDefinition(Json const& _node);
	ASTPointer<ParameterList> createParameterList(Json const& _node);
	ASTPointer<OverrideSpecifier> createOverrideSpecifier(Json const& _node);
	ASTPointer<FunctionDefinition> createFunctionDefinition(Json const& _node);
	ASTPointer<VariableDeclaration> createVariableDeclaration(Json const& _node);
	ASTPointer<ModifierDefinition> createModifierDefinition(Json const& _node);
	ASTPointer<ModifierInvocation> createModifierInvocation(Json const& _node);
	ASTPointer<EventDefinition> createEventDefinition(Json const& _node);
	ASTPointer<ErrorDefinition> createErrorDefinition(Json const& _node);
	ASTPointer<ElementaryTypeName> createElementaryTypeName(Json const& _node);
	ASTPointer<UserDefinedTypeName> createUserDefinedTypeName(Json const& _node);
	ASTPointer<FunctionTypeName> createFunctionTypeName(Json const& _node);
	ASTPointer<Mapping> createMapping(Json const& _node);
	ASTPointer<ArrayTypeName> createArrayTypeName(Json const& _node);
	ASTPointer<InlineAssembly> createInlineAssembly(Json const& _node);
	ASTPointer<Block> createBlock(Json const& _node, bool _unchecked);
	ASTPointer<PlaceholderStatement> createPlaceholderStatement(Json const& _node);
	ASTPointer<IfStatement> createIfStatement(Json const& _node);
	ASTPointer<TryCatchClause> createTryCatchClause(Json const& _node);
	ASTPointer<TryStatement> createTryStatement(Json const& _node);
	ASTPointer<WhileStatement> createWhileStatement(Json const& _node, bool _isDoWhile);
	ASTPointer<ForStatement> createForStatement(Json const& _node);
	ASTPointer<Continue> createContinue(Json const& _node);
	ASTPointer<Break> createBreak(Json const& _node);
	ASTPointer<Return> createReturn(Json const& _node);
	ASTPointer<Throw> createThrow(Json const& _node);
	ASTPointer<EmitStatement> createEmitStatement(Json const& _node);
	ASTPointer<RevertStatement> createRevertStatement(Json const& _node);
	ASTPointer<VariableDeclarationStatement> createVariableDeclarationStatement(Json const& _node);
	ASTPointer<ExpressionStatement> createExpressionStatement(Json const& _node);
	ASTPointer<Conditional> createConditional(Json const& _node);
	ASTPointer<Assignment> createAssignment(Json const& _node);
	ASTPointer<TupleExpression> createTupleExpression(Json const& _node);
	ASTPointer<UnaryOperation> createUnaryOperation(Json const& _node);
	ASTPointer<BinaryOperation> createBinaryOperation(Json const& _node);
	ASTPointer<FunctionCall> createFunctionCall(Json const& _node);
	ASTPointer<FunctionCallOptions> createFunctionCallOptions(Json const& _node);
	ASTPointer<NewExpression> createNewExpression(Json const& _node);
	ASTPointer<MemberAccess> createMemberAccess(Json const& _node);
	ASTPointer<IndexAccess> createIndexAccess(Json const& _node);
	ASTPointer<IndexRangeAccess> createIndexRangeAccess(Json const& _node);
	ASTPointer<Identifier> createIdentifier(Json const& _node);
	ASTPointer<ElementaryTypeNameExpression> createElementaryTypeNameExpression(Json const& _node);
	ASTPointer<ASTNode> createLiteral(Json const& _node);
	ASTPointer<StructuredDocumentation> createDocumentation(Json const& _node);
	///@}

	// =============== general helper functions ===================
	/// @returns the member of a given JSON object, throws if member does not exist
	Json member(Json const& _node, std::string const& _name);
	/// @returns the appropriate TokenObject used in parsed Strings (pragma directive or operator)
	Token scanSingleToken(Json const& _node);
	template<class T>
	///@returns nullptr or an ASTPointer cast to a specific Class
	ASTPointer<T> nullOrCast(Json const& _json);
	/// @returns nullptr or ASTString, given an JSON string or an empty field
	ASTPointer<ASTString> nullOrASTString(Json const& _json, std::string const& _name);

	// ============== JSON to definition helpers ===============
	/// \defgroup typeHelpers Json to ast-datatype helpers
	/// {@
	ASTPointer<ASTString> memberAsASTString(Json const& _node, std::string const& _name);
	bool memberAsBool(Json const& _node, std::string const& _name);
	Visibility visibility(Json const& _node);
	StateMutability stateMutability(Json const& _node);
	VariableDeclaration::Location location(Json const& _node);
	ContractKind contractKind(Json const& _node);
	Token literalTokenKind(Json const& _node);
	Literal::SubDenomination subdenomination(Json const& _node);
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
	/// Configured EOF version. Equals std::nullopt if non-EOF
	std::optional<uint8_t> m_eofVersion;
};

}
