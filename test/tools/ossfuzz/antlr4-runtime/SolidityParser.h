
// Generated from Solidity.g4 by ANTLR 4.8

#pragma once


#include "antlr4-runtime.h"




class  SolidityParser : public antlr4::Parser {
public:
  enum {
    ReservedKeywords = 1, Pragma = 2, Abstract = 3, Anonymous = 4, Address = 5, 
    As = 6, Assembly = 7, Bool = 8, Break = 9, Bytes = 10, Calldata = 11, 
    Catch = 12, Constant = 13, Constructor = 14, Continue = 15, Contract = 16, 
    Delete = 17, Do = 18, Else = 19, Emit = 20, Enum = 21, Event = 22, External = 23, 
    Fallback = 24, False = 25, Fixed = 26, From = 27, FixedBytes = 28, For = 29, 
    Function = 30, Hex = 31, If = 32, Immutable = 33, Import = 34, Indexed = 35, 
    Interface = 36, Internal = 37, Is = 38, Library = 39, Mapping = 40, 
    Memory = 41, Modifier = 42, New = 43, NumberUnit = 44, Override = 45, 
    Payable = 46, Private = 47, Public = 48, Pure = 49, Receive = 50, Return = 51, 
    Returns = 52, SignedIntegerType = 53, Storage = 54, String = 55, Struct = 56, 
    True = 57, Try = 58, Type = 59, Ufixed = 60, UnsignedIntegerType = 61, 
    Using = 62, View = 63, Virtual = 64, While = 65, LParen = 66, RParen = 67, 
    LBrack = 68, RBrack = 69, LBrace = 70, RBrace = 71, Colon = 72, Semicolon = 73, 
    Period = 74, Conditional = 75, Arrow = 76, Assign = 77, AssignBitOr = 78, 
    AssignBitXor = 79, AssignBitAnd = 80, AssignShl = 81, AssignSar = 82, 
    AssignShr = 83, AssignAdd = 84, AssignSub = 85, AssignMul = 86, AssignDiv = 87, 
    AssignMod = 88, Comma = 89, Or = 90, And = 91, BitOr = 92, BitXor = 93, 
    BitAnd = 94, Shl = 95, Sar = 96, Shr = 97, Add = 98, Sub = 99, Mul = 100, 
    Div = 101, Mod = 102, Exp = 103, Equal = 104, NotEqual = 105, LessThan = 106, 
    GreaterThan = 107, LessThanOrEqual = 108, GreaterThanOrEqual = 109, 
    Not = 110, BitNot = 111, Inc = 112, Dec = 113, StringLiteral = 114, 
    NonEmptyStringLiteral = 115, UnicodeStringLiteral = 116, HexString = 117, 
    HexNumber = 118, DecimalNumber = 119, Identifier = 120, WS = 121, COMMENT = 122, 
    LINE_COMMENT = 123, AssemblyDialect = 124, AssemblyLBrace = 125, AssemblyBlockWS = 126, 
    AssemblyBlockCOMMENT = 127, AssemblyBlockLINE_COMMENT = 128, YulBreak = 129, 
    YulCase = 130, YulContinue = 131, YulDefault = 132, YulFalse = 133, 
    YulFor = 134, YulFunction = 135, YulIf = 136, YulLeave = 137, YulLet = 138, 
    YulSwitch = 139, YulTrue = 140, YulEVMBuiltin = 141, YulLBrace = 142, 
    YulRBrace = 143, YulLParen = 144, YulRParen = 145, YulAssign = 146, 
    YulPeriod = 147, YulComma = 148, YulArrow = 149, YulIdentifier = 150, 
    YulHexNumber = 151, YulDecimalNumber = 152, YulStringLiteral = 153, 
    YulWS = 154, YulCOMMENT = 155, YulLINE_COMMENT = 156, PragmaToken = 157, 
    PragmaSemicolon = 158, PragmaWS = 159, PragmaCOMMENT = 160, PragmaLINE_COMMENT = 161
  };

  enum {
    RuleSourceUnit = 0, RulePragmaDirective = 1, RuleImportDirective = 2, 
    RuleImportAliases = 3, RulePath = 4, RuleSymbolAliases = 5, RuleContractDefinition = 6, 
    RuleInterfaceDefinition = 7, RuleLibraryDefinition = 8, RuleInheritanceSpecifierList = 9, 
    RuleInheritanceSpecifier = 10, RuleContractBodyElement = 11, RuleNamedArgument = 12, 
    RuleCallArgumentList = 13, RuleUserDefinedTypeName = 14, RuleModifierInvocation = 15, 
    RuleVisibility = 16, RuleParameterList = 17, RuleParameterDeclaration = 18, 
    RuleConstructorDefinition = 19, RuleStateMutability = 20, RuleOverrideSpecifier = 21, 
    RuleFunctionDefinition = 22, RuleModifierDefinition = 23, RuleFallbackReceiveFunctionDefinition = 24, 
    RuleStructDefinition = 25, RuleStructMember = 26, RuleEnumDefinition = 27, 
    RuleStateVariableDeclaration = 28, RuleEventParameter = 29, RuleEventDefinition = 30, 
    RuleUsingDirective = 31, RuleTypeName = 32, RuleElementaryTypeName = 33, 
    RuleFunctionTypeName = 34, RuleVariableDeclaration = 35, RuleDataLocation = 36, 
    RuleExpression = 37, RuleAssignOp = 38, RuleTupleExpression = 39, RuleInlineArrayExpression = 40, 
    RuleIdentifier = 41, RuleLiteral = 42, RuleboolLiteral = 43, RuleStringLiteral = 44, 
    RuleHexStringLiteral = 45, RuleUnicodeStringLiteral = 46, RuleNumberLiteral = 47, 
    RuleBlock = 48, RuleStatement = 49, RuleSimpleStatement = 50, RuleIfStatement = 51, 
    RuleForStatement = 52, RuleWhileStatement = 53, RuleDoWhileStatement = 54, 
    RuleContinueStatement = 55, RuleBreakStatement = 56, RuleTryStatement = 57, 
    RuleCatchClause = 58, RuleReturnStatement = 59, RuleEmitStatement = 60, 
    RuleAssemblyStatement = 61, RuleVariableDeclarationList = 62, RuleVariableDeclarationTuple = 63, 
    RuleVariableDeclarationStatement = 64, RuleExpressionStatement = 65, 
    RuleMappingType = 66, RuleMappingKeyType = 67, RuleYulStatement = 68, 
    RuleYulBlock = 69, RuleYulVariableDeclaration = 70, RuleYulAssignment = 71, 
    RuleYulIfStatement = 72, RuleYulForStatement = 73, RuleYulSwitchCase = 74, 
    RuleYulSwitchStatement = 75, RuleYulFunctionDefinition = 76, RuleYulPath = 77, 
    RuleYulFunctionCall = 78, RuleYulbool = 79, RuleYulLiteral = 80, 
    RuleYulExpression = 81
  };

  SolidityParser(antlr4::TokenStream *input);
  ~SolidityParser();

  virtual std::string getGrammarFileName() const override;
  virtual const antlr4::atn::ATN& getATN() const override { return _atn; };
  virtual const std::vector<std::string>& getTokenNames() const override { return _tokenNames; }; // deprecated: use vocabulary instead.
  virtual const std::vector<std::string>& getRuleNames() const override;
  virtual antlr4::dfa::Vocabulary& getVocabulary() const override;


  class SourceUnitContext;
  class PragmaDirectiveContext;
  class ImportDirectiveContext;
  class ImportAliasesContext;
  class PathContext;
  class SymbolAliasesContext;
  class ContractDefinitionContext;
  class InterfaceDefinitionContext;
  class LibraryDefinitionContext;
  class InheritanceSpecifierListContext;
  class InheritanceSpecifierContext;
  class ContractBodyElementContext;
  class NamedArgumentContext;
  class CallArgumentListContext;
  class UserDefinedTypeNameContext;
  class ModifierInvocationContext;
  class VisibilityContext;
  class ParameterListContext;
  class ParameterDeclarationContext;
  class ConstructorDefinitionContext;
  class StateMutabilityContext;
  class OverrideSpecifierContext;
  class FunctionDefinitionContext;
  class ModifierDefinitionContext;
  class FallbackReceiveFunctionDefinitionContext;
  class StructDefinitionContext;
  class StructMemberContext;
  class EnumDefinitionContext;
  class StateVariableDeclarationContext;
  class EventParameterContext;
  class EventDefinitionContext;
  class UsingDirectiveContext;
  class TypeNameContext;
  class ElementaryTypeNameContext;
  class FunctionTypeNameContext;
  class VariableDeclarationContext;
  class DataLocationContext;
  class ExpressionContext;
  class AssignOpContext;
  class TupleExpressionContext;
  class InlineArrayExpressionContext;
  class IdentifierContext;
  class LiteralContext;
  class boolLiteralContext;
  class StringLiteralContext;
  class HexStringLiteralContext;
  class UnicodeStringLiteralContext;
  class NumberLiteralContext;
  class BlockContext;
  class StatementContext;
  class SimpleStatementContext;
  class IfStatementContext;
  class ForStatementContext;
  class WhileStatementContext;
  class DoWhileStatementContext;
  class ContinueStatementContext;
  class BreakStatementContext;
  class TryStatementContext;
  class CatchClauseContext;
  class ReturnStatementContext;
  class EmitStatementContext;
  class AssemblyStatementContext;
  class VariableDeclarationListContext;
  class VariableDeclarationTupleContext;
  class VariableDeclarationStatementContext;
  class ExpressionStatementContext;
  class MappingTypeContext;
  class MappingKeyTypeContext;
  class YulStatementContext;
  class YulBlockContext;
  class YulVariableDeclarationContext;
  class YulAssignmentContext;
  class YulIfStatementContext;
  class YulForStatementContext;
  class YulSwitchCaseContext;
  class YulSwitchStatementContext;
  class YulFunctionDefinitionContext;
  class YulPathContext;
  class YulFunctionCallContext;
  class YulboolContext;
  class YulLiteralContext;
  class YulExpressionContext; 

  class  SourceUnitContext : public antlr4::ParserRuleContext {
  public:
    SourceUnitContext(antlr4::ParserRuleContext *parent, size_t invokingState);
    virtual size_t getRuleIndex() const override;
    antlr4::tree::TerminalNode *EOF();
    std::vector<PragmaDirectiveContext *> pragmaDirective();
    PragmaDirectiveContext* pragmaDirective(size_t i);
    std::vector<ImportDirectiveContext *> importDirective();
    ImportDirectiveContext* importDirective(size_t i);
    std::vector<ContractDefinitionContext *> contractDefinition();
    ContractDefinitionContext* contractDefinition(size_t i);
    std::vector<InterfaceDefinitionContext *> interfaceDefinition();
    InterfaceDefinitionContext* interfaceDefinition(size_t i);
    std::vector<LibraryDefinitionContext *> libraryDefinition();
    LibraryDefinitionContext* libraryDefinition(size_t i);
    std::vector<FunctionDefinitionContext *> functionDefinition();
    FunctionDefinitionContext* functionDefinition(size_t i);
    std::vector<StructDefinitionContext *> structDefinition();
    StructDefinitionContext* structDefinition(size_t i);
    std::vector<EnumDefinitionContext *> enumDefinition();
    EnumDefinitionContext* enumDefinition(size_t i);


