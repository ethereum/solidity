
// Generated from Solidity.g4 by ANTLR 4.8

#pragma once


#include "antlr4-runtime.h"
#include <test/tools/ossfuzz/antlr4-runtime/SolidityVisitor.h>

/**
 * This class provides an empty implementation of SolidityVisitor, which can be
 * extended to create a visitor which only needs to handle a subset of the available methods.
 */
class  SolidityBaseVisitor : public SolidityVisitor {
public:

  virtual antlrcpp::Any visitSourceUnit(SolidityParser::SourceUnitContext *ctx) override {
    return visitChildren(ctx);
  }

  virtual antlrcpp::Any visitPragmaDirective(SolidityParser::PragmaDirectiveContext *ctx) override {
    return visitChildren(ctx);
  }

  virtual antlrcpp::Any visitImportDirective(SolidityParser::ImportDirectiveContext *ctx) override {
    return visitChildren(ctx);
  }

  virtual antlrcpp::Any visitImportAliases(SolidityParser::ImportAliasesContext *ctx) override {
    return visitChildren(ctx);
  }

  virtual antlrcpp::Any visitPath(SolidityParser::PathContext *ctx) override {
    return visitChildren(ctx);
  }

  virtual antlrcpp::Any visitSymbolAliases(SolidityParser::SymbolAliasesContext *ctx) override {
    return visitChildren(ctx);
  }

  virtual antlrcpp::Any visitContractDefinition(SolidityParser::ContractDefinitionContext *ctx) override {
    return visitChildren(ctx);
  }

  virtual antlrcpp::Any visitInterfaceDefinition(SolidityParser::InterfaceDefinitionContext *ctx) override {
    return visitChildren(ctx);
  }

  virtual antlrcpp::Any visitLibraryDefinition(SolidityParser::LibraryDefinitionContext *ctx) override {
    return visitChildren(ctx);
  }

  virtual antlrcpp::Any visitInheritanceSpecifierList(SolidityParser::InheritanceSpecifierListContext *ctx) override {
    return visitChildren(ctx);
  }

  virtual antlrcpp::Any visitInheritanceSpecifier(SolidityParser::InheritanceSpecifierContext *ctx) override {
    return visitChildren(ctx);
  }

  virtual antlrcpp::Any visitContractBodyElement(SolidityParser::ContractBodyElementContext *ctx) override {
    return visitChildren(ctx);
  }

  virtual antlrcpp::Any visitNamedArgument(SolidityParser::NamedArgumentContext *ctx) override {
    return visitChildren(ctx);
  }

  virtual antlrcpp::Any visitCallArgumentList(SolidityParser::CallArgumentListContext *ctx) override {
    return visitChildren(ctx);
  }

  virtual antlrcpp::Any visitUserDefinedTypeName(SolidityParser::UserDefinedTypeNameContext *ctx) override {
    return visitChildren(ctx);
  }

  virtual antlrcpp::Any visitModifierInvocation(SolidityParser::ModifierInvocationContext *ctx) override {
    return visitChildren(ctx);
  }

  virtual antlrcpp::Any visitVisibility(SolidityParser::VisibilityContext *ctx) override {
    return visitChildren(ctx);
  }

  virtual antlrcpp::Any visitParameterList(SolidityParser::ParameterListContext *ctx) override {
    return visitChildren(ctx);
  }

  virtual antlrcpp::Any visitParameterDeclaration(SolidityParser::ParameterDeclarationContext *ctx) override {
    return visitChildren(ctx);
  }

  virtual antlrcpp::Any visitConstructorDefinition(SolidityParser::ConstructorDefinitionContext *ctx) override {
    return visitChildren(ctx);
  }

  virtual antlrcpp::Any visitStateMutability(SolidityParser::StateMutabilityContext *ctx) override {
    return visitChildren(ctx);
  }

  virtual antlrcpp::Any visitOverrideSpecifier(SolidityParser::OverrideSpecifierContext *ctx) override {
    return visitChildren(ctx);
  }

  virtual antlrcpp::Any visitFunctionDefinition(SolidityParser::FunctionDefinitionContext *ctx) override {
    return visitChildren(ctx);
  }

  virtual antlrcpp::Any visitModifierDefinition(SolidityParser::ModifierDefinitionContext *ctx) override {
    return visitChildren(ctx);
  }

