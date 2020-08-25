
// Generated from Solidity.g4 by ANTLR 4.8

#pragma once


#include "antlr4-runtime.h"
#include <test/tools/ossfuzz/antlr4-runtime/SolidityParser.h>



/**
 * This class defines an abstract visitor for a parse tree
 * produced by SolidityParser.
 */
class  SolidityVisitor : public antlr4::tree::AbstractParseTreeVisitor {
public:

  /**
   * Visit parse trees produced by SolidityParser.
   */
    virtual antlrcpp::Any visitSourceUnit(SolidityParser::SourceUnitContext *context) = 0;

    virtual antlrcpp::Any visitPragmaDirective(SolidityParser::PragmaDirectiveContext *context) = 0;

    virtual antlrcpp::Any visitImportDirective(SolidityParser::ImportDirectiveContext *context) = 0;

    virtual antlrcpp::Any visitImportAliases(SolidityParser::ImportAliasesContext *context) = 0;

    virtual antlrcpp::Any visitPath(SolidityParser::PathContext *context) = 0;

    virtual antlrcpp::Any visitSymbolAliases(SolidityParser::SymbolAliasesContext *context) = 0;

    virtual antlrcpp::Any visitContractDefinition(SolidityParser::ContractDefinitionContext *context) = 0;

    virtual antlrcpp::Any visitInterfaceDefinition(SolidityParser::InterfaceDefinitionContext *context) = 0;

    virtual antlrcpp::Any visitLibraryDefinition(SolidityParser::LibraryDefinitionContext *context) = 0;

    virtual antlrcpp::Any visitInheritanceSpecifierList(SolidityParser::InheritanceSpecifierListContext *context) = 0;

    virtual antlrcpp::Any visitInheritanceSpecifier(SolidityParser::InheritanceSpecifierContext *context) = 0;

    virtual antlrcpp::Any visitContractBodyElement(SolidityParser::ContractBodyElementContext *context) = 0;

    virtual antlrcpp::Any visitNamedArgument(SolidityParser::NamedArgumentContext *context) = 0;

    virtual antlrcpp::Any visitCallArgumentList(SolidityParser::CallArgumentListContext *context) = 0;

    virtual antlrcpp::Any visitUserDefinedTypeName(SolidityParser::UserDefinedTypeNameContext *context) = 0;

    virtual antlrcpp::Any visitModifierInvocation(SolidityParser::ModifierInvocationContext *context) = 0;

    virtual antlrcpp::Any visitVisibility(SolidityParser::VisibilityContext *context) = 0;

    virtual antlrcpp::Any visitParameterList(SolidityParser::ParameterListContext *context) = 0;

    virtual antlrcpp::Any visitParameterDeclaration(SolidityParser::ParameterDeclarationContext *context) = 0;

    virtual antlrcpp::Any visitConstructorDefinition(SolidityParser::ConstructorDefinitionContext *context) = 0;

    virtual antlrcpp::Any visitStateMutability(SolidityParser::StateMutabilityContext *context) = 0;

    virtual antlrcpp::Any visitOverrideSpecifier(SolidityParser::OverrideSpecifierContext *context) = 0;

    virtual antlrcpp::Any visitFunctionDefinition(SolidityParser::FunctionDefinitionContext *context) = 0;

    virtual antlrcpp::Any visitModifierDefinition(SolidityParser::ModifierDefinitionContext *context) = 0;

    virtual antlrcpp::Any visitFallbackReceiveFunctionDefinition(SolidityParser::FallbackReceiveFunctionDefinitionContext *context) = 0;

    virtual antlrcpp::Any visitStructDefinition(SolidityParser::StructDefinitionContext *context) = 0;

    virtual antlrcpp::Any visitStructMember(SolidityParser::StructMemberContext *context) = 0;

    virtual antlrcpp::Any visitEnumDefinition(SolidityParser::EnumDefinitionContext *context) = 0;

    virtual antlrcpp::Any visitStateVariableDeclaration(SolidityParser::StateVariableDeclarationContext *context) = 0;

    virtual antlrcpp::Any visitEventParameter(SolidityParser::EventParameterContext *context) = 0;

    virtual antlrcpp::Any visitEventDefinition(SolidityParser::EventDefinitionContext *context) = 0;

    virtual antlrcpp::Any visitUsingDirective(SolidityParser::UsingDirectiveContext *context) = 0;