    virtual antlrcpp::Any accept(antlr4::tree::ParseTreeVisitor *visitor) override;
   
  };

  SourceUnitContext* sourceUnit();

  class  PragmaDirectiveContext : public antlr4::ParserRuleContext {
  public:
    PragmaDirectiveContext(antlr4::ParserRuleContext *parent, size_t invokingState);
    virtual size_t getRuleIndex() const override;
    antlr4::tree::TerminalNode *Pragma();
    antlr4::tree::TerminalNode *PragmaSemicolon();
    std::vector<antlr4::tree::TerminalNode *> PragmaToken();
    antlr4::tree::TerminalNode* PragmaToken(size_t i);


    virtual antlrcpp::Any accept(antlr4::tree::ParseTreeVisitor *visitor) override;
   
  };

  PragmaDirectiveContext* pragmaDirective();

  class  ImportDirectiveContext : public antlr4::ParserRuleContext {
  public:
    SolidityParser::IdentifierContext *unitAlias = nullptr;
    ImportDirectiveContext(antlr4::ParserRuleContext *parent, size_t invokingState);
    virtual size_t getRuleIndex() const override;
    antlr4::tree::TerminalNode *Import();
    antlr4::tree::TerminalNode *Semicolon();
    PathContext *path();
    SymbolAliasesContext *symbolAliases();
    antlr4::tree::TerminalNode *From();
    antlr4::tree::TerminalNode *Mul();
    antlr4::tree::TerminalNode *As();
    IdentifierContext *identifier();


    virtual antlrcpp::Any accept(antlr4::tree::ParseTreeVisitor *visitor) override;
   
  };

  ImportDirectiveContext* importDirective();

  class  ImportAliasesContext : public antlr4::ParserRuleContext {
  public:
    SolidityParser::IdentifierContext *symbol = nullptr;
    SolidityParser::IdentifierContext *alias = nullptr;
    ImportAliasesContext(antlr4::ParserRuleContext *parent, size_t invokingState);
    virtual size_t getRuleIndex() const override;
    std::vector<IdentifierContext *> identifier();
    IdentifierContext* identifier(size_t i);
    antlr4::tree::TerminalNode *As();


    virtual antlrcpp::Any accept(antlr4::tree::ParseTreeVisitor *visitor) override;
   
  };

  ImportAliasesContext* importAliases();

  class  PathContext : public antlr4::ParserRuleContext {
  public:
    PathContext(antlr4::ParserRuleContext *parent, size_t invokingState);
    virtual size_t getRuleIndex() const override;
    antlr4::tree::TerminalNode *NonEmptyStringLiteral();


    virtual antlrcpp::Any accept(antlr4::tree::ParseTreeVisitor *visitor) override;
   
  };

  PathContext* path();

  class  SymbolAliasesContext : public antlr4::ParserRuleContext {
  public:
    SolidityParser::ImportAliasesContext *importAliasesContext = nullptr;
    std::vector<ImportAliasesContext *> aliases;
    SymbolAliasesContext(antlr4::ParserRuleContext *parent, size_t invokingState);
    virtual size_t getRuleIndex() const override;
    antlr4::tree::TerminalNode *LBrace();
    antlr4::tree::TerminalNode *RBrace();
    std::vector<ImportAliasesContext *> importAliases();
    ImportAliasesContext* importAliases(size_t i);
    std::vector<antlr4::tree::TerminalNode *> Comma();
    antlr4::tree::TerminalNode* Comma(size_t i);


    virtual antlrcpp::Any accept(antlr4::tree::ParseTreeVisitor *visitor) override;
   
  };

  SymbolAliasesContext* symbolAliases();

  class  ContractDefinitionContext : public antlr4::ParserRuleContext {
  public:
    SolidityParser::IdentifierContext *name = nullptr;
    ContractDefinitionContext(antlr4::ParserRuleContext *parent, size_t invokingState);
    virtual size_t getRuleIndex() const override;
    antlr4::tree::TerminalNode *Contract();
    antlr4::tree::TerminalNode *LBrace();
    antlr4::tree::TerminalNode *RBrace();
    IdentifierContext *identifier();
    antlr4::tree::TerminalNode *Abstract();
    InheritanceSpecifierListContext *inheritanceSpecifierList();
    std::vector<ContractBodyElementContext *> contractBodyElement();
    ContractBodyElementContext* contractBodyElement(size_t i);


    virtual antlrcpp::Any accept(antlr4::tree::ParseTreeVisitor *visitor) override;
   
  };

  ContractDefinitionContext* contractDefinition();

  class  InterfaceDefinitionContext : public antlr4::ParserRuleContext {
  public:
    SolidityParser::IdentifierContext *name = nullptr;
    InterfaceDefinitionContext(antlr4::ParserRuleContext *parent, size_t invokingState);
    virtual size_t getRuleIndex() const override;
    antlr4::tree::TerminalNode *Interface();
    antlr4::tree::TerminalNode *LBrace();
    antlr4::tree::TerminalNode *RBrace();
    IdentifierContext *identifier();
    InheritanceSpecifierListContext *inheritanceSpecifierList();
    std::vector<ContractBodyElementContext *> contractBodyElement();
    ContractBodyElementContext* contractBodyElement(size_t i);


    virtual antlrcpp::Any accept(antlr4::tree::ParseTreeVisitor *visitor) override;
   
  };

  InterfaceDefinitionContext* interfaceDefinition();

  class  LibraryDefinitionContext : public antlr4::ParserRuleContext {
  public:
    SolidityParser::IdentifierContext *name = nullptr;
    LibraryDefinitionContext(antlr4::ParserRuleContext *parent, size_t invokingState);
    virtual size_t getRuleIndex() const override;
    antlr4::tree::TerminalNode *Library();
    antlr4::tree::TerminalNode *LBrace();
    antlr4::tree::TerminalNode *RBrace();
    IdentifierContext *identifier();
    std::vector<ContractBodyElementContext *> contractBodyElement();
    ContractBodyElementContext* contractBodyElement(size_t i);


    virtual antlrcpp::Any accept(antlr4::tree::ParseTreeVisitor *visitor) override;
   
  };

  LibraryDefinitionContext* libraryDefinition();

  class  InheritanceSpecifierListContext : public antlr4::ParserRuleContext {
  public:
    SolidityParser::InheritanceSpecifierContext *inheritanceSpecifierContext = nullptr;
    std::vector<InheritanceSpecifierContext *> inheritanceSpecifiers;
    InheritanceSpecifierListContext(antlr4::ParserRuleContext *parent, size_t invokingState);
    virtual size_t getRuleIndex() const override;
    antlr4::tree::TerminalNode *Is();
    std::vector<InheritanceSpecifierContext *> inheritanceSpecifier();
    InheritanceSpecifierContext* inheritanceSpecifier(size_t i);
    std::vector<antlr4::tree::TerminalNode *> Comma();
    antlr4::tree::TerminalNode* Comma(size_t i);


    virtual antlrcpp::Any accept(antlr4::tree::ParseTreeVisitor *visitor) override;
   
  };

  InheritanceSpecifierListContext* inheritanceSpecifierList();

  class  InheritanceSpecifierContext : public antlr4::ParserRuleContext {
  public:
    SolidityParser::UserDefinedTypeNameContext *name = nullptr;
    SolidityParser::CallArgumentListContext *arguments = nullptr;
    InheritanceSpecifierContext(antlr4::ParserRuleContext *parent, size_t invokingState);
    virtual size_t getRuleIndex() const override;
    UserDefinedTypeNameContext *userDefinedTypeName();
    CallArgumentListContext *callArgumentList();


    virtual antlrcpp::Any accept(antlr4::tree::ParseTreeVisitor *visitor) override;
   
  };

  InheritanceSpecifierContext* inheritanceSpecifier();

  class  ContractBodyElementContext : public antlr4::ParserRuleContext {
  public:
    ContractBodyElementContext(antlr4::ParserRuleContext *parent, size_t invokingState);
    virtual size_t getRuleIndex() const override;
    ConstructorDefinitionContext *constructorDefinition();
    FunctionDefinitionContext *functionDefinition();
    ModifierDefinitionContext *modifierDefinition();
    FallbackReceiveFunctionDefinitionContext *fallbackReceiveFunctionDefinition();
    StructDefinitionContext *structDefinition();
    EnumDefinitionContext *enumDefinition();
    StateVariableDeclarationContext *stateVariableDeclaration();
    EventDefinitionContext *eventDefinition();
    UsingDirectiveContext *usingDirective();


    virtual antlrcpp::Any accept(antlr4::tree::ParseTreeVisitor *visitor) override;
   
  };

  ContractBodyElementContext* contractBodyElement();

  class  NamedArgumentContext : public antlr4::ParserRuleContext {
  public:
    SolidityParser::IdentifierContext *name = nullptr;
    SolidityParser::ExpressionContext *value = nullptr;
    NamedArgumentContext(antlr4::ParserRuleContext *parent, size_t invokingState);
    virtual size_t getRuleIndex() const override;
    antlr4::tree::TerminalNode *Colon();
    IdentifierContext *identifier();
    ExpressionContext *expression();


    virtual antlrcpp::Any accept(antlr4::tree::ParseTreeVisitor *visitor) override;
   
  };

  NamedArgumentContext* namedArgument();

  class  CallArgumentListContext : public antlr4::ParserRuleContext {
  public:
    CallArgumentListContext(antlr4::ParserRuleContext *parent, size_t invokingState);
    virtual size_t getRuleIndex() const override;
    antlr4::tree::TerminalNode *LParen();
    antlr4::tree::TerminalNode *RParen();
    antlr4::tree::TerminalNode *LBrace();
    antlr4::tree::TerminalNode *RBrace();
    std::vector<ExpressionContext *> expression();
    ExpressionContext* expression(size_t i);
    std::vector<NamedArgumentContext *> namedArgument();
    NamedArgumentContext* namedArgument(size_t i);
    std::vector<antlr4::tree::TerminalNode *> Comma();
    antlr4::tree::TerminalNode* Comma(size_t i);


    virtual antlrcpp::Any accept(antlr4::tree::ParseTreeVisitor *visitor) override;
   
  };

  CallArgumentListContext* callArgumentList();