  virtual antlrcpp::Any visitFallbackReceiveFunctionDefinition(SolidityParser::FallbackReceiveFunctionDefinitionContext *ctx) override {
    return visitChildren(ctx);
  }

  virtual antlrcpp::Any visitStructDefinition(SolidityParser::StructDefinitionContext *ctx) override {
    return visitChildren(ctx);
  }

  virtual antlrcpp::Any visitStructMember(SolidityParser::StructMemberContext *ctx) override {
    return visitChildren(ctx);
  }

  virtual antlrcpp::Any visitEnumDefinition(SolidityParser::EnumDefinitionContext *ctx) override {
    return visitChildren(ctx);
  }

  virtual antlrcpp::Any visitStateVariableDeclaration(SolidityParser::StateVariableDeclarationContext *ctx) override {
    return visitChildren(ctx);
  }

  virtual antlrcpp::Any visitEventParameter(SolidityParser::EventParameterContext *ctx) override {
    return visitChildren(ctx);
  }

  virtual antlrcpp::Any visitEventDefinition(SolidityParser::EventDefinitionContext *ctx) override {
    return visitChildren(ctx);
  }

  virtual antlrcpp::Any visitUsingDirective(SolidityParser::UsingDirectiveContext *ctx) override {
    return visitChildren(ctx);
  }

  virtual antlrcpp::Any visitTypeName(SolidityParser::TypeNameContext *ctx) override {
    return visitChildren(ctx);
  }

  virtual antlrcpp::Any visitElementaryTypeName(SolidityParser::ElementaryTypeNameContext *ctx) override {
    return visitChildren(ctx);
  }

  virtual antlrcpp::Any visitFunctionTypeName(SolidityParser::FunctionTypeNameContext *ctx) override {
    return visitChildren(ctx);
  }

  virtual antlrcpp::Any visitVariableDeclaration(SolidityParser::VariableDeclarationContext *ctx) override {
    return visitChildren(ctx);
  }

  virtual antlrcpp::Any visitDataLocation(SolidityParser::DataLocationContext *ctx) override {
    return visitChildren(ctx);
  }

  virtual antlrcpp::Any visitUnaryPrefixOperation(SolidityParser::UnaryPrefixOperationContext *ctx) override {
    return visitChildren(ctx);
  }

  virtual antlrcpp::Any visitPrimaryExpression(SolidityParser::PrimaryExpressionContext *ctx) override {
    return visitChildren(ctx);
  }

  virtual antlrcpp::Any visitOrderComparison(SolidityParser::OrderComparisonContext *ctx) override {
    return visitChildren(ctx);
  }

  virtual antlrcpp::Any visitConditional(SolidityParser::ConditionalContext *ctx) override {
    return visitChildren(ctx);
  }

  virtual antlrcpp::Any visitPayableConversion(SolidityParser::PayableConversionContext *ctx) override {
    return visitChildren(ctx);
  }

  virtual antlrcpp::Any visitAssignment(SolidityParser::AssignmentContext *ctx) override {
    return visitChildren(ctx);
  }

  virtual antlrcpp::Any visitUnarySuffixOperation(SolidityParser::UnarySuffixOperationContext *ctx) override {
    return visitChildren(ctx);
  }

  virtual antlrcpp::Any visitShiftOperation(SolidityParser::ShiftOperationContext *ctx) override {
    return visitChildren(ctx);
  }

  virtual antlrcpp::Any visitBitAndOperation(SolidityParser::BitAndOperationContext *ctx) override {
    return visitChildren(ctx);
  }

  virtual antlrcpp::Any visitFunctionCall(SolidityParser::FunctionCallContext *ctx) override {
    return visitChildren(ctx);
  }

  virtual antlrcpp::Any visitIndexRangeAccess(SolidityParser::IndexRangeAccessContext *ctx) override {
    return visitChildren(ctx);
  }

  virtual antlrcpp::Any visitNewExpression(SolidityParser::NewExpressionContext *ctx) override {
    return visitChildren(ctx);
  }

  virtual antlrcpp::Any visitIndexAccess(SolidityParser::IndexAccessContext *ctx) override {
    return visitChildren(ctx);
  }

  virtual antlrcpp::Any visitAddSubOperation(SolidityParser::AddSubOperationContext *ctx) override {
    return visitChildren(ctx);
  }