    virtual antlrcpp::Any visitTypeName(SolidityParser::TypeNameContext *context) = 0;

    virtual antlrcpp::Any visitElementaryTypeName(SolidityParser::ElementaryTypeNameContext *context) = 0;

    virtual antlrcpp::Any visitFunctionTypeName(SolidityParser::FunctionTypeNameContext *context) = 0;

    virtual antlrcpp::Any visitVariableDeclaration(SolidityParser::VariableDeclarationContext *context) = 0;

    virtual antlrcpp::Any visitDataLocation(SolidityParser::DataLocationContext *context) = 0;

    virtual antlrcpp::Any visitUnaryPrefixOperation(SolidityParser::UnaryPrefixOperationContext *context) = 0;

    virtual antlrcpp::Any visitPrimaryExpression(SolidityParser::PrimaryExpressionContext *context) = 0;

    virtual antlrcpp::Any visitOrderComparison(SolidityParser::OrderComparisonContext *context) = 0;

    virtual antlrcpp::Any visitConditional(SolidityParser::ConditionalContext *context) = 0;

    virtual antlrcpp::Any visitPayableConversion(SolidityParser::PayableConversionContext *context) = 0;

    virtual antlrcpp::Any visitAssignment(SolidityParser::AssignmentContext *context) = 0;

    virtual antlrcpp::Any visitUnarySuffixOperation(SolidityParser::UnarySuffixOperationContext *context) = 0;

    virtual antlrcpp::Any visitShiftOperation(SolidityParser::ShiftOperationContext *context) = 0;

    virtual antlrcpp::Any visitBitAndOperation(SolidityParser::BitAndOperationContext *context) = 0;

    virtual antlrcpp::Any visitFunctionCall(SolidityParser::FunctionCallContext *context) = 0;

    virtual antlrcpp::Any visitIndexRangeAccess(SolidityParser::IndexRangeAccessContext *context) = 0;

    virtual antlrcpp::Any visitNewExpression(SolidityParser::NewExpressionContext *context) = 0;

    virtual antlrcpp::Any visitIndexAccess(SolidityParser::IndexAccessContext *context) = 0;

    virtual antlrcpp::Any visitAddSubOperation(SolidityParser::AddSubOperationContext *context) = 0;

    virtual antlrcpp::Any visitBitOrOperation(SolidityParser::BitOrOperationContext *context) = 0;

    virtual antlrcpp::Any visitExpOperation(SolidityParser::ExpOperationContext *context) = 0;

    virtual antlrcpp::Any visitAndOperation(SolidityParser::AndOperationContext *context) = 0;

    virtual antlrcpp::Any visitInlineArray(SolidityParser::InlineArrayContext *context) = 0;

    virtual antlrcpp::Any visitOrOperation(SolidityParser::OrOperationContext *context) = 0;

    virtual antlrcpp::Any visitMemberAccess(SolidityParser::MemberAccessContext *context) = 0;

    virtual antlrcpp::Any visitMulDivModOperation(SolidityParser::MulDivModOperationContext *context) = 0;

    virtual antlrcpp::Any visitFunctionCallOptions(SolidityParser::FunctionCallOptionsContext *context) = 0;

    virtual antlrcpp::Any visitBitXorOperation(SolidityParser::BitXorOperationContext *context) = 0;

    virtual antlrcpp::Any visitTuple(SolidityParser::TupleContext *context) = 0;

    virtual antlrcpp::Any visitEqualityComparison(SolidityParser::EqualityComparisonContext *context) = 0;

    virtual antlrcpp::Any visitMetaType(SolidityParser::MetaTypeContext *context) = 0;

    virtual antlrcpp::Any visitAssignOp(SolidityParser::AssignOpContext *context) = 0;

    virtual antlrcpp::Any visitTupleExpression(SolidityParser::TupleExpressionContext *context) = 0;

    virtual antlrcpp::Any visitInlineArrayExpression(SolidityParser::InlineArrayExpressionContext *context) = 0;

    virtual antlrcpp::Any visitIdentifier(SolidityParser::IdentifierContext *context) = 0;

    virtual antlrcpp::Any visitLiteral(SolidityParser::LiteralContext *context) = 0;

    virtual antlrcpp::Any visitBooleanLiteral(SolidityParser::boolLiteralContext *context) = 0;

    virtual antlrcpp::Any visitStringLiteral(SolidityParser::StringLiteralContext *context) = 0;