  class  UserDefinedTypeNameContext : public antlr4::ParserRuleContext {
  public:
    UserDefinedTypeNameContext(antlr4::ParserRuleContext *parent, size_t invokingState);
    virtual size_t getRuleIndex() const override;
    std::vector<IdentifierContext *> identifier();
    IdentifierContext* identifier(size_t i);
    std::vector<antlr4::tree::TerminalNode *> Period();
    antlr4::tree::TerminalNode* Period(size_t i);


    virtual antlrcpp::Any accept(antlr4::tree::ParseTreeVisitor *visitor) override;
   
  };

  UserDefinedTypeNameContext* userDefinedTypeName();

  class  ModifierInvocationContext : public antlr4::ParserRuleContext {
  public:
    ModifierInvocationContext(antlr4::ParserRuleContext *parent, size_t invokingState);
    virtual size_t getRuleIndex() const override;
    IdentifierContext *identifier();
    CallArgumentListContext *callArgumentList();


    virtual antlrcpp::Any accept(antlr4::tree::ParseTreeVisitor *visitor) override;
   
  };

  ModifierInvocationContext* modifierInvocation();

  class  VisibilityContext : public antlr4::ParserRuleContext {
  public:
    VisibilityContext(antlr4::ParserRuleContext *parent, size_t invokingState);
    virtual size_t getRuleIndex() const override;
    antlr4::tree::TerminalNode *Internal();
    antlr4::tree::TerminalNode *External();
    antlr4::tree::TerminalNode *Private();
    antlr4::tree::TerminalNode *Public();


    virtual antlrcpp::Any accept(antlr4::tree::ParseTreeVisitor *visitor) override;
   
  };

  VisibilityContext* visibility();

  class  ParameterListContext : public antlr4::ParserRuleContext {
  public:
    SolidityParser::ParameterDeclarationContext *parameterDeclarationContext = nullptr;
    std::vector<ParameterDeclarationContext *> parameters;
    ParameterListContext(antlr4::ParserRuleContext *parent, size_t invokingState);
    virtual size_t getRuleIndex() const override;
    std::vector<ParameterDeclarationContext *> parameterDeclaration();
    ParameterDeclarationContext* parameterDeclaration(size_t i);
    std::vector<antlr4::tree::TerminalNode *> Comma();
    antlr4::tree::TerminalNode* Comma(size_t i);


    virtual antlrcpp::Any accept(antlr4::tree::ParseTreeVisitor *visitor) override;
   
  };

  ParameterListContext* parameterList();

  class  ParameterDeclarationContext : public antlr4::ParserRuleContext {
  public:
    SolidityParser::TypeNameContext *type = nullptr;
    SolidityParser::DataLocationContext *location = nullptr;
    SolidityParser::IdentifierContext *name = nullptr;
    ParameterDeclarationContext(antlr4::ParserRuleContext *parent, size_t invokingState);
    virtual size_t getRuleIndex() const override;
    TypeNameContext *typeName();
    DataLocationContext *dataLocation();
    IdentifierContext *identifier();


    virtual antlrcpp::Any accept(antlr4::tree::ParseTreeVisitor *visitor) override;
   
  };

  ParameterDeclarationContext* parameterDeclaration();

  class  ConstructorDefinitionContext : public antlr4::ParserRuleContext {
  public:
    bool payableSet = false;
    bool visibilitySet = false;
    SolidityParser::ParameterListContext *arguments = nullptr;
    SolidityParser::BlockContext *body = nullptr;
    ConstructorDefinitionContext(antlr4::ParserRuleContext *parent, size_t invokingState);
    virtual size_t getRuleIndex() const override;
    antlr4::tree::TerminalNode *Constructor();
    antlr4::tree::TerminalNode *LParen();
    antlr4::tree::TerminalNode *RParen();
    BlockContext *block();
    std::vector<ModifierInvocationContext *> modifierInvocation();
    ModifierInvocationContext* modifierInvocation(size_t i);
    std::vector<antlr4::tree::TerminalNode *> Payable();
    antlr4::tree::TerminalNode* Payable(size_t i);
    std::vector<antlr4::tree::TerminalNode *> Internal();
    antlr4::tree::TerminalNode* Internal(size_t i);
    std::vector<antlr4::tree::TerminalNode *> Public();
    antlr4::tree::TerminalNode* Public(size_t i);
    ParameterListContext *parameterList();


    virtual antlrcpp::Any accept(antlr4::tree::ParseTreeVisitor *visitor) override;
   
  };

  ConstructorDefinitionContext* constructorDefinition();

  class  StateMutabilityContext : public antlr4::ParserRuleContext {
  public:
    StateMutabilityContext(antlr4::ParserRuleContext *parent, size_t invokingState);
    virtual size_t getRuleIndex() const override;
    antlr4::tree::TerminalNode *Pure();
    antlr4::tree::TerminalNode *View();
    antlr4::tree::TerminalNode *Payable();


    virtual antlrcpp::Any accept(antlr4::tree::ParseTreeVisitor *visitor) override;
   
  };

  StateMutabilityContext* stateMutability();

  class  OverrideSpecifierContext : public antlr4::ParserRuleContext {
  public:
    SolidityParser::UserDefinedTypeNameContext *userDefinedTypeNameContext = nullptr;
    std::vector<UserDefinedTypeNameContext *> overrides;
    OverrideSpecifierContext(antlr4::ParserRuleContext *parent, size_t invokingState);
    virtual size_t getRuleIndex() const override;
    antlr4::tree::TerminalNode *Override();
    antlr4::tree::TerminalNode *LParen();
    antlr4::tree::TerminalNode *RParen();
    std::vector<UserDefinedTypeNameContext *> userDefinedTypeName();
    UserDefinedTypeNameContext* userDefinedTypeName(size_t i);
    std::vector<antlr4::tree::TerminalNode *> Comma();
    antlr4::tree::TerminalNode* Comma(size_t i);


    virtual antlrcpp::Any accept(antlr4::tree::ParseTreeVisitor *visitor) override;
   
  };

  OverrideSpecifierContext* overrideSpecifier();

  class  FunctionDefinitionContext : public antlr4::ParserRuleContext {
  public:
    bool visibilitySet = false;
    bool mutabilitySet = false;
    bool virtualSet = false;
    bool overrideSpecifierSet = false;
    SolidityParser::ParameterListContext *arguments = nullptr;
    SolidityParser::ParameterListContext *returnParameters = nullptr;
    SolidityParser::BlockContext *body = nullptr;
    FunctionDefinitionContext(antlr4::ParserRuleContext *parent, size_t invokingState);
    virtual size_t getRuleIndex() const override;
    antlr4::tree::TerminalNode *Function();
    std::vector<antlr4::tree::TerminalNode *> LParen();
    antlr4::tree::TerminalNode* LParen(size_t i);
    std::vector<antlr4::tree::TerminalNode *> RParen();
    antlr4::tree::TerminalNode* RParen(size_t i);
    IdentifierContext *identifier();
    antlr4::tree::TerminalNode *Fallback();
    antlr4::tree::TerminalNode *Receive();
    antlr4::tree::TerminalNode *Semicolon();
    std::vector<VisibilityContext *> visibility();
    VisibilityContext* visibility(size_t i);
    std::vector<StateMutabilityContext *> stateMutability();
    StateMutabilityContext* stateMutability(size_t i);
    std::vector<ModifierInvocationContext *> modifierInvocation();
    ModifierInvocationContext* modifierInvocation(size_t i);
    std::vector<antlr4::tree::TerminalNode *> Virtual();
    antlr4::tree::TerminalNode* Virtual(size_t i);
    std::vector<OverrideSpecifierContext *> overrideSpecifier();
    OverrideSpecifierContext* overrideSpecifier(size_t i);
    antlr4::tree::TerminalNode *Returns();
    BlockContext *block();
    std::vector<ParameterListContext *> parameterList();
    ParameterListContext* parameterList(size_t i);


    virtual antlrcpp::Any accept(antlr4::tree::ParseTreeVisitor *visitor) override;
   
  };

  FunctionDefinitionContext* functionDefinition();

  class  ModifierDefinitionContext : public antlr4::ParserRuleContext {
  public:
    bool virtualSet = false;
    bool overrideSpecifierSet = false;
    SolidityParser::IdentifierContext *name = nullptr;
    SolidityParser::ParameterListContext *arguments = nullptr;
    SolidityParser::BlockContext *body = nullptr;
    ModifierDefinitionContext(antlr4::ParserRuleContext *parent, size_t invokingState);
    virtual size_t getRuleIndex() const override;
    antlr4::tree::TerminalNode *Modifier();
    IdentifierContext *identifier();
    antlr4::tree::TerminalNode *Semicolon();
    antlr4::tree::TerminalNode *LParen();
    antlr4::tree::TerminalNode *RParen();
    std::vector<antlr4::tree::TerminalNode *> Virtual();
    antlr4::tree::TerminalNode* Virtual(size_t i);
    std::vector<OverrideSpecifierContext *> overrideSpecifier();
    OverrideSpecifierContext* overrideSpecifier(size_t i);
    BlockContext *block();
    ParameterListContext *parameterList();


    virtual antlrcpp::Any accept(antlr4::tree::ParseTreeVisitor *visitor) override;
   
  };

  ModifierDefinitionContext* modifierDefinition();

  class  FallbackReceiveFunctionDefinitionContext : public antlr4::ParserRuleContext {
  public:
    bool visibilitySet = false;
    bool mutabilitySet = false;
    bool virtualSet = false;
    bool overrideSpecifierSet = false;
    antlr4::Token *kind = nullptr;
    SolidityParser::BlockContext *body = nullptr;
    FallbackReceiveFunctionDefinitionContext(antlr4::ParserRuleContext *parent, size_t invokingState);
    virtual size_t getRuleIndex() const override;
    antlr4::tree::TerminalNode *LParen();
    antlr4::tree::TerminalNode *RParen();
    antlr4::tree::TerminalNode *Fallback();
    antlr4::tree::TerminalNode *Receive();
    antlr4::tree::TerminalNode *Semicolon();
    std::vector<VisibilityContext *> visibility();
    VisibilityContext* visibility(size_t i);
    std::vector<StateMutabilityContext *> stateMutability();
    StateMutabilityContext* stateMutability(size_t i);
    std::vector<ModifierInvocationContext *> modifierInvocation();
    ModifierInvocationContext* modifierInvocation(size_t i);
    std::vector<antlr4::tree::TerminalNode *> Virtual();
    antlr4::tree::TerminalNode* Virtual(size_t i);
    std::vector<OverrideSpecifierContext *> overrideSpecifier();
    OverrideSpecifierContext* overrideSpecifier(size_t i);
    BlockContext *block();


    virtual antlrcpp::Any accept(antlr4::tree::ParseTreeVisitor *visitor) override;
   
  };