  virtual antlrcpp::Any visitBitOrOperation(SolidityParser::BitOrOperationContext *ctx) override {
    return visitChildren(ctx);
  }

  virtual antlrcpp::Any visitExpOperation(SolidityParser::ExpOperationContext *ctx) override {
    return visitChildren(ctx);
  }

  virtual antlrcpp::Any visitAndOperation(SolidityParser::AndOperationContext *ctx) override {
    return visitChildren(ctx);
  }

  virtual antlrcpp::Any visitInlineArray(SolidityParser::InlineArrayContext *ctx) override {
    return visitChildren(ctx);
  }

  virtual antlrcpp::Any visitOrOperation(SolidityParser::OrOperationContext *ctx) override {
    return visitChildren(ctx);
  }

  virtual antlrcpp::Any visitMemberAccess(SolidityParser::MemberAccessContext *ctx) override {
    return visitChildren(ctx);
  }

  virtual antlrcpp::Any visitMulDivModOperation(SolidityParser::MulDivModOperationContext *ctx) override {
    return visitChildren(ctx);
  }

  virtual antlrcpp::Any visitFunctionCallOptions(SolidityParser::FunctionCallOptionsContext *ctx) override {
    return visitChildren(ctx);
  }

  virtual antlrcpp::Any visitBitXorOperation(SolidityParser::BitXorOperationContext *ctx) override {
    return visitChildren(ctx);
  }

  virtual antlrcpp::Any visitTuple(SolidityParser::TupleContext *ctx) override {
    return visitChildren(ctx);
  }

  virtual antlrcpp::Any visitEqualityComparison(SolidityParser::EqualityComparisonContext *ctx) override {
    return visitChildren(ctx);
  }

  virtual antlrcpp::Any visitMetaType(SolidityParser::MetaTypeContext *ctx) override {
    return visitChildren(ctx);
  }

  virtual antlrcpp::Any visitAssignOp(SolidityParser::AssignOpContext *ctx) override {
    return visitChildren(ctx);
  }

  virtual antlrcpp::Any visitTupleExpression(SolidityParser::TupleExpressionContext *ctx) override {
    return visitChildren(ctx);
  }

  virtual antlrcpp::Any visitInlineArrayExpression(SolidityParser::InlineArrayExpressionContext *ctx) override {
    return visitChildren(ctx);
  }

  virtual antlrcpp::Any visitIdentifier(SolidityParser::IdentifierContext *ctx) override {
    return visitChildren(ctx);
  }

  virtual antlrcpp::Any visitLiteral(SolidityParser::LiteralContext *ctx) override {
    return visitChildren(ctx);
  }

  virtual antlrcpp::Any visitBooleanLiteral(SolidityParser::boolLiteralContext *ctx) override {
    return visitChildren(ctx);
  }

  virtual antlrcpp::Any visitStringLiteral(SolidityParser::StringLiteralContext *ctx) override {
    return visitChildren(ctx);
  }

  virtual antlrcpp::Any visitHexStringLiteral(SolidityParser::HexStringLiteralContext *ctx) override {
    return visitChildren(ctx);
  }

  virtual antlrcpp::Any visitUnicodeStringLiteral(SolidityParser::UnicodeStringLiteralContext *ctx) override {
    return visitChildren(ctx);
  }

  virtual antlrcpp::Any visitNumberLiteral(SolidityParser::NumberLiteralContext *ctx) override {
    return visitChildren(ctx);
  }

  virtual antlrcpp::Any visitBlock(SolidityParser::BlockContext *ctx) override {
    return visitChildren(ctx);
  }

  virtual antlrcpp::Any visitStatement(SolidityParser::StatementContext *ctx) override {
    return visitChildren(ctx);
  }

  virtual antlrcpp::Any visitSimpleStatement(SolidityParser::SimpleStatementContext *ctx) override {
    return visitChildren(ctx);
  }

  virtual antlrcpp::Any visitIfStatement(SolidityParser::IfStatementContext *ctx) override {
    return visitChildren(ctx);
  }

  virtual antlrcpp::Any visitForStatement(SolidityParser::ForStatementContext *ctx) override {
    return visitChildren(ctx);
  }

  virtual antlrcpp::Any visitWhileStatement(SolidityParser::WhileStatementContext *ctx) override {
    return visitChildren(ctx);
  }

  virtual antlrcpp::Any visitDoWhileStatement(SolidityParser::DoWhileStatementContext *ctx) override {
    return visitChildren(ctx);
  }