    virtual antlrcpp::Any visitHexStringLiteral(SolidityParser::HexStringLiteralContext *context) = 0;

    virtual antlrcpp::Any visitUnicodeStringLiteral(SolidityParser::UnicodeStringLiteralContext *context) = 0;

    virtual antlrcpp::Any visitNumberLiteral(SolidityParser::NumberLiteralContext *context) = 0;

    virtual antlrcpp::Any visitBlock(SolidityParser::BlockContext *context) = 0;

    virtual antlrcpp::Any visitStatement(SolidityParser::StatementContext *context) = 0;

    virtual antlrcpp::Any visitSimpleStatement(SolidityParser::SimpleStatementContext *context) = 0;

    virtual antlrcpp::Any visitIfStatement(SolidityParser::IfStatementContext *context) = 0;

    virtual antlrcpp::Any visitForStatement(SolidityParser::ForStatementContext *context) = 0;

    virtual antlrcpp::Any visitWhileStatement(SolidityParser::WhileStatementContext *context) = 0;

    virtual antlrcpp::Any visitDoWhileStatement(SolidityParser::DoWhileStatementContext *context) = 0;

    virtual antlrcpp::Any visitContinueStatement(SolidityParser::ContinueStatementContext *context) = 0;

    virtual antlrcpp::Any visitBreakStatement(SolidityParser::BreakStatementContext *context) = 0;

    virtual antlrcpp::Any visitTryStatement(SolidityParser::TryStatementContext *context) = 0;

    virtual antlrcpp::Any visitCatchClause(SolidityParser::CatchClauseContext *context) = 0;

    virtual antlrcpp::Any visitReturnStatement(SolidityParser::ReturnStatementContext *context) = 0;

    virtual antlrcpp::Any visitEmitStatement(SolidityParser::EmitStatementContext *context) = 0;

    virtual antlrcpp::Any visitAssemblyStatement(SolidityParser::AssemblyStatementContext *context) = 0;

    virtual antlrcpp::Any visitVariableDeclarationList(SolidityParser::VariableDeclarationListContext *context) = 0;

    virtual antlrcpp::Any visitVariableDeclarationTuple(SolidityParser::VariableDeclarationTupleContext *context) = 0;

    virtual antlrcpp::Any visitVariableDeclarationStatement(SolidityParser::VariableDeclarationStatementContext *context) = 0;

    virtual antlrcpp::Any visitExpressionStatement(SolidityParser::ExpressionStatementContext *context) = 0;

    virtual antlrcpp::Any visitMappingType(SolidityParser::MappingTypeContext *context) = 0;

    virtual antlrcpp::Any visitMappingKeyType(SolidityParser::MappingKeyTypeContext *context) = 0;

    virtual antlrcpp::Any visitYulStatement(SolidityParser::YulStatementContext *context) = 0;

    virtual antlrcpp::Any visitYulBlock(SolidityParser::YulBlockContext *context) = 0;

    virtual antlrcpp::Any visitYulVariableDeclaration(SolidityParser::YulVariableDeclarationContext *context) = 0;

    virtual antlrcpp::Any visitYulAssignment(SolidityParser::YulAssignmentContext *context) = 0;

    virtual antlrcpp::Any visitYulIfStatement(SolidityParser::YulIfStatementContext *context) = 0;

    virtual antlrcpp::Any visitYulForStatement(SolidityParser::YulForStatementContext *context) = 0;

    virtual antlrcpp::Any visitYulSwitchCase(SolidityParser::YulSwitchCaseContext *context) = 0;

    virtual antlrcpp::Any visitYulSwitchStatement(SolidityParser::YulSwitchStatementContext *context) = 0;

    virtual antlrcpp::Any visitYulFunctionDefinition(SolidityParser::YulFunctionDefinitionContext *context) = 0;

    virtual antlrcpp::Any visitYulPath(SolidityParser::YulPathContext *context) = 0;

    virtual antlrcpp::Any visitYulFunctionCall(SolidityParser::YulFunctionCallContext *context) = 0;

    virtual antlrcpp::Any visitYulBoolean(SolidityParser::YulboolContext *context) = 0;

    virtual antlrcpp::Any visitYulLiteral(SolidityParser::YulLiteralContext *context) = 0;

    virtual antlrcpp::Any visitYulExpression(SolidityParser::YulExpressionContext *context) = 0;


};