  FallbackReceiveFunctionDefinitionContext* fallbackReceiveFunctionDefinition();

  class  StructDefinitionContext : public antlr4::ParserRuleContext {
  public:
    SolidityParser::IdentifierContext *name = nullptr;
    SolidityParser::StructMemberContext *members = nullptr;
    StructDefinitionContext(antlr4::ParserRuleContext *parent, size_t invokingState);
    virtual size_t getRuleIndex() const override;
    antlr4::tree::TerminalNode *Struct();
    antlr4::tree::TerminalNode *LBrace();
    antlr4::tree::TerminalNode *RBrace();
    IdentifierContext *identifier();
    std::vector<StructMemberContext *> structMember();
    StructMemberContext* structMember(size_t i);


    virtual antlrcpp::Any accept(antlr4::tree::ParseTreeVisitor *visitor) override;
   
  };

  StructDefinitionContext* structDefinition();

  class  StructMemberContext : public antlr4::ParserRuleContext {
  public:
    SolidityParser::TypeNameContext *type = nullptr;
    SolidityParser::IdentifierContext *name = nullptr;
    StructMemberContext(antlr4::ParserRuleContext *parent, size_t invokingState);
    virtual size_t getRuleIndex() const override;
    antlr4::tree::TerminalNode *Semicolon();
    TypeNameContext *typeName();
    IdentifierContext *identifier();


    virtual antlrcpp::Any accept(antlr4::tree::ParseTreeVisitor *visitor) override;
   
  };

  StructMemberContext* structMember();

  class  EnumDefinitionContext : public antlr4::ParserRuleContext {
  public:
    SolidityParser::IdentifierContext *name = nullptr;
    SolidityParser::IdentifierContext *identifierContext = nullptr;
    std::vector<IdentifierContext *> enumValues;
    EnumDefinitionContext(antlr4::ParserRuleContext *parent, size_t invokingState);
    virtual size_t getRuleIndex() const override;
    antlr4::tree::TerminalNode *Enum();
    antlr4::tree::TerminalNode *LBrace();
    antlr4::tree::TerminalNode *RBrace();
    std::vector<IdentifierContext *> identifier();
    IdentifierContext* identifier(size_t i);
    std::vector<antlr4::tree::TerminalNode *> Comma();
    antlr4::tree::TerminalNode* Comma(size_t i);


    virtual antlrcpp::Any accept(antlr4::tree::ParseTreeVisitor *visitor) override;
   
  };

  EnumDefinitionContext* enumDefinition();

  class  StateVariableDeclarationContext : public antlr4::ParserRuleContext {
  public:
    bool constantnessSet = false;
    bool visibilitySet = false;
    bool overrideSpecifierSet = false;
    SolidityParser::TypeNameContext *type = nullptr;
    SolidityParser::IdentifierContext *name = nullptr;
    SolidityParser::ExpressionContext *initialValue = nullptr;
    StateVariableDeclarationContext(antlr4::ParserRuleContext *parent, size_t invokingState);
    virtual size_t getRuleIndex() const override;
    antlr4::tree::TerminalNode *Semicolon();
    TypeNameContext *typeName();
    IdentifierContext *identifier();
    std::vector<antlr4::tree::TerminalNode *> Public();
    antlr4::tree::TerminalNode* Public(size_t i);
    std::vector<antlr4::tree::TerminalNode *> Private();
    antlr4::tree::TerminalNode* Private(size_t i);
    std::vector<antlr4::tree::TerminalNode *> Internal();
    antlr4::tree::TerminalNode* Internal(size_t i);
    std::vector<antlr4::tree::TerminalNode *> Constant();
    antlr4::tree::TerminalNode* Constant(size_t i);
    std::vector<OverrideSpecifierContext *> overrideSpecifier();
    OverrideSpecifierContext* overrideSpecifier(size_t i);
    std::vector<antlr4::tree::TerminalNode *> Immutable();
    antlr4::tree::TerminalNode* Immutable(size_t i);
    antlr4::tree::TerminalNode *Assign();
    ExpressionContext *expression();


    virtual antlrcpp::Any accept(antlr4::tree::ParseTreeVisitor *visitor) override;
   
  };

  StateVariableDeclarationContext* stateVariableDeclaration();

  class  EventParameterContext : public antlr4::ParserRuleContext {
  public:
    SolidityParser::TypeNameContext *type = nullptr;
    SolidityParser::IdentifierContext *name = nullptr;
    EventParameterContext(antlr4::ParserRuleContext *parent, size_t invokingState);
    virtual size_t getRuleIndex() const override;
    TypeNameContext *typeName();
    antlr4::tree::TerminalNode *Indexed();
    IdentifierContext *identifier();


    virtual antlrcpp::Any accept(antlr4::tree::ParseTreeVisitor *visitor) override;
   
  };

  EventParameterContext* eventParameter();

  class  EventDefinitionContext : public antlr4::ParserRuleContext {
  public:
    SolidityParser::IdentifierContext *name = nullptr;
    SolidityParser::EventParameterContext *eventParameterContext = nullptr;
    std::vector<EventParameterContext *> parameters;
    EventDefinitionContext(antlr4::ParserRuleContext *parent, size_t invokingState);
    virtual size_t getRuleIndex() const override;
    antlr4::tree::TerminalNode *Event();
    antlr4::tree::TerminalNode *LParen();
    antlr4::tree::TerminalNode *RParen();
    antlr4::tree::TerminalNode *Semicolon();
    IdentifierContext *identifier();
    antlr4::tree::TerminalNode *Anonymous();
    std::vector<EventParameterContext *> eventParameter();
    EventParameterContext* eventParameter(size_t i);
    std::vector<antlr4::tree::TerminalNode *> Comma();
    antlr4::tree::TerminalNode* Comma(size_t i);


    virtual antlrcpp::Any accept(antlr4::tree::ParseTreeVisitor *visitor) override;
   
  };

  EventDefinitionContext* eventDefinition();

  class  UsingDirectiveContext : public antlr4::ParserRuleContext {
  public:
    UsingDirectiveContext(antlr4::ParserRuleContext *parent, size_t invokingState);
    virtual size_t getRuleIndex() const override;
    antlr4::tree::TerminalNode *Using();
    UserDefinedTypeNameContext *userDefinedTypeName();
    antlr4::tree::TerminalNode *For();
    antlr4::tree::TerminalNode *Semicolon();
    antlr4::tree::TerminalNode *Mul();
    TypeNameContext *typeName();


    virtual antlrcpp::Any accept(antlr4::tree::ParseTreeVisitor *visitor) override;
   
  };

  UsingDirectiveContext* usingDirective();

  class  TypeNameContext : public antlr4::ParserRuleContext {
  public:
    TypeNameContext(antlr4::ParserRuleContext *parent, size_t invokingState);
    virtual size_t getRuleIndex() const override;
    ElementaryTypeNameContext *elementaryTypeName();
    FunctionTypeNameContext *functionTypeName();
    MappingTypeContext *mappingType();
    UserDefinedTypeNameContext *userDefinedTypeName();
    TypeNameContext *typeName();
    antlr4::tree::TerminalNode *LBrack();
    antlr4::tree::TerminalNode *RBrack();
    ExpressionContext *expression();


    virtual antlrcpp::Any accept(antlr4::tree::ParseTreeVisitor *visitor) override;
   
  };

  TypeNameContext* typeName();
  TypeNameContext* typeName(int precedence);
  class  ElementaryTypeNameContext : public antlr4::ParserRuleContext {
  public:
    bool allowAddressPayable;
    ElementaryTypeNameContext(antlr4::ParserRuleContext *parent, size_t invokingState);
    ElementaryTypeNameContext(antlr4::ParserRuleContext *parent, size_t invokingState, bool allowAddressPayable);
    virtual size_t getRuleIndex() const override;
    antlr4::tree::TerminalNode *Address();
    antlr4::tree::TerminalNode *Payable();
    antlr4::tree::TerminalNode *Bool();
    antlr4::tree::TerminalNode *String();
    antlr4::tree::TerminalNode *Bytes();
    antlr4::tree::TerminalNode *SignedIntegerType();
    antlr4::tree::TerminalNode *UnsignedIntegerType();
    antlr4::tree::TerminalNode *FixedBytes();
    antlr4::tree::TerminalNode *Fixed();
    antlr4::tree::TerminalNode *Ufixed();


    virtual antlrcpp::Any accept(antlr4::tree::ParseTreeVisitor *visitor) override;
   
  };

  ElementaryTypeNameContext* elementaryTypeName(bool allowAddressPayable);

  class  FunctionTypeNameContext : public antlr4::ParserRuleContext {
  public:
    bool visibilitySet = false;
    bool mutabilitySet = false;
    SolidityParser::ParameterListContext *arguments = nullptr;
    SolidityParser::ParameterListContext *returnParameters = nullptr;
    FunctionTypeNameContext(antlr4::ParserRuleContext *parent, size_t invokingState);
    virtual size_t getRuleIndex() const override;
    antlr4::tree::TerminalNode *Function();
    std::vector<antlr4::tree::TerminalNode *> LParen();
    antlr4::tree::TerminalNode* LParen(size_t i);
    std::vector<antlr4::tree::TerminalNode *> RParen();
    antlr4::tree::TerminalNode* RParen(size_t i);
    std::vector<VisibilityContext *> visibility();
    VisibilityContext* visibility(size_t i);
    std::vector<StateMutabilityContext *> stateMutability();
    StateMutabilityContext* stateMutability(size_t i);
    antlr4::tree::TerminalNode *Returns();
    std::vector<ParameterListContext *> parameterList();
    ParameterListContext* parameterList(size_t i);


    virtual antlrcpp::Any accept(antlr4::tree::ParseTreeVisitor *visitor) override;
   
  };

  FunctionTypeNameContext* functionTypeName();

  class  VariableDeclarationContext : public antlr4::ParserRuleContext {
  public:
    SolidityParser::TypeNameContext *type = nullptr;
    SolidityParser::DataLocationContext *location = nullptr;
    SolidityParser::IdentifierContext *name = nullptr;
    VariableDeclarationContext(antlr4::ParserRuleContext *parent, size_t invokingState);
    virtual size_t getRuleIndex() const override;
    TypeNameContext *typeName();
    IdentifierContext *identifier();
    DataLocationContext *dataLocation();


    virtual antlrcpp::Any accept(antlr4::tree::ParseTreeVisitor *visitor) override;
   
  };

  VariableDeclarationContext* variableDeclaration();

  class  DataLocationContext : public antlr4::ParserRuleContext {
  public:
    DataLocationContext(antlr4::ParserRuleContext *parent, size_t invokingState);
    virtual size_t getRuleIndex() const override;
    antlr4::tree::TerminalNode *Memory();
    antlr4::tree::TerminalNode *Storage();
    antlr4::tree::TerminalNode *Calldata();