  virtual antlrcpp::Any visitContinueStatement(SolidityParser::ContinueStatementContext *ctx) override {
    return visitChildren(ctx);
  }

  virtual antlrcpp::Any visitBreakStatement(SolidityParser::BreakStatementContext *ctx) override {
    return visitChildren(ctx);
  }

  virtual antlrcpp::Any visitTryStatement(SolidityParser::TryStatementContext *ctx) override {
    return visitChildren(ctx);
  }

  virtual antlrcpp::Any visitCatchClause(SolidityParser::CatchClauseContext *ctx) override {
    return visitChildren(ctx);
  }

  virtual antlrcpp::Any visitReturnStatement(SolidityParser::ReturnStatementContext *ctx) override {
    return visitChildren(ctx);
  }

  virtual antlrcpp::Any visitEmitStatement(SolidityParser::EmitStatementContext *ctx) override {
    return visitChildren(ctx);
  }

  virtual antlrcpp::Any visitAssemblyStatement(SolidityParser::AssemblyStatementContext *ctx) override {
    return visitChildren(ctx);
  }

  virtual antlrcpp::Any visitVariableDeclarationList(SolidityParser::VariableDeclarationListContext *ctx) override {
    return visitChildren(ctx);
  }

  virtual antlrcpp::Any visitVariableDeclarationTuple(SolidityParser::VariableDeclarationTupleContext *ctx) override {
    return visitChildren(ctx);
  }

  virtual antlrcpp::Any visitVariableDeclarationStatement(SolidityParser::VariableDeclarationStatementContext *ctx) override {
    return visitChildren(ctx);
  }

  virtual antlrcpp::Any visitExpressionStatement(SolidityParser::ExpressionStatementContext *ctx) override {
    return visitChildren(ctx);
  }

  virtual antlrcpp::Any visitMappingType(SolidityParser::MappingTypeContext *ctx) override {
    return visitChildren(ctx);
  }

  virtual antlrcpp::Any visitMappingKeyType(SolidityParser::MappingKeyTypeContext *ctx) override {
    return visitChildren(ctx);
  }

  virtual antlrcpp::Any visitYulStatement(SolidityParser::YulStatementContext *ctx) override {
    return visitChildren(ctx);
  }

  virtual antlrcpp::Any visitYulBlock(SolidityParser::YulBlockContext *ctx) override {
    return visitChildren(ctx);
  }

  virtual antlrcpp::Any visitYulVariableDeclaration(SolidityParser::YulVariableDeclarationContext *ctx) override {
    return visitChildren(ctx);
  }

  virtual antlrcpp::Any visitYulAssignment(SolidityParser::YulAssignmentContext *ctx) override {
    return visitChildren(ctx);
  }

  virtual antlrcpp::Any visitYulIfStatement(SolidityParser::YulIfStatementContext *ctx) override {
    return visitChildren(ctx);
  }

  virtual antlrcpp::Any visitYulForStatement(SolidityParser::YulForStatementContext *ctx) override {
    return visitChildren(ctx);
  }

  virtual antlrcpp::Any visitYulSwitchCase(SolidityParser::YulSwitchCaseContext *ctx) override {
    return visitChildren(ctx);
  }

  virtual antlrcpp::Any visitYulSwitchStatement(SolidityParser::YulSwitchStatementContext *ctx) override {
    return visitChildren(ctx);
  }

  virtual antlrcpp::Any visitYulFunctionDefinition(SolidityParser::YulFunctionDefinitionContext *ctx) override {
    return visitChildren(ctx);
  }

  virtual antlrcpp::Any visitYulPath(SolidityParser::YulPathContext *ctx) override {
    return visitChildren(ctx);
  }

  virtual antlrcpp::Any visitYulFunctionCall(SolidityParser::YulFunctionCallContext *ctx) override {
    return visitChildren(ctx);
  }

  virtual antlrcpp::Any visitYulBoolean(SolidityParser::YulboolContext *ctx) override {
    return visitChildren(ctx);
  }

  virtual antlrcpp::Any visitYulLiteral(SolidityParser::YulLiteralContext *ctx) override {
    return visitChildren(ctx);
  }

  virtual antlrcpp::Any visitYulExpression(SolidityParser::YulExpressionContext *ctx) override {
    return visitChildren(ctx);
  }


};