    virtual antlrcpp::Any accept(antlr4::tree::ParseTreeVisitor *visitor) override;
   
  };

  DataLocationContext* dataLocation();

  class  ExpressionContext : public antlr4::ParserRuleContext {
  public:
    ExpressionContext(antlr4::ParserRuleContext *parent, size_t invokingState);
   
    ExpressionContext() = default;
    void copyFrom(ExpressionContext *context);
    using antlr4::ParserRuleContext::copyFrom;

    virtual size_t getRuleIndex() const override;

   
  };

  class  UnaryPrefixOperationContext : public ExpressionContext {
  public:
    UnaryPrefixOperationContext(ExpressionContext *ctx);

    ExpressionContext *expression();
    antlr4::tree::TerminalNode *Inc();
    antlr4::tree::TerminalNode *Dec();
    antlr4::tree::TerminalNode *Not();
    antlr4::tree::TerminalNode *BitNot();
    antlr4::tree::TerminalNode *Delete();
    antlr4::tree::TerminalNode *Sub();

    virtual antlrcpp::Any accept(antlr4::tree::ParseTreeVisitor *visitor) override;
  };

  class  PrimaryExpressionContext : public ExpressionContext {
  public:
    PrimaryExpressionContext(ExpressionContext *ctx);

    IdentifierContext *identifier();
    LiteralContext *literal();
    ElementaryTypeNameContext *elementaryTypeName();
    UserDefinedTypeNameContext *userDefinedTypeName();

    virtual antlrcpp::Any accept(antlr4::tree::ParseTreeVisitor *visitor) override;
  };

  class  OrderComparisonContext : public ExpressionContext {
  public:
    OrderComparisonContext(ExpressionContext *ctx);

    std::vector<ExpressionContext *> expression();
    ExpressionContext* expression(size_t i);
    antlr4::tree::TerminalNode *LessThan();
    antlr4::tree::TerminalNode *GreaterThan();
    antlr4::tree::TerminalNode *LessThanOrEqual();
    antlr4::tree::TerminalNode *GreaterThanOrEqual();

    virtual antlrcpp::Any accept(antlr4::tree::ParseTreeVisitor *visitor) override;
  };

  class  ConditionalContext : public ExpressionContext {
  public:
    ConditionalContext(ExpressionContext *ctx);

    std::vector<ExpressionContext *> expression();
    ExpressionContext* expression(size_t i);
    antlr4::tree::TerminalNode *Conditional();
    antlr4::tree::TerminalNode *Colon();

    virtual antlrcpp::Any accept(antlr4::tree::ParseTreeVisitor *visitor) override;
  };

  class  PayableConversionContext : public ExpressionContext {
  public:
    PayableConversionContext(ExpressionContext *ctx);

    antlr4::tree::TerminalNode *Payable();
    CallArgumentListContext *callArgumentList();

    virtual antlrcpp::Any accept(antlr4::tree::ParseTreeVisitor *visitor) override;
  };

  class  AssignmentContext : public ExpressionContext {
  public:
    AssignmentContext(ExpressionContext *ctx);

    std::vector<ExpressionContext *> expression();
    ExpressionContext* expression(size_t i);
    AssignOpContext *assignOp();

    virtual antlrcpp::Any accept(antlr4::tree::ParseTreeVisitor *visitor) override;
  };

  class  UnarySuffixOperationContext : public ExpressionContext {
  public:
    UnarySuffixOperationContext(ExpressionContext *ctx);

    ExpressionContext *expression();
    antlr4::tree::TerminalNode *Inc();
    antlr4::tree::TerminalNode *Dec();

    virtual antlrcpp::Any accept(antlr4::tree::ParseTreeVisitor *visitor) override;
  };

  class  ShiftOperationContext : public ExpressionContext {
  public:
    ShiftOperationContext(ExpressionContext *ctx);

    std::vector<ExpressionContext *> expression();
    ExpressionContext* expression(size_t i);
    antlr4::tree::TerminalNode *Shl();
    antlr4::tree::TerminalNode *Sar();
    antlr4::tree::TerminalNode *Shr();

    virtual antlrcpp::Any accept(antlr4::tree::ParseTreeVisitor *visitor) override;
  };

  class  BitAndOperationContext : public ExpressionContext {
  public:
    BitAndOperationContext(ExpressionContext *ctx);

    std::vector<ExpressionContext *> expression();
    ExpressionContext* expression(size_t i);
    antlr4::tree::TerminalNode *BitAnd();

    virtual antlrcpp::Any accept(antlr4::tree::ParseTreeVisitor *visitor) override;
  };

  class  FunctionCallContext : public ExpressionContext {
  public:
    FunctionCallContext(ExpressionContext *ctx);

    ExpressionContext *expression();
    CallArgumentListContext *callArgumentList();

    virtual antlrcpp::Any accept(antlr4::tree::ParseTreeVisitor *visitor) override;
  };

  class  IndexRangeAccessContext : public ExpressionContext {
  public:
    IndexRangeAccessContext(ExpressionContext *ctx);

    SolidityParser::ExpressionContext *start = nullptr;
    SolidityParser::ExpressionContext *end = nullptr;
    std::vector<ExpressionContext *> expression();
    ExpressionContext* expression(size_t i);
    antlr4::tree::TerminalNode *LBrack();
    antlr4::tree::TerminalNode *Colon();
    antlr4::tree::TerminalNode *RBrack();

    virtual antlrcpp::Any accept(antlr4::tree::ParseTreeVisitor *visitor) override;
  };

  class  NewExpressionContext : public ExpressionContext {
  public:
    NewExpressionContext(ExpressionContext *ctx);

    antlr4::tree::TerminalNode *New();
    TypeNameContext *typeName();

    virtual antlrcpp::Any accept(antlr4::tree::ParseTreeVisitor *visitor) override;
  };

  class  IndexAccessContext : public ExpressionContext {
  public:
    IndexAccessContext(ExpressionContext *ctx);

    SolidityParser::ExpressionContext *index = nullptr;
    std::vector<ExpressionContext *> expression();
    ExpressionContext* expression(size_t i);
    antlr4::tree::TerminalNode *LBrack();
    antlr4::tree::TerminalNode *RBrack();

    virtual antlrcpp::Any accept(antlr4::tree::ParseTreeVisitor *visitor) override;
  };

  class  AddSubOperationContext : public ExpressionContext {
  public:
    AddSubOperationContext(ExpressionContext *ctx);

    std::vector<ExpressionContext *> expression();
    ExpressionContext* expression(size_t i);
    antlr4::tree::TerminalNode *Add();
    antlr4::tree::TerminalNode *Sub();

    virtual antlrcpp::Any accept(antlr4::tree::ParseTreeVisitor *visitor) override;
  };

  class  BitOrOperationContext : public ExpressionContext {
  public:
    BitOrOperationContext(ExpressionContext *ctx);

    std::vector<ExpressionContext *> expression();
    ExpressionContext* expression(size_t i);
    antlr4::tree::TerminalNode *BitOr();

    virtual antlrcpp::Any accept(antlr4::tree::ParseTreeVisitor *visitor) override;
  };

  class  ExpOperationContext : public ExpressionContext {
  public:
    ExpOperationContext(ExpressionContext *ctx);

    std::vector<ExpressionContext *> expression();
    ExpressionContext* expression(size_t i);
    antlr4::tree::TerminalNode *Exp();

    virtual antlrcpp::Any accept(antlr4::tree::ParseTreeVisitor *visitor) override;
  };

  class  AndOperationContext : public ExpressionContext {
  public:
    AndOperationContext(ExpressionContext *ctx);

    std::vector<ExpressionContext *> expression();
    ExpressionContext* expression(size_t i);
    antlr4::tree::TerminalNode *And();

    virtual antlrcpp::Any accept(antlr4::tree::ParseTreeVisitor *visitor) override;
  };

  class  InlineArrayContext : public ExpressionContext {
  public:
    InlineArrayContext(ExpressionContext *ctx);

    InlineArrayExpressionContext *inlineArrayExpression();

    virtual antlrcpp::Any accept(antlr4::tree::ParseTreeVisitor *visitor) override;
  };

  class  OrOperationContext : public ExpressionContext {
  public:
    OrOperationContext(ExpressionContext *ctx);

    std::vector<ExpressionContext *> expression();
    ExpressionContext* expression(size_t i);
    antlr4::tree::TerminalNode *Or();

    virtual antlrcpp::Any accept(antlr4::tree::ParseTreeVisitor *visitor) override;
  };

  class  MemberAccessContext : public ExpressionContext {
  public:
    MemberAccessContext(ExpressionContext *ctx);

    ExpressionContext *expression();
    antlr4::tree::TerminalNode *Period();
    IdentifierContext *identifier();
    antlr4::tree::TerminalNode *Address();

    virtual antlrcpp::Any accept(antlr4::tree::ParseTreeVisitor *visitor) override;
  };

  class  MulDivModOperationContext : public ExpressionContext {
  public:
    MulDivModOperationContext(ExpressionContext *ctx);

    std::vector<ExpressionContext *> expression();
    ExpressionContext* expression(size_t i);
    antlr4::tree::TerminalNode *Mul();
    antlr4::tree::TerminalNode *Div();
    antlr4::tree::TerminalNode *Mod();

    virtual antlrcpp::Any accept(antlr4::tree::ParseTreeVisitor *visitor) override;
  };

  class  FunctionCallOptionsContext : public ExpressionContext {
  public:
    FunctionCallOptionsContext(ExpressionContext *ctx);

    ExpressionContext *expression();
    antlr4::tree::TerminalNode *LBrace();
    antlr4::tree::TerminalNode *RBrace();
    std::vector<NamedArgumentContext *> namedArgument();
    NamedArgumentContext* namedArgument(size_t i);
    std::vector<antlr4::tree::TerminalNode *> Comma();
    antlr4::tree::TerminalNode* Comma(size_t i);

    virtual antlrcpp::Any accept(antlr4::tree::ParseTreeVisitor *visitor) override;
  };

  class  BitXorOperationContext : public ExpressionContext {
  public:
    BitXorOperationContext(ExpressionContext *ctx);

    std::vector<ExpressionContext *> expression();
    ExpressionContext* expression(size_t i);
    antlr4::tree::TerminalNode *BitXor();

    virtual antlrcpp::Any accept(antlr4::tree::ParseTreeVisitor *visitor) override;
  };

  class  TupleContext : public ExpressionContext {
  public:
    TupleContext(ExpressionContext *ctx);

    TupleExpressionContext *tupleExpression();

    virtual antlrcpp::Any accept(antlr4::tree::ParseTreeVisitor *visitor) override;
  };

  class  EqualityComparisonContext : public ExpressionContext {
  public:
    EqualityComparisonContext(ExpressionContext *ctx);

    std::vector<ExpressionContext *> expression();
    ExpressionContext* expression(size_t i);
    antlr4::tree::TerminalNode *Equal();
    antlr4::tree::TerminalNode *NotEqual();

    virtual antlrcpp::Any accept(antlr4::tree::ParseTreeVisitor *visitor) override;
  };

  class  MetaTypeContext : public ExpressionContext {
  public:
    MetaTypeContext(ExpressionContext *ctx);

    antlr4::tree::TerminalNode *Type();
    antlr4::tree::TerminalNode *LParen();
    TypeNameContext *typeName();
    antlr4::tree::TerminalNode *RParen();

    virtual antlrcpp::Any accept(antlr4::tree::ParseTreeVisitor *visitor) override;
  };

  ExpressionContext* expression();
  ExpressionContext* expression(int precedence);
  class  AssignOpContext : public antlr4::ParserRuleContext {
  public:
    AssignOpContext(antlr4::ParserRuleContext *parent, size_t invokingState);
    virtual size_t getRuleIndex() const override;
    antlr4::tree::TerminalNode *Assign();
    antlr4::tree::TerminalNode *AssignBitOr();
    antlr4::tree::TerminalNode *AssignBitXor();
    antlr4::tree::TerminalNode *AssignBitAnd();
    antlr4::tree::TerminalNode *AssignShl();
    antlr4::tree::TerminalNode *AssignSar();
    antlr4::tree::TerminalNode *AssignShr();
    antlr4::tree::TerminalNode *AssignAdd();
    antlr4::tree::TerminalNode *AssignSub();
    antlr4::tree::TerminalNode *AssignMul();
    antlr4::tree::TerminalNode *AssignDiv();
    antlr4::tree::TerminalNode *AssignMod();


    virtual antlrcpp::Any accept(antlr4::tree::ParseTreeVisitor *visitor) override;
   
  };

  AssignOpContext* assignOp();

  class  TupleExpressionContext : public antlr4::ParserRuleContext {
  public:
    TupleExpressionContext(antlr4::ParserRuleContext *parent, size_t invokingState);
    virtual size_t getRuleIndex() const override;
    antlr4::tree::TerminalNode *LParen();
    antlr4::tree::TerminalNode *RParen();
    std::vector<ExpressionContext *> expression();
    ExpressionContext* expression(size_t i);
    std::vector<antlr4::tree::TerminalNode *> Comma();
    antlr4::tree::TerminalNode* Comma(size_t i);


    virtual antlrcpp::Any accept(antlr4::tree::ParseTreeVisitor *visitor) override;
   
  };

  TupleExpressionContext* tupleExpression();

  class  InlineArrayExpressionContext : public antlr4::ParserRuleContext {
  public:
    InlineArrayExpressionContext(antlr4::ParserRuleContext *parent, size_t invokingState);
    virtual size_t getRuleIndex() const override;
    antlr4::tree::TerminalNode *LBrack();
    antlr4::tree::TerminalNode *RBrack();
    std::vector<ExpressionContext *> expression();
    ExpressionContext* expression(size_t i);
    std::vector<antlr4::tree::TerminalNode *> Comma();
    antlr4::tree::TerminalNode* Comma(size_t i);


    virtual antlrcpp::Any accept(antlr4::tree::ParseTreeVisitor *visitor) override;
   
  };

  InlineArrayExpressionContext* inlineArrayExpression();

  class  IdentifierContext : public antlr4::ParserRuleContext {
  public:
    IdentifierContext(antlr4::ParserRuleContext *parent, size_t invokingState);
    virtual size_t getRuleIndex() const override;
    antlr4::tree::TerminalNode *Identifier();
    antlr4::tree::TerminalNode *From();


    virtual antlrcpp::Any accept(antlr4::tree::ParseTreeVisitor *visitor) override;
   
  };

  IdentifierContext* identifier();

  class  LiteralContext : public antlr4::ParserRuleContext {
  public:
    LiteralContext(antlr4::ParserRuleContext *parent, size_t invokingState);
    virtual size_t getRuleIndex() const override;
    StringLiteralContext *stringLiteral();
    NumberLiteralContext *numberLiteral();
    boolLiteralContext *boolLiteral();
    HexStringLiteralContext *hexStringLiteral();
    UnicodeStringLiteralContext *unicodeStringLiteral();


    virtual antlrcpp::Any accept(antlr4::tree::ParseTreeVisitor *visitor) override;
   
  };

  LiteralContext* literal();

  class  boolLiteralContext : public antlr4::ParserRuleContext {
  public:
    boolLiteralContext(antlr4::ParserRuleContext *parent, size_t invokingState);
    virtual size_t getRuleIndex() const override;
    antlr4::tree::TerminalNode *True();
    antlr4::tree::TerminalNode *False();


    virtual antlrcpp::Any accept(antlr4::tree::ParseTreeVisitor *visitor) override;
   
  };

  boolLiteralContext* boolLiteral();

  class  StringLiteralContext : public antlr4::ParserRuleContext {
  public:
    StringLiteralContext(antlr4::ParserRuleContext *parent, size_t invokingState);
    virtual size_t getRuleIndex() const override;
    std::vector<antlr4::tree::TerminalNode *> StringLiteral();
    antlr4::tree::TerminalNode* StringLiteral(size_t i);


    virtual antlrcpp::Any accept(antlr4::tree::ParseTreeVisitor *visitor) override;
   
  };

  StringLiteralContext* stringLiteral();

  class  HexStringLiteralContext : public antlr4::ParserRuleContext {
  public:
    HexStringLiteralContext(antlr4::ParserRuleContext *parent, size_t invokingState);
    virtual size_t getRuleIndex() const override;
    std::vector<antlr4::tree::TerminalNode *> HexString();
    antlr4::tree::TerminalNode* HexString(size_t i);


    virtual antlrcpp::Any accept(antlr4::tree::ParseTreeVisitor *visitor) override;
   
  };

  HexStringLiteralContext* hexStringLiteral();

  class  UnicodeStringLiteralContext : public antlr4::ParserRuleContext {
  public:
    UnicodeStringLiteralContext(antlr4::ParserRuleContext *parent, size_t invokingState);
    virtual size_t getRuleIndex() const override;
    std::vector<antlr4::tree::TerminalNode *> UnicodeStringLiteral();
    antlr4::tree::TerminalNode* UnicodeStringLiteral(size_t i);


    virtual antlrcpp::Any accept(antlr4::tree::ParseTreeVisitor *visitor) override;
   
  };

  UnicodeStringLiteralContext* unicodeStringLiteral();

  class  NumberLiteralContext : public antlr4::ParserRuleContext {
  public:
    NumberLiteralContext(antlr4::ParserRuleContext *parent, size_t invokingState);
    virtual size_t getRuleIndex() const override;
    antlr4::tree::TerminalNode *DecimalNumber();
    antlr4::tree::TerminalNode *HexNumber();
    antlr4::tree::TerminalNode *NumberUnit();


    virtual antlrcpp::Any accept(antlr4::tree::ParseTreeVisitor *visitor) override;
   
  };

  NumberLiteralContext* numberLiteral();

  class  BlockContext : public antlr4::ParserRuleContext {
  public:
    BlockContext(antlr4::ParserRuleContext *parent, size_t invokingState);
    virtual size_t getRuleIndex() const override;
    antlr4::tree::TerminalNode *LBrace();
    antlr4::tree::TerminalNode *RBrace();
    std::vector<StatementContext *> statement();
    StatementContext* statement(size_t i);


    virtual antlrcpp::Any accept(antlr4::tree::ParseTreeVisitor *visitor) override;
   
  };

  BlockContext* block();

  class  StatementContext : public antlr4::ParserRuleContext {
  public:
    StatementContext(antlr4::ParserRuleContext *parent, size_t invokingState);
    virtual size_t getRuleIndex() const override;
    BlockContext *block();
    SimpleStatementContext *simpleStatement();
    IfStatementContext *ifStatement();
    ForStatementContext *forStatement();
    WhileStatementContext *whileStatement();
    DoWhileStatementContext *doWhileStatement();
    ContinueStatementContext *continueStatement();
    BreakStatementContext *breakStatement();
    TryStatementContext *tryStatement();
    ReturnStatementContext *returnStatement();
    EmitStatementContext *emitStatement();
    AssemblyStatementContext *assemblyStatement();


    virtual antlrcpp::Any accept(antlr4::tree::ParseTreeVisitor *visitor) override;
   
  };

  StatementContext* statement();

  class  SimpleStatementContext : public antlr4::ParserRuleContext {
  public:
    SimpleStatementContext(antlr4::ParserRuleContext *parent, size_t invokingState);
    virtual size_t getRuleIndex() const override;
    VariableDeclarationStatementContext *variableDeclarationStatement();
    ExpressionStatementContext *expressionStatement();


    virtual antlrcpp::Any accept(antlr4::tree::ParseTreeVisitor *visitor) override;
   
  };

  SimpleStatementContext* simpleStatement();

  class  IfStatementContext : public antlr4::ParserRuleContext {
  public:
    IfStatementContext(antlr4::ParserRuleContext *parent, size_t invokingState);
    virtual size_t getRuleIndex() const override;
    antlr4::tree::TerminalNode *If();
    antlr4::tree::TerminalNode *LParen();
    ExpressionContext *expression();
    antlr4::tree::TerminalNode *RParen();
    std::vector<StatementContext *> statement();
    StatementContext* statement(size_t i);
    antlr4::tree::TerminalNode *Else();


    virtual antlrcpp::Any accept(antlr4::tree::ParseTreeVisitor *visitor) override;
   
  };

  IfStatementContext* ifStatement();

  class  ForStatementContext : public antlr4::ParserRuleContext {
  public:
    ForStatementContext(antlr4::ParserRuleContext *parent, size_t invokingState);
    virtual size_t getRuleIndex() const override;
    antlr4::tree::TerminalNode *For();
    antlr4::tree::TerminalNode *LParen();
    antlr4::tree::TerminalNode *RParen();
    StatementContext *statement();
    SimpleStatementContext *simpleStatement();
    std::vector<antlr4::tree::TerminalNode *> Semicolon();
    antlr4::tree::TerminalNode* Semicolon(size_t i);
    ExpressionStatementContext *expressionStatement();
    ExpressionContext *expression();


    virtual antlrcpp::Any accept(antlr4::tree::ParseTreeVisitor *visitor) override;
   
  };

  ForStatementContext* forStatement();

  class  WhileStatementContext : public antlr4::ParserRuleContext {
  public:
    WhileStatementContext(antlr4::ParserRuleContext *parent, size_t invokingState);
    virtual size_t getRuleIndex() const override;
    antlr4::tree::TerminalNode *While();
    antlr4::tree::TerminalNode *LParen();
    ExpressionContext *expression();
    antlr4::tree::TerminalNode *RParen();
    StatementContext *statement();


    virtual antlrcpp::Any accept(antlr4::tree::ParseTreeVisitor *visitor) override;
   
  };

  WhileStatementContext* whileStatement();

  class  DoWhileStatementContext : public antlr4::ParserRuleContext {
  public:
    DoWhileStatementContext(antlr4::ParserRuleContext *parent, size_t invokingState);
    virtual size_t getRuleIndex() const override;
    antlr4::tree::TerminalNode *Do();
    StatementContext *statement();
    antlr4::tree::TerminalNode *While();
    antlr4::tree::TerminalNode *LParen();
    ExpressionContext *expression();
    antlr4::tree::TerminalNode *RParen();
    antlr4::tree::TerminalNode *Semicolon();


    virtual antlrcpp::Any accept(antlr4::tree::ParseTreeVisitor *visitor) override;
   
  };

  DoWhileStatementContext* doWhileStatement();

  class  ContinueStatementContext : public antlr4::ParserRuleContext {
  public:
    ContinueStatementContext(antlr4::ParserRuleContext *parent, size_t invokingState);
    virtual size_t getRuleIndex() const override;
    antlr4::tree::TerminalNode *Continue();
    antlr4::tree::TerminalNode *Semicolon();


    virtual antlrcpp::Any accept(antlr4::tree::ParseTreeVisitor *visitor) override;
   
  };

  ContinueStatementContext* continueStatement();

  class  BreakStatementContext : public antlr4::ParserRuleContext {
  public:
    BreakStatementContext(antlr4::ParserRuleContext *parent, size_t invokingState);
    virtual size_t getRuleIndex() const override;
    antlr4::tree::TerminalNode *Break();
    antlr4::tree::TerminalNode *Semicolon();


    virtual antlrcpp::Any accept(antlr4::tree::ParseTreeVisitor *visitor) override;
   
  };

  BreakStatementContext* breakStatement();

  class  TryStatementContext : public antlr4::ParserRuleContext {
  public:
    SolidityParser::ParameterListContext *returnParameters = nullptr;
    TryStatementContext(antlr4::ParserRuleContext *parent, size_t invokingState);
    virtual size_t getRuleIndex() const override;
    antlr4::tree::TerminalNode *Try();
    ExpressionContext *expression();
    BlockContext *block();
    antlr4::tree::TerminalNode *Returns();
    antlr4::tree::TerminalNode *LParen();
    antlr4::tree::TerminalNode *RParen();
    std::vector<CatchClauseContext *> catchClause();
    CatchClauseContext* catchClause(size_t i);
    ParameterListContext *parameterList();


    virtual antlrcpp::Any accept(antlr4::tree::ParseTreeVisitor *visitor) override;
   
  };

  TryStatementContext* tryStatement();

  class  CatchClauseContext : public antlr4::ParserRuleContext {
  public:
    SolidityParser::ParameterListContext *arguments = nullptr;
    CatchClauseContext(antlr4::ParserRuleContext *parent, size_t invokingState);
    virtual size_t getRuleIndex() const override;
    antlr4::tree::TerminalNode *Catch();
    BlockContext *block();
    antlr4::tree::TerminalNode *LParen();
    antlr4::tree::TerminalNode *RParen();
    IdentifierContext *identifier();
    ParameterListContext *parameterList();


    virtual antlrcpp::Any accept(antlr4::tree::ParseTreeVisitor *visitor) override;
   
  };

  CatchClauseContext* catchClause();

  class  ReturnStatementContext : public antlr4::ParserRuleContext {
  public:
    ReturnStatementContext(antlr4::ParserRuleContext *parent, size_t invokingState);
    virtual size_t getRuleIndex() const override;
    antlr4::tree::TerminalNode *Return();
    antlr4::tree::TerminalNode *Semicolon();
    ExpressionContext *expression();


    virtual antlrcpp::Any accept(antlr4::tree::ParseTreeVisitor *visitor) override;
   
  };

  ReturnStatementContext* returnStatement();

  class  EmitStatementContext : public antlr4::ParserRuleContext {
  public:
    EmitStatementContext(antlr4::ParserRuleContext *parent, size_t invokingState);
    virtual size_t getRuleIndex() const override;
    antlr4::tree::TerminalNode *Emit();
    ExpressionContext *expression();
    CallArgumentListContext *callArgumentList();
    antlr4::tree::TerminalNode *Semicolon();


    virtual antlrcpp::Any accept(antlr4::tree::ParseTreeVisitor *visitor) override;
   
  };

  EmitStatementContext* emitStatement();

  class  AssemblyStatementContext : public antlr4::ParserRuleContext {
  public:
    AssemblyStatementContext(antlr4::ParserRuleContext *parent, size_t invokingState);
    virtual size_t getRuleIndex() const override;
    antlr4::tree::TerminalNode *Assembly();
    antlr4::tree::TerminalNode *AssemblyLBrace();
    antlr4::tree::TerminalNode *YulRBrace();
    antlr4::tree::TerminalNode *AssemblyDialect();
    std::vector<YulStatementContext *> yulStatement();
    YulStatementContext* yulStatement(size_t i);


    virtual antlrcpp::Any accept(antlr4::tree::ParseTreeVisitor *visitor) override;
   
  };

  AssemblyStatementContext* assemblyStatement();

  class  VariableDeclarationListContext : public antlr4::ParserRuleContext {
  public:
    SolidityParser::VariableDeclarationContext *variableDeclarationContext = nullptr;
    std::vector<VariableDeclarationContext *> variableDeclarations;
    VariableDeclarationListContext(antlr4::ParserRuleContext *parent, size_t invokingState);
    virtual size_t getRuleIndex() const override;
    std::vector<VariableDeclarationContext *> variableDeclaration();
    VariableDeclarationContext* variableDeclaration(size_t i);
    std::vector<antlr4::tree::TerminalNode *> Comma();
    antlr4::tree::TerminalNode* Comma(size_t i);


    virtual antlrcpp::Any accept(antlr4::tree::ParseTreeVisitor *visitor) override;
   
  };

  VariableDeclarationListContext* variableDeclarationList();

  class  VariableDeclarationTupleContext : public antlr4::ParserRuleContext {
  public:
    SolidityParser::VariableDeclarationContext *variableDeclarationContext = nullptr;
    std::vector<VariableDeclarationContext *> variableDeclarations;
    VariableDeclarationTupleContext(antlr4::ParserRuleContext *parent, size_t invokingState);
    virtual size_t getRuleIndex() const override;
    antlr4::tree::TerminalNode *LParen();
    antlr4::tree::TerminalNode *RParen();
    std::vector<VariableDeclarationContext *> variableDeclaration();
    VariableDeclarationContext* variableDeclaration(size_t i);
    std::vector<antlr4::tree::TerminalNode *> Comma();
    antlr4::tree::TerminalNode* Comma(size_t i);


    virtual antlrcpp::Any accept(antlr4::tree::ParseTreeVisitor *visitor) override;
   
  };

  VariableDeclarationTupleContext* variableDeclarationTuple();

  class  VariableDeclarationStatementContext : public antlr4::ParserRuleContext {
  public:
    VariableDeclarationStatementContext(antlr4::ParserRuleContext *parent, size_t invokingState);
    virtual size_t getRuleIndex() const override;
    antlr4::tree::TerminalNode *Semicolon();
    VariableDeclarationContext *variableDeclaration();
    VariableDeclarationTupleContext *variableDeclarationTuple();
    antlr4::tree::TerminalNode *Assign();
    ExpressionContext *expression();


    virtual antlrcpp::Any accept(antlr4::tree::ParseTreeVisitor *visitor) override;
   
  };

  VariableDeclarationStatementContext* variableDeclarationStatement();

  class  ExpressionStatementContext : public antlr4::ParserRuleContext {
  public:
    ExpressionStatementContext(antlr4::ParserRuleContext *parent, size_t invokingState);
    virtual size_t getRuleIndex() const override;
    ExpressionContext *expression();
    antlr4::tree::TerminalNode *Semicolon();


    virtual antlrcpp::Any accept(antlr4::tree::ParseTreeVisitor *visitor) override;
   
  };

  ExpressionStatementContext* expressionStatement();

  class  MappingTypeContext : public antlr4::ParserRuleContext {
  public:
    SolidityParser::MappingKeyTypeContext *key = nullptr;
    SolidityParser::TypeNameContext *value = nullptr;
    MappingTypeContext(antlr4::ParserRuleContext *parent, size_t invokingState);
    virtual size_t getRuleIndex() const override;
    antlr4::tree::TerminalNode *Mapping();
    antlr4::tree::TerminalNode *LParen();
    antlr4::tree::TerminalNode *Arrow();
    antlr4::tree::TerminalNode *RParen();
    MappingKeyTypeContext *mappingKeyType();
    TypeNameContext *typeName();


    virtual antlrcpp::Any accept(antlr4::tree::ParseTreeVisitor *visitor) override;
   
  };

  MappingTypeContext* mappingType();

  class  MappingKeyTypeContext : public antlr4::ParserRuleContext {
  public:
    MappingKeyTypeContext(antlr4::ParserRuleContext *parent, size_t invokingState);
    virtual size_t getRuleIndex() const override;
    ElementaryTypeNameContext *elementaryTypeName();
    UserDefinedTypeNameContext *userDefinedTypeName();


    virtual antlrcpp::Any accept(antlr4::tree::ParseTreeVisitor *visitor) override;
   
  };

  MappingKeyTypeContext* mappingKeyType();

  class  YulStatementContext : public antlr4::ParserRuleContext {
  public:
    YulStatementContext(antlr4::ParserRuleContext *parent, size_t invokingState);
    virtual size_t getRuleIndex() const override;
    YulBlockContext *yulBlock();
    YulVariableDeclarationContext *yulVariableDeclaration();
    YulAssignmentContext *yulAssignment();
    YulFunctionCallContext *yulFunctionCall();
    YulIfStatementContext *yulIfStatement();
    YulForStatementContext *yulForStatement();
    YulSwitchStatementContext *yulSwitchStatement();
    antlr4::tree::TerminalNode *YulLeave();
    antlr4::tree::TerminalNode *YulBreak();
    antlr4::tree::TerminalNode *YulContinue();
    YulFunctionDefinitionContext *yulFunctionDefinition();


    virtual antlrcpp::Any accept(antlr4::tree::ParseTreeVisitor *visitor) override;
   
  };

  YulStatementContext* yulStatement();

  class  YulBlockContext : public antlr4::ParserRuleContext {
  public:
    YulBlockContext(antlr4::ParserRuleContext *parent, size_t invokingState);
    virtual size_t getRuleIndex() const override;
    antlr4::tree::TerminalNode *YulLBrace();
    antlr4::tree::TerminalNode *YulRBrace();
    std::vector<YulStatementContext *> yulStatement();
    YulStatementContext* yulStatement(size_t i);


    virtual antlrcpp::Any accept(antlr4::tree::ParseTreeVisitor *visitor) override;
   
  };

  YulBlockContext* yulBlock();

  class  YulVariableDeclarationContext : public antlr4::ParserRuleContext {
  public:
    antlr4::Token *yulidentifierToken = nullptr;
    std::vector<antlr4::Token *> variables;
    YulVariableDeclarationContext(antlr4::ParserRuleContext *parent, size_t invokingState);
    virtual size_t getRuleIndex() const override;
    antlr4::tree::TerminalNode *YulLet();
    std::vector<antlr4::tree::TerminalNode *> YulIdentifier();
    antlr4::tree::TerminalNode* YulIdentifier(size_t i);
    antlr4::tree::TerminalNode *YulAssign();
    YulExpressionContext *yulExpression();
    std::vector<antlr4::tree::TerminalNode *> YulComma();
    antlr4::tree::TerminalNode* YulComma(size_t i);
    YulFunctionCallContext *yulFunctionCall();


    virtual antlrcpp::Any accept(antlr4::tree::ParseTreeVisitor *visitor) override;
   
  };

  YulVariableDeclarationContext* yulVariableDeclaration();

  class  YulAssignmentContext : public antlr4::ParserRuleContext {
  public:
    YulAssignmentContext(antlr4::ParserRuleContext *parent, size_t invokingState);
    virtual size_t getRuleIndex() const override;
    std::vector<YulPathContext *> yulPath();
    YulPathContext* yulPath(size_t i);
    antlr4::tree::TerminalNode *YulAssign();
    YulExpressionContext *yulExpression();
    YulFunctionCallContext *yulFunctionCall();
    std::vector<antlr4::tree::TerminalNode *> YulComma();
    antlr4::tree::TerminalNode* YulComma(size_t i);


    virtual antlrcpp::Any accept(antlr4::tree::ParseTreeVisitor *visitor) override;
   
  };

  YulAssignmentContext* yulAssignment();

  class  YulIfStatementContext : public antlr4::ParserRuleContext {
  public:
    SolidityParser::YulExpressionContext *cond = nullptr;
    SolidityParser::YulBlockContext *body = nullptr;
    YulIfStatementContext(antlr4::ParserRuleContext *parent, size_t invokingState);
    virtual size_t getRuleIndex() const override;
    antlr4::tree::TerminalNode *YulIf();
    YulExpressionContext *yulExpression();
    YulBlockContext *yulBlock();


    virtual antlrcpp::Any accept(antlr4::tree::ParseTreeVisitor *visitor) override;
   
  };

  YulIfStatementContext* yulIfStatement();

  class  YulForStatementContext : public antlr4::ParserRuleContext {
  public:
    SolidityParser::YulBlockContext *init = nullptr;
    SolidityParser::YulExpressionContext *cond = nullptr;
    SolidityParser::YulBlockContext *post = nullptr;
    SolidityParser::YulBlockContext *body = nullptr;
    YulForStatementContext(antlr4::ParserRuleContext *parent, size_t invokingState);
    virtual size_t getRuleIndex() const override;
    antlr4::tree::TerminalNode *YulFor();
    std::vector<YulBlockContext *> yulBlock();
    YulBlockContext* yulBlock(size_t i);
    YulExpressionContext *yulExpression();


    virtual antlrcpp::Any accept(antlr4::tree::ParseTreeVisitor *visitor) override;
   
  };

  YulForStatementContext* yulForStatement();

  class  YulSwitchCaseContext : public antlr4::ParserRuleContext {
  public:
    YulSwitchCaseContext(antlr4::ParserRuleContext *parent, size_t invokingState);
    virtual size_t getRuleIndex() const override;
    antlr4::tree::TerminalNode *YulCase();
    YulLiteralContext *yulLiteral();
    YulBlockContext *yulBlock();


    virtual antlrcpp::Any accept(antlr4::tree::ParseTreeVisitor *visitor) override;
   
  };

  YulSwitchCaseContext* yulSwitchCase();

  class  YulSwitchStatementContext : public antlr4::ParserRuleContext {
  public:
    YulSwitchStatementContext(antlr4::ParserRuleContext *parent, size_t invokingState);
    virtual size_t getRuleIndex() const override;
    antlr4::tree::TerminalNode *YulSwitch();
    YulExpressionContext *yulExpression();
    antlr4::tree::TerminalNode *YulDefault();
    YulBlockContext *yulBlock();
    std::vector<YulSwitchCaseContext *> yulSwitchCase();
    YulSwitchCaseContext* yulSwitchCase(size_t i);


    virtual antlrcpp::Any accept(antlr4::tree::ParseTreeVisitor *visitor) override;
   
  };

  YulSwitchStatementContext* yulSwitchStatement();

  class  YulFunctionDefinitionContext : public antlr4::ParserRuleContext {
  public:
    antlr4::Token *yulidentifierToken = nullptr;
    std::vector<antlr4::Token *> arguments;
    std::vector<antlr4::Token *> returnParameters;
    SolidityParser::YulBlockContext *body = nullptr;
    YulFunctionDefinitionContext(antlr4::ParserRuleContext *parent, size_t invokingState);
    virtual size_t getRuleIndex() const override;
    antlr4::tree::TerminalNode *YulFunction();
    std::vector<antlr4::tree::TerminalNode *> YulIdentifier();
    antlr4::tree::TerminalNode* YulIdentifier(size_t i);
    antlr4::tree::TerminalNode *YulLParen();
    antlr4::tree::TerminalNode *YulRParen();
    YulBlockContext *yulBlock();
    antlr4::tree::TerminalNode *YulArrow();
    std::vector<antlr4::tree::TerminalNode *> YulComma();
    antlr4::tree::TerminalNode* YulComma(size_t i);


    virtual antlrcpp::Any accept(antlr4::tree::ParseTreeVisitor *visitor) override;
   
  };

  YulFunctionDefinitionContext* yulFunctionDefinition();

  class  YulPathContext : public antlr4::ParserRuleContext {
  public:
    YulPathContext(antlr4::ParserRuleContext *parent, size_t invokingState);
    virtual size_t getRuleIndex() const override;
    std::vector<antlr4::tree::TerminalNode *> YulIdentifier();
    antlr4::tree::TerminalNode* YulIdentifier(size_t i);
    std::vector<antlr4::tree::TerminalNode *> YulPeriod();
    antlr4::tree::TerminalNode* YulPeriod(size_t i);


    virtual antlrcpp::Any accept(antlr4::tree::ParseTreeVisitor *visitor) override;
   
  };

  YulPathContext* yulPath();

  class  YulFunctionCallContext : public antlr4::ParserRuleContext {
  public:
    YulFunctionCallContext(antlr4::ParserRuleContext *parent, size_t invokingState);
    virtual size_t getRuleIndex() const override;
    antlr4::tree::TerminalNode *YulLParen();
    antlr4::tree::TerminalNode *YulRParen();
    antlr4::tree::TerminalNode *YulIdentifier();
    antlr4::tree::TerminalNode *YulEVMBuiltin();
    std::vector<YulExpressionContext *> yulExpression();
    YulExpressionContext* yulExpression(size_t i);
    std::vector<antlr4::tree::TerminalNode *> YulComma();
    antlr4::tree::TerminalNode* YulComma(size_t i);


    virtual antlrcpp::Any accept(antlr4::tree::ParseTreeVisitor *visitor) override;
   
  };

  YulFunctionCallContext* yulFunctionCall();

  class  YulboolContext : public antlr4::ParserRuleContext {
  public:
    YulboolContext(antlr4::ParserRuleContext *parent, size_t invokingState);
    virtual size_t getRuleIndex() const override;
    antlr4::tree::TerminalNode *YulTrue();
    antlr4::tree::TerminalNode *YulFalse();


    virtual antlrcpp::Any accept(antlr4::tree::ParseTreeVisitor *visitor) override;
   
  };

  YulboolContext* yulbool();

  class  YulLiteralContext : public antlr4::ParserRuleContext {
  public:
    YulLiteralContext(antlr4::ParserRuleContext *parent, size_t invokingState);
    virtual size_t getRuleIndex() const override;
    antlr4::tree::TerminalNode *YulDecimalNumber();
    antlr4::tree::TerminalNode *YulStringLiteral();
    antlr4::tree::TerminalNode *YulHexNumber();
    YulboolContext *yulbool();


    virtual antlrcpp::Any accept(antlr4::tree::ParseTreeVisitor *visitor) override;
   
  };

  YulLiteralContext* yulLiteral();

  class  YulExpressionContext : public antlr4::ParserRuleContext {
  public:
    YulExpressionContext(antlr4::ParserRuleContext *parent, size_t invokingState);
    virtual size_t getRuleIndex() const override;
    YulPathContext *yulPath();
    YulFunctionCallContext *yulFunctionCall();
    YulLiteralContext *yulLiteral();


    virtual antlrcpp::Any accept(antlr4::tree::ParseTreeVisitor *visitor) override;
   
  };

  YulExpressionContext* yulExpression();


  virtual bool sempred(antlr4::RuleContext *_localctx, size_t ruleIndex, size_t predicateIndex) override;
  bool constructorDefinitionSempred(ConstructorDefinitionContext *_localctx, size_t predicateIndex);
  bool functionDefinitionSempred(FunctionDefinitionContext *_localctx, size_t predicateIndex);
  bool modifierDefinitionSempred(ModifierDefinitionContext *_localctx, size_t predicateIndex);
  bool fallbackReceiveFunctionDefinitionSempred(FallbackReceiveFunctionDefinitionContext *_localctx, size_t predicateIndex);
  bool stateVariableDeclarationSempred(StateVariableDeclarationContext *_localctx, size_t predicateIndex);
  bool typeNameSempred(TypeNameContext *_localctx, size_t predicateIndex);
  bool elementaryTypeNameSempred(ElementaryTypeNameContext *_localctx, size_t predicateIndex);
  bool functionTypeNameSempred(FunctionTypeNameContext *_localctx, size_t predicateIndex);
  bool expressionSempred(ExpressionContext *_localctx, size_t predicateIndex);

private:
  static std::vector<antlr4::dfa::DFA> _decisionToDFA;
  static antlr4::atn::PredictionContextCache _sharedContextCache;
  static std::vector<std::string> _ruleNames;
  static std::vector<std::string> _tokenNames;

  static std::vector<std::string> _literalNames;
  static std::vector<std::string> _symbolicNames;
  static antlr4::dfa::Vocabulary _vocabulary;
  static antlr4::atn::ATN _atn;
  static std::vector<uint16_t> _serializedATN;


  struct Initializer {
    Initializer();
  };
  static Initializer _init;
};

