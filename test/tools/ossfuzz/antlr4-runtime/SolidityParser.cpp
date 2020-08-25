
// Generated from Solidity.g4 by ANTLR 4.8


#include <test/tools/ossfuzz/antlr4-runtime/SolidityVisitor.h>
#include <test/tools/ossfuzz/antlr4-runtime/SolidityParser.h>


using namespace antlrcpp;
using namespace antlr4;

SolidityParser::SolidityParser(TokenStream *input) : Parser(input) {
  _interpreter = new atn::ParserATNSimulator(this, _atn, _decisionToDFA, _sharedContextCache);
}

SolidityParser::~SolidityParser() {
  delete _interpreter;
}

std::string SolidityParser::getGrammarFileName() const {
  return "Solidity.g4";
}

const std::vector<std::string>& SolidityParser::getRuleNames() const {
  return _ruleNames;
}

dfa::Vocabulary& SolidityParser::getVocabulary() const {
  return _vocabulary;
}


//----------------- SourceUnitContext ------------------------------------------------------------------

SolidityParser::SourceUnitContext::SourceUnitContext(ParserRuleContext *parent, size_t invokingState)
  : ParserRuleContext(parent, invokingState) {
}

tree::TerminalNode* SolidityParser::SourceUnitContext::EOF() {
  return getToken(SolidityParser::EOF, 0);
}

std::vector<SolidityParser::PragmaDirectiveContext *> SolidityParser::SourceUnitContext::pragmaDirective() {
  return getRuleContexts<SolidityParser::PragmaDirectiveContext>();
}

SolidityParser::PragmaDirectiveContext* SolidityParser::SourceUnitContext::pragmaDirective(size_t i) {
  return getRuleContext<SolidityParser::PragmaDirectiveContext>(i);
}

std::vector<SolidityParser::ImportDirectiveContext *> SolidityParser::SourceUnitContext::importDirective() {
  return getRuleContexts<SolidityParser::ImportDirectiveContext>();
}

SolidityParser::ImportDirectiveContext* SolidityParser::SourceUnitContext::importDirective(size_t i) {
  return getRuleContext<SolidityParser::ImportDirectiveContext>(i);
}

std::vector<SolidityParser::ContractDefinitionContext *> SolidityParser::SourceUnitContext::contractDefinition() {
  return getRuleContexts<SolidityParser::ContractDefinitionContext>();
}

SolidityParser::ContractDefinitionContext* SolidityParser::SourceUnitContext::contractDefinition(size_t i) {
  return getRuleContext<SolidityParser::ContractDefinitionContext>(i);
}

std::vector<SolidityParser::InterfaceDefinitionContext *> SolidityParser::SourceUnitContext::interfaceDefinition() {
  return getRuleContexts<SolidityParser::InterfaceDefinitionContext>();
}

SolidityParser::InterfaceDefinitionContext* SolidityParser::SourceUnitContext::interfaceDefinition(size_t i) {
  return getRuleContext<SolidityParser::InterfaceDefinitionContext>(i);
}

std::vector<SolidityParser::LibraryDefinitionContext *> SolidityParser::SourceUnitContext::libraryDefinition() {
  return getRuleContexts<SolidityParser::LibraryDefinitionContext>();
}

SolidityParser::LibraryDefinitionContext* SolidityParser::SourceUnitContext::libraryDefinition(size_t i) {
  return getRuleContext<SolidityParser::LibraryDefinitionContext>(i);
}

std::vector<SolidityParser::FunctionDefinitionContext *> SolidityParser::SourceUnitContext::functionDefinition() {
  return getRuleContexts<SolidityParser::FunctionDefinitionContext>();
}

SolidityParser::FunctionDefinitionContext* SolidityParser::SourceUnitContext::functionDefinition(size_t i) {
  return getRuleContext<SolidityParser::FunctionDefinitionContext>(i);
}

std::vector<SolidityParser::StructDefinitionContext *> SolidityParser::SourceUnitContext::structDefinition() {
  return getRuleContexts<SolidityParser::StructDefinitionContext>();
}

SolidityParser::StructDefinitionContext* SolidityParser::SourceUnitContext::structDefinition(size_t i) {
  return getRuleContext<SolidityParser::StructDefinitionContext>(i);
}

std::vector<SolidityParser::EnumDefinitionContext *> SolidityParser::SourceUnitContext::enumDefinition() {
  return getRuleContexts<SolidityParser::EnumDefinitionContext>();
}

SolidityParser::EnumDefinitionContext* SolidityParser::SourceUnitContext::enumDefinition(size_t i) {
  return getRuleContext<SolidityParser::EnumDefinitionContext>(i);
}


size_t SolidityParser::SourceUnitContext::getRuleIndex() const {
  return SolidityParser::RuleSourceUnit;
}


antlrcpp::Any SolidityParser::SourceUnitContext::accept(tree::ParseTreeVisitor *visitor) {
  if (auto parserVisitor = dynamic_cast<SolidityVisitor*>(visitor))
    return parserVisitor->visitSourceUnit(this);
  else
    return visitor->visitChildren(this);
}

SolidityParser::SourceUnitContext* SolidityParser::sourceUnit() {
  SourceUnitContext *_localctx = _tracker.createInstance<SourceUnitContext>(_ctx, getState());
  enterRule(_localctx, 0, SolidityParser::RuleSourceUnit);
  size_t _la = 0;

  auto onExit = finally([=] {
    exitRule();
  });
  try {
    enterOuterAlt(_localctx, 1);
    setState(174);
    _errHandler->sync(this);
    _la = _input->LA(1);
    while ((((_la & ~ 0x3fULL) == 0) &&
      ((1ULL << _la) & ((1ULL << SolidityParser::Pragma)
      | (1ULL << SolidityParser::Abstract)
      | (1ULL << SolidityParser::Contract)
      | (1ULL << SolidityParser::Enum)
      | (1ULL << SolidityParser::Function)
      | (1ULL << SolidityParser::Import)
      | (1ULL << SolidityParser::Interface)
      | (1ULL << SolidityParser::Library)
      | (1ULL << SolidityParser::Struct))) != 0)) {
      setState(172);
      _errHandler->sync(this);
      switch (_input->LA(1)) {
        case SolidityParser::Pragma: {
          setState(164);
          pragmaDirective();
          break;
        }

        case SolidityParser::Import: {
          setState(165);
          importDirective();
          break;
        }

        case SolidityParser::Abstract:
        case SolidityParser::Contract: {
          setState(166);
          contractDefinition();
          break;
        }

        case SolidityParser::Interface: {
          setState(167);
          interfaceDefinition();
          break;
        }

        case SolidityParser::Library: {
          setState(168);
          libraryDefinition();
          break;
        }

        case SolidityParser::Function: {
          setState(169);
          functionDefinition();
          break;
        }

        case SolidityParser::Struct: {
          setState(170);
          structDefinition();
          break;
        }

        case SolidityParser::Enum: {
          setState(171);
          enumDefinition();
          break;
        }

      default:
        throw NoViableAltException(this);
      }
      setState(176);
      _errHandler->sync(this);
      _la = _input->LA(1);
    }
    setState(177);
    match(SolidityParser::EOF);
   
  }
  catch (RecognitionException &e) {
    _errHandler->reportError(this, e);
    _localctx->exception = std::current_exception();
    _errHandler->recover(this, _localctx->exception);
  }

  return _localctx;
}

//----------------- PragmaDirectiveContext ------------------------------------------------------------------

SolidityParser::PragmaDirectiveContext::PragmaDirectiveContext(ParserRuleContext *parent, size_t invokingState)
  : ParserRuleContext(parent, invokingState) {
}

tree::TerminalNode* SolidityParser::PragmaDirectiveContext::Pragma() {
  return getToken(SolidityParser::Pragma, 0);
}

tree::TerminalNode* SolidityParser::PragmaDirectiveContext::PragmaSemicolon() {
  return getToken(SolidityParser::PragmaSemicolon, 0);
}

std::vector<tree::TerminalNode *> SolidityParser::PragmaDirectiveContext::PragmaToken() {
  return getTokens(SolidityParser::PragmaToken);
}

tree::TerminalNode* SolidityParser::PragmaDirectiveContext::PragmaToken(size_t i) {
  return getToken(SolidityParser::PragmaToken, i);
}


size_t SolidityParser::PragmaDirectiveContext::getRuleIndex() const {
  return SolidityParser::RulePragmaDirective;
}


antlrcpp::Any SolidityParser::PragmaDirectiveContext::accept(tree::ParseTreeVisitor *visitor) {
  if (auto parserVisitor = dynamic_cast<SolidityVisitor*>(visitor))
    return parserVisitor->visitPragmaDirective(this);
  else
    return visitor->visitChildren(this);
}

SolidityParser::PragmaDirectiveContext* SolidityParser::pragmaDirective() {
  PragmaDirectiveContext *_localctx = _tracker.createInstance<PragmaDirectiveContext>(_ctx, getState());
  enterRule(_localctx, 2, SolidityParser::RulePragmaDirective);
  size_t _la = 0;

  auto onExit = finally([=] {
    exitRule();
  });
  try {
    enterOuterAlt(_localctx, 1);
    setState(179);
    match(SolidityParser::Pragma);
    setState(181); 
    _errHandler->sync(this);
    _la = _input->LA(1);
    do {
      setState(180);
      match(SolidityParser::PragmaToken);
      setState(183); 
      _errHandler->sync(this);
      _la = _input->LA(1);
    } while (_la == SolidityParser::PragmaToken);
    setState(185);
    match(SolidityParser::PragmaSemicolon);
   
  }
  catch (RecognitionException &e) {
    _errHandler->reportError(this, e);
    _localctx->exception = std::current_exception();
    _errHandler->recover(this, _localctx->exception);
  }

  return _localctx;
}

//----------------- ImportDirectiveContext ------------------------------------------------------------------

SolidityParser::ImportDirectiveContext::ImportDirectiveContext(ParserRuleContext *parent, size_t invokingState)
  : ParserRuleContext(parent, invokingState) {
}

tree::TerminalNode* SolidityParser::ImportDirectiveContext::Import() {
  return getToken(SolidityParser::Import, 0);
}

tree::TerminalNode* SolidityParser::ImportDirectiveContext::Semicolon() {
  return getToken(SolidityParser::Semicolon, 0);
}

SolidityParser::PathContext* SolidityParser::ImportDirectiveContext::path() {
  return getRuleContext<SolidityParser::PathContext>(0);
}

SolidityParser::SymbolAliasesContext* SolidityParser::ImportDirectiveContext::symbolAliases() {
  return getRuleContext<SolidityParser::SymbolAliasesContext>(0);
}

tree::TerminalNode* SolidityParser::ImportDirectiveContext::From() {
  return getToken(SolidityParser::From, 0);
}

tree::TerminalNode* SolidityParser::ImportDirectiveContext::Mul() {
  return getToken(SolidityParser::Mul, 0);
}

tree::TerminalNode* SolidityParser::ImportDirectiveContext::As() {
  return getToken(SolidityParser::As, 0);
}

SolidityParser::IdentifierContext* SolidityParser::ImportDirectiveContext::identifier() {
  return getRuleContext<SolidityParser::IdentifierContext>(0);
}


size_t SolidityParser::ImportDirectiveContext::getRuleIndex() const {
  return SolidityParser::RuleImportDirective;
}


antlrcpp::Any SolidityParser::ImportDirectiveContext::accept(tree::ParseTreeVisitor *visitor) {
  if (auto parserVisitor = dynamic_cast<SolidityVisitor*>(visitor))
    return parserVisitor->visitImportDirective(this);
  else
    return visitor->visitChildren(this);
}

SolidityParser::ImportDirectiveContext* SolidityParser::importDirective() {
  ImportDirectiveContext *_localctx = _tracker.createInstance<ImportDirectiveContext>(_ctx, getState());
  enterRule(_localctx, 4, SolidityParser::RuleImportDirective);
  size_t _la = 0;

  auto onExit = finally([=] {
    exitRule();
  });
  try {
    enterOuterAlt(_localctx, 1);
    setState(187);
    match(SolidityParser::Import);
    setState(203);
    _errHandler->sync(this);
    switch (_input->LA(1)) {
      case SolidityParser::NonEmptyStringLiteral: {
        setState(188);
        path();
        setState(191);
        _errHandler->sync(this);

        _la = _input->LA(1);
        if (_la == SolidityParser::As) {
          setState(189);
          match(SolidityParser::As);
          setState(190);
          dynamic_cast<ImportDirectiveContext *>(_localctx)->unitAlias = identifier();
        }
        break;
      }

      case SolidityParser::LBrace: {
        setState(193);
        symbolAliases();
        setState(194);
        match(SolidityParser::From);
        setState(195);
        path();
        break;
      }

      case SolidityParser::Mul: {
        setState(197);
        match(SolidityParser::Mul);
        setState(198);
        match(SolidityParser::As);
        setState(199);
        dynamic_cast<ImportDirectiveContext *>(_localctx)->unitAlias = identifier();
        setState(200);
        match(SolidityParser::From);
        setState(201);
        path();
        break;
      }

    default:
      throw NoViableAltException(this);
    }
    setState(205);
    match(SolidityParser::Semicolon);
   
  }
  catch (RecognitionException &e) {
    _errHandler->reportError(this, e);
    _localctx->exception = std::current_exception();
    _errHandler->recover(this, _localctx->exception);
  }

  return _localctx;
}

//----------------- ImportAliasesContext ------------------------------------------------------------------

SolidityParser::ImportAliasesContext::ImportAliasesContext(ParserRuleContext *parent, size_t invokingState)
  : ParserRuleContext(parent, invokingState) {
}

std::vector<SolidityParser::IdentifierContext *> SolidityParser::ImportAliasesContext::identifier() {
  return getRuleContexts<SolidityParser::IdentifierContext>();
}

SolidityParser::IdentifierContext* SolidityParser::ImportAliasesContext::identifier(size_t i) {
  return getRuleContext<SolidityParser::IdentifierContext>(i);
}

tree::TerminalNode* SolidityParser::ImportAliasesContext::As() {
  return getToken(SolidityParser::As, 0);
}


size_t SolidityParser::ImportAliasesContext::getRuleIndex() const {
  return SolidityParser::RuleImportAliases;
}


antlrcpp::Any SolidityParser::ImportAliasesContext::accept(tree::ParseTreeVisitor *visitor) {
  if (auto parserVisitor = dynamic_cast<SolidityVisitor*>(visitor))
    return parserVisitor->visitImportAliases(this);
  else
    return visitor->visitChildren(this);
}

SolidityParser::ImportAliasesContext* SolidityParser::importAliases() {
  ImportAliasesContext *_localctx = _tracker.createInstance<ImportAliasesContext>(_ctx, getState());
  enterRule(_localctx, 6, SolidityParser::RuleImportAliases);
  size_t _la = 0;

  auto onExit = finally([=] {
    exitRule();
  });
  try {
    enterOuterAlt(_localctx, 1);
    setState(207);
    dynamic_cast<ImportAliasesContext *>(_localctx)->symbol = identifier();
    setState(210);
    _errHandler->sync(this);

    _la = _input->LA(1);
    if (_la == SolidityParser::As) {
      setState(208);
      match(SolidityParser::As);
      setState(209);
      dynamic_cast<ImportAliasesContext *>(_localctx)->alias = identifier();
    }
   
  }
  catch (RecognitionException &e) {
    _errHandler->reportError(this, e);
    _localctx->exception = std::current_exception();
    _errHandler->recover(this, _localctx->exception);
  }

  return _localctx;
}

//----------------- PathContext ------------------------------------------------------------------

SolidityParser::PathContext::PathContext(ParserRuleContext *parent, size_t invokingState)
  : ParserRuleContext(parent, invokingState) {
}

tree::TerminalNode* SolidityParser::PathContext::NonEmptyStringLiteral() {
  return getToken(SolidityParser::NonEmptyStringLiteral, 0);
}


size_t SolidityParser::PathContext::getRuleIndex() const {
  return SolidityParser::RulePath;
}


antlrcpp::Any SolidityParser::PathContext::accept(tree::ParseTreeVisitor *visitor) {
  if (auto parserVisitor = dynamic_cast<SolidityVisitor*>(visitor))
    return parserVisitor->visitPath(this);
  else
    return visitor->visitChildren(this);
}

SolidityParser::PathContext* SolidityParser::path() {
  PathContext *_localctx = _tracker.createInstance<PathContext>(_ctx, getState());
  enterRule(_localctx, 8, SolidityParser::RulePath);

  auto onExit = finally([=] {
    exitRule();
  });
  try {
    enterOuterAlt(_localctx, 1);
    setState(212);
    match(SolidityParser::NonEmptyStringLiteral);
   
  }
  catch (RecognitionException &e) {
    _errHandler->reportError(this, e);
    _localctx->exception = std::current_exception();
    _errHandler->recover(this, _localctx->exception);
  }

  return _localctx;
}

//----------------- SymbolAliasesContext ------------------------------------------------------------------

SolidityParser::SymbolAliasesContext::SymbolAliasesContext(ParserRuleContext *parent, size_t invokingState)
  : ParserRuleContext(parent, invokingState) {
}

tree::TerminalNode* SolidityParser::SymbolAliasesContext::LBrace() {
  return getToken(SolidityParser::LBrace, 0);
}

tree::TerminalNode* SolidityParser::SymbolAliasesContext::RBrace() {
  return getToken(SolidityParser::RBrace, 0);
}

std::vector<SolidityParser::ImportAliasesContext *> SolidityParser::SymbolAliasesContext::importAliases() {
  return getRuleContexts<SolidityParser::ImportAliasesContext>();
}

SolidityParser::ImportAliasesContext* SolidityParser::SymbolAliasesContext::importAliases(size_t i) {
  return getRuleContext<SolidityParser::ImportAliasesContext>(i);
}

std::vector<tree::TerminalNode *> SolidityParser::SymbolAliasesContext::Comma() {
  return getTokens(SolidityParser::Comma);
}

tree::TerminalNode* SolidityParser::SymbolAliasesContext::Comma(size_t i) {
  return getToken(SolidityParser::Comma, i);
}


size_t SolidityParser::SymbolAliasesContext::getRuleIndex() const {
  return SolidityParser::RuleSymbolAliases;
}


antlrcpp::Any SolidityParser::SymbolAliasesContext::accept(tree::ParseTreeVisitor *visitor) {
  if (auto parserVisitor = dynamic_cast<SolidityVisitor*>(visitor))
    return parserVisitor->visitSymbolAliases(this);
  else
    return visitor->visitChildren(this);
}

SolidityParser::SymbolAliasesContext* SolidityParser::symbolAliases() {
  SymbolAliasesContext *_localctx = _tracker.createInstance<SymbolAliasesContext>(_ctx, getState());
  enterRule(_localctx, 10, SolidityParser::RuleSymbolAliases);
  size_t _la = 0;

  auto onExit = finally([=] {
    exitRule();
  });
  try {
    enterOuterAlt(_localctx, 1);
    setState(214);
    match(SolidityParser::LBrace);
    setState(215);
    dynamic_cast<SymbolAliasesContext *>(_localctx)->importAliasesContext = importAliases();
    dynamic_cast<SymbolAliasesContext *>(_localctx)->aliases.push_back(dynamic_cast<SymbolAliasesContext *>(_localctx)->importAliasesContext);
    setState(220);
    _errHandler->sync(this);
    _la = _input->LA(1);
    while (_la == SolidityParser::Comma) {
      setState(216);
      match(SolidityParser::Comma);
      setState(217);
      dynamic_cast<SymbolAliasesContext *>(_localctx)->importAliasesContext = importAliases();
      dynamic_cast<SymbolAliasesContext *>(_localctx)->aliases.push_back(dynamic_cast<SymbolAliasesContext *>(_localctx)->importAliasesContext);
      setState(222);
      _errHandler->sync(this);
      _la = _input->LA(1);
    }
    setState(223);
    match(SolidityParser::RBrace);
   
  }
  catch (RecognitionException &e) {
    _errHandler->reportError(this, e);
    _localctx->exception = std::current_exception();
    _errHandler->recover(this, _localctx->exception);
  }

  return _localctx;
}

//----------------- ContractDefinitionContext ------------------------------------------------------------------

SolidityParser::ContractDefinitionContext::ContractDefinitionContext(ParserRuleContext *parent, size_t invokingState)
  : ParserRuleContext(parent, invokingState) {
}

tree::TerminalNode* SolidityParser::ContractDefinitionContext::Contract() {
  return getToken(SolidityParser::Contract, 0);
}

tree::TerminalNode* SolidityParser::ContractDefinitionContext::LBrace() {
  return getToken(SolidityParser::LBrace, 0);
}

tree::TerminalNode* SolidityParser::ContractDefinitionContext::RBrace() {
  return getToken(SolidityParser::RBrace, 0);
}

SolidityParser::IdentifierContext* SolidityParser::ContractDefinitionContext::identifier() {
  return getRuleContext<SolidityParser::IdentifierContext>(0);
}

tree::TerminalNode* SolidityParser::ContractDefinitionContext::Abstract() {
  return getToken(SolidityParser::Abstract, 0);
}

SolidityParser::InheritanceSpecifierListContext* SolidityParser::ContractDefinitionContext::inheritanceSpecifierList() {
  return getRuleContext<SolidityParser::InheritanceSpecifierListContext>(0);
}

std::vector<SolidityParser::ContractBodyElementContext *> SolidityParser::ContractDefinitionContext::contractBodyElement() {
  return getRuleContexts<SolidityParser::ContractBodyElementContext>();
}

SolidityParser::ContractBodyElementContext* SolidityParser::ContractDefinitionContext::contractBodyElement(size_t i) {
  return getRuleContext<SolidityParser::ContractBodyElementContext>(i);
}


size_t SolidityParser::ContractDefinitionContext::getRuleIndex() const {
  return SolidityParser::RuleContractDefinition;
}


antlrcpp::Any SolidityParser::ContractDefinitionContext::accept(tree::ParseTreeVisitor *visitor) {
  if (auto parserVisitor = dynamic_cast<SolidityVisitor*>(visitor))
    return parserVisitor->visitContractDefinition(this);
  else
    return visitor->visitChildren(this);
}

SolidityParser::ContractDefinitionContext* SolidityParser::contractDefinition() {
  ContractDefinitionContext *_localctx = _tracker.createInstance<ContractDefinitionContext>(_ctx, getState());
  enterRule(_localctx, 12, SolidityParser::RuleContractDefinition);
  size_t _la = 0;

  auto onExit = finally([=] {
    exitRule();
  });
  try {
    size_t alt;
    enterOuterAlt(_localctx, 1);
    setState(226);
    _errHandler->sync(this);

    _la = _input->LA(1);
    if (_la == SolidityParser::Abstract) {
      setState(225);
      match(SolidityParser::Abstract);
    }
    setState(228);
    match(SolidityParser::Contract);
    setState(229);
    dynamic_cast<ContractDefinitionContext *>(_localctx)->name = identifier();
    setState(231);
    _errHandler->sync(this);

    _la = _input->LA(1);
    if (_la == SolidityParser::Is) {
      setState(230);
      inheritanceSpecifierList();
    }
    setState(233);
    match(SolidityParser::LBrace);
    setState(237);
    _errHandler->sync(this);
    alt = getInterpreter<atn::ParserATNSimulator>()->adaptivePredict(_input, 9, _ctx);
    while (alt != 2 && alt != atn::ATN::INVALID_ALT_NUMBER) {
      if (alt == 1) {
        setState(234);
        contractBodyElement(); 
      }
      setState(239);
      _errHandler->sync(this);
      alt = getInterpreter<atn::ParserATNSimulator>()->adaptivePredict(_input, 9, _ctx);
    }
    setState(240);
    match(SolidityParser::RBrace);
   
  }
  catch (RecognitionException &e) {
    _errHandler->reportError(this, e);
    _localctx->exception = std::current_exception();
    _errHandler->recover(this, _localctx->exception);
  }

  return _localctx;
}

//----------------- InterfaceDefinitionContext ------------------------------------------------------------------

SolidityParser::InterfaceDefinitionContext::InterfaceDefinitionContext(ParserRuleContext *parent, size_t invokingState)
  : ParserRuleContext(parent, invokingState) {
}

tree::TerminalNode* SolidityParser::InterfaceDefinitionContext::Interface() {
  return getToken(SolidityParser::Interface, 0);
}

tree::TerminalNode* SolidityParser::InterfaceDefinitionContext::LBrace() {
  return getToken(SolidityParser::LBrace, 0);
}

tree::TerminalNode* SolidityParser::InterfaceDefinitionContext::RBrace() {
  return getToken(SolidityParser::RBrace, 0);
}

SolidityParser::IdentifierContext* SolidityParser::InterfaceDefinitionContext::identifier() {
  return getRuleContext<SolidityParser::IdentifierContext>(0);
}

SolidityParser::InheritanceSpecifierListContext* SolidityParser::InterfaceDefinitionContext::inheritanceSpecifierList() {
  return getRuleContext<SolidityParser::InheritanceSpecifierListContext>(0);
}

std::vector<SolidityParser::ContractBodyElementContext *> SolidityParser::InterfaceDefinitionContext::contractBodyElement() {
  return getRuleContexts<SolidityParser::ContractBodyElementContext>();
}

SolidityParser::ContractBodyElementContext* SolidityParser::InterfaceDefinitionContext::contractBodyElement(size_t i) {
  return getRuleContext<SolidityParser::ContractBodyElementContext>(i);
}


size_t SolidityParser::InterfaceDefinitionContext::getRuleIndex() const {
  return SolidityParser::RuleInterfaceDefinition;
}


antlrcpp::Any SolidityParser::InterfaceDefinitionContext::accept(tree::ParseTreeVisitor *visitor) {
  if (auto parserVisitor = dynamic_cast<SolidityVisitor*>(visitor))
    return parserVisitor->visitInterfaceDefinition(this);
  else
    return visitor->visitChildren(this);
}

SolidityParser::InterfaceDefinitionContext* SolidityParser::interfaceDefinition() {
  InterfaceDefinitionContext *_localctx = _tracker.createInstance<InterfaceDefinitionContext>(_ctx, getState());
  enterRule(_localctx, 14, SolidityParser::RuleInterfaceDefinition);
  size_t _la = 0;

  auto onExit = finally([=] {
    exitRule();
  });
  try {
    size_t alt;
    enterOuterAlt(_localctx, 1);
    setState(242);
    match(SolidityParser::Interface);
    setState(243);
    dynamic_cast<InterfaceDefinitionContext *>(_localctx)->name = identifier();
    setState(245);
    _errHandler->sync(this);

    _la = _input->LA(1);
    if (_la == SolidityParser::Is) {
      setState(244);
      inheritanceSpecifierList();
    }
    setState(247);
    match(SolidityParser::LBrace);
    setState(251);
    _errHandler->sync(this);
    alt = getInterpreter<atn::ParserATNSimulator>()->adaptivePredict(_input, 11, _ctx);
    while (alt != 2 && alt != atn::ATN::INVALID_ALT_NUMBER) {
      if (alt == 1) {
        setState(248);
        contractBodyElement(); 
      }
      setState(253);
      _errHandler->sync(this);
      alt = getInterpreter<atn::ParserATNSimulator>()->adaptivePredict(_input, 11, _ctx);
    }
    setState(254);
    match(SolidityParser::RBrace);
   
  }
  catch (RecognitionException &e) {
    _errHandler->reportError(this, e);
    _localctx->exception = std::current_exception();
    _errHandler->recover(this, _localctx->exception);
  }

  return _localctx;
}

//----------------- LibraryDefinitionContext ------------------------------------------------------------------

SolidityParser::LibraryDefinitionContext::LibraryDefinitionContext(ParserRuleContext *parent, size_t invokingState)
  : ParserRuleContext(parent, invokingState) {
}

tree::TerminalNode* SolidityParser::LibraryDefinitionContext::Library() {
  return getToken(SolidityParser::Library, 0);
}

tree::TerminalNode* SolidityParser::LibraryDefinitionContext::LBrace() {
  return getToken(SolidityParser::LBrace, 0);
}

tree::TerminalNode* SolidityParser::LibraryDefinitionContext::RBrace() {
  return getToken(SolidityParser::RBrace, 0);
}

SolidityParser::IdentifierContext* SolidityParser::LibraryDefinitionContext::identifier() {
  return getRuleContext<SolidityParser::IdentifierContext>(0);
}

std::vector<SolidityParser::ContractBodyElementContext *> SolidityParser::LibraryDefinitionContext::contractBodyElement() {
  return getRuleContexts<SolidityParser::ContractBodyElementContext>();
}

SolidityParser::ContractBodyElementContext* SolidityParser::LibraryDefinitionContext::contractBodyElement(size_t i) {
  return getRuleContext<SolidityParser::ContractBodyElementContext>(i);
}


size_t SolidityParser::LibraryDefinitionContext::getRuleIndex() const {
  return SolidityParser::RuleLibraryDefinition;
}


antlrcpp::Any SolidityParser::LibraryDefinitionContext::accept(tree::ParseTreeVisitor *visitor) {
  if (auto parserVisitor = dynamic_cast<SolidityVisitor*>(visitor))
    return parserVisitor->visitLibraryDefinition(this);
  else
    return visitor->visitChildren(this);
}

SolidityParser::LibraryDefinitionContext* SolidityParser::libraryDefinition() {
  LibraryDefinitionContext *_localctx = _tracker.createInstance<LibraryDefinitionContext>(_ctx, getState());
  enterRule(_localctx, 16, SolidityParser::RuleLibraryDefinition);

  auto onExit = finally([=] {
    exitRule();
  });
  try {
    size_t alt;
    enterOuterAlt(_localctx, 1);
    setState(256);
    match(SolidityParser::Library);
    setState(257);
    dynamic_cast<LibraryDefinitionContext *>(_localctx)->name = identifier();
    setState(258);
    match(SolidityParser::LBrace);
    setState(262);
    _errHandler->sync(this);
    alt = getInterpreter<atn::ParserATNSimulator>()->adaptivePredict(_input, 12, _ctx);
    while (alt != 2 && alt != atn::ATN::INVALID_ALT_NUMBER) {
      if (alt == 1) {
        setState(259);
        contractBodyElement(); 
      }
      setState(264);
      _errHandler->sync(this);
      alt = getInterpreter<atn::ParserATNSimulator>()->adaptivePredict(_input, 12, _ctx);
    }
    setState(265);
    match(SolidityParser::RBrace);
   
  }
  catch (RecognitionException &e) {
    _errHandler->reportError(this, e);
    _localctx->exception = std::current_exception();
    _errHandler->recover(this, _localctx->exception);
  }

  return _localctx;
}

//----------------- InheritanceSpecifierListContext ------------------------------------------------------------------

SolidityParser::InheritanceSpecifierListContext::InheritanceSpecifierListContext(ParserRuleContext *parent, size_t invokingState)
  : ParserRuleContext(parent, invokingState) {
}

tree::TerminalNode* SolidityParser::InheritanceSpecifierListContext::Is() {
  return getToken(SolidityParser::Is, 0);
}

std::vector<SolidityParser::InheritanceSpecifierContext *> SolidityParser::InheritanceSpecifierListContext::inheritanceSpecifier() {
  return getRuleContexts<SolidityParser::InheritanceSpecifierContext>();
}

SolidityParser::InheritanceSpecifierContext* SolidityParser::InheritanceSpecifierListContext::inheritanceSpecifier(size_t i) {
  return getRuleContext<SolidityParser::InheritanceSpecifierContext>(i);
}

std::vector<tree::TerminalNode *> SolidityParser::InheritanceSpecifierListContext::Comma() {
  return getTokens(SolidityParser::Comma);
}

tree::TerminalNode* SolidityParser::InheritanceSpecifierListContext::Comma(size_t i) {
  return getToken(SolidityParser::Comma, i);
}


size_t SolidityParser::InheritanceSpecifierListContext::getRuleIndex() const {
  return SolidityParser::RuleInheritanceSpecifierList;
}


antlrcpp::Any SolidityParser::InheritanceSpecifierListContext::accept(tree::ParseTreeVisitor *visitor) {
  if (auto parserVisitor = dynamic_cast<SolidityVisitor*>(visitor))
    return parserVisitor->visitInheritanceSpecifierList(this);
  else
    return visitor->visitChildren(this);
}

SolidityParser::InheritanceSpecifierListContext* SolidityParser::inheritanceSpecifierList() {
  InheritanceSpecifierListContext *_localctx = _tracker.createInstance<InheritanceSpecifierListContext>(_ctx, getState());
  enterRule(_localctx, 18, SolidityParser::RuleInheritanceSpecifierList);

  auto onExit = finally([=] {
    exitRule();
  });
  try {
    size_t alt;
    enterOuterAlt(_localctx, 1);
    setState(267);
    match(SolidityParser::Is);
    setState(268);
    dynamic_cast<InheritanceSpecifierListContext *>(_localctx)->inheritanceSpecifierContext = inheritanceSpecifier();
    dynamic_cast<InheritanceSpecifierListContext *>(_localctx)->inheritanceSpecifiers.push_back(dynamic_cast<InheritanceSpecifierListContext *>(_localctx)->inheritanceSpecifierContext);
    setState(273);
    _errHandler->sync(this);
    alt = getInterpreter<atn::ParserATNSimulator>()->adaptivePredict(_input, 13, _ctx);
    while (alt != 1 && alt != atn::ATN::INVALID_ALT_NUMBER) {
      if (alt == 1 + 1) {
        setState(269);
        match(SolidityParser::Comma);
        setState(270);
        dynamic_cast<InheritanceSpecifierListContext *>(_localctx)->inheritanceSpecifierContext = inheritanceSpecifier();
        dynamic_cast<InheritanceSpecifierListContext *>(_localctx)->inheritanceSpecifiers.push_back(dynamic_cast<InheritanceSpecifierListContext *>(_localctx)->inheritanceSpecifierContext); 
      }
      setState(275);
      _errHandler->sync(this);
      alt = getInterpreter<atn::ParserATNSimulator>()->adaptivePredict(_input, 13, _ctx);
    }
   
  }
  catch (RecognitionException &e) {
    _errHandler->reportError(this, e);
    _localctx->exception = std::current_exception();
    _errHandler->recover(this, _localctx->exception);
  }

  return _localctx;
}

//----------------- InheritanceSpecifierContext ------------------------------------------------------------------

SolidityParser::InheritanceSpecifierContext::InheritanceSpecifierContext(ParserRuleContext *parent, size_t invokingState)
  : ParserRuleContext(parent, invokingState) {
}

SolidityParser::UserDefinedTypeNameContext* SolidityParser::InheritanceSpecifierContext::userDefinedTypeName() {
  return getRuleContext<SolidityParser::UserDefinedTypeNameContext>(0);
}

SolidityParser::CallArgumentListContext* SolidityParser::InheritanceSpecifierContext::callArgumentList() {
  return getRuleContext<SolidityParser::CallArgumentListContext>(0);
}


size_t SolidityParser::InheritanceSpecifierContext::getRuleIndex() const {
  return SolidityParser::RuleInheritanceSpecifier;
}


antlrcpp::Any SolidityParser::InheritanceSpecifierContext::accept(tree::ParseTreeVisitor *visitor) {
  if (auto parserVisitor = dynamic_cast<SolidityVisitor*>(visitor))
    return parserVisitor->visitInheritanceSpecifier(this);
  else
    return visitor->visitChildren(this);
}

SolidityParser::InheritanceSpecifierContext* SolidityParser::inheritanceSpecifier() {
  InheritanceSpecifierContext *_localctx = _tracker.createInstance<InheritanceSpecifierContext>(_ctx, getState());
  enterRule(_localctx, 20, SolidityParser::RuleInheritanceSpecifier);
  size_t _la = 0;

  auto onExit = finally([=] {
    exitRule();
  });
  try {
    enterOuterAlt(_localctx, 1);
    setState(276);
    dynamic_cast<InheritanceSpecifierContext *>(_localctx)->name = userDefinedTypeName();
    setState(278);
    _errHandler->sync(this);

    _la = _input->LA(1);
    if (_la == SolidityParser::LParen) {
      setState(277);
      dynamic_cast<InheritanceSpecifierContext *>(_localctx)->arguments = callArgumentList();
    }
   
  }
  catch (RecognitionException &e) {
    _errHandler->reportError(this, e);
    _localctx->exception = std::current_exception();
    _errHandler->recover(this, _localctx->exception);
  }

  return _localctx;
}

//----------------- ContractBodyElementContext ------------------------------------------------------------------

SolidityParser::ContractBodyElementContext::ContractBodyElementContext(ParserRuleContext *parent, size_t invokingState)
  : ParserRuleContext(parent, invokingState) {
}

SolidityParser::ConstructorDefinitionContext* SolidityParser::ContractBodyElementContext::constructorDefinition() {
  return getRuleContext<SolidityParser::ConstructorDefinitionContext>(0);
}

SolidityParser::FunctionDefinitionContext* SolidityParser::ContractBodyElementContext::functionDefinition() {
  return getRuleContext<SolidityParser::FunctionDefinitionContext>(0);
}

SolidityParser::ModifierDefinitionContext* SolidityParser::ContractBodyElementContext::modifierDefinition() {
  return getRuleContext<SolidityParser::ModifierDefinitionContext>(0);
}

SolidityParser::FallbackReceiveFunctionDefinitionContext* SolidityParser::ContractBodyElementContext::fallbackReceiveFunctionDefinition() {
  return getRuleContext<SolidityParser::FallbackReceiveFunctionDefinitionContext>(0);
}

SolidityParser::StructDefinitionContext* SolidityParser::ContractBodyElementContext::structDefinition() {
  return getRuleContext<SolidityParser::StructDefinitionContext>(0);
}

SolidityParser::EnumDefinitionContext* SolidityParser::ContractBodyElementContext::enumDefinition() {
  return getRuleContext<SolidityParser::EnumDefinitionContext>(0);
}

SolidityParser::StateVariableDeclarationContext* SolidityParser::ContractBodyElementContext::stateVariableDeclaration() {
  return getRuleContext<SolidityParser::StateVariableDeclarationContext>(0);
}

SolidityParser::EventDefinitionContext* SolidityParser::ContractBodyElementContext::eventDefinition() {
  return getRuleContext<SolidityParser::EventDefinitionContext>(0);
}

SolidityParser::UsingDirectiveContext* SolidityParser::ContractBodyElementContext::usingDirective() {
  return getRuleContext<SolidityParser::UsingDirectiveContext>(0);
}


size_t SolidityParser::ContractBodyElementContext::getRuleIndex() const {
  return SolidityParser::RuleContractBodyElement;
}


antlrcpp::Any SolidityParser::ContractBodyElementContext::accept(tree::ParseTreeVisitor *visitor) {
  if (auto parserVisitor = dynamic_cast<SolidityVisitor*>(visitor))
    return parserVisitor->visitContractBodyElement(this);
  else
    return visitor->visitChildren(this);
}

SolidityParser::ContractBodyElementContext* SolidityParser::contractBodyElement() {
  ContractBodyElementContext *_localctx = _tracker.createInstance<ContractBodyElementContext>(_ctx, getState());
  enterRule(_localctx, 22, SolidityParser::RuleContractBodyElement);

  auto onExit = finally([=] {
    exitRule();
  });
  try {
    setState(289);
    _errHandler->sync(this);
    switch (getInterpreter<atn::ParserATNSimulator>()->adaptivePredict(_input, 15, _ctx)) {
    case 1: {
      enterOuterAlt(_localctx, 1);
      setState(280);
      constructorDefinition();
      break;
    }

    case 2: {
      enterOuterAlt(_localctx, 2);
      setState(281);
      functionDefinition();
      break;
    }

    case 3: {
      enterOuterAlt(_localctx, 3);
      setState(282);
      modifierDefinition();
      break;
    }

    case 4: {
      enterOuterAlt(_localctx, 4);
      setState(283);
      fallbackReceiveFunctionDefinition();
      break;
    }

    case 5: {
      enterOuterAlt(_localctx, 5);
      setState(284);
      structDefinition();
      break;
    }

    case 6: {
      enterOuterAlt(_localctx, 6);
      setState(285);
      enumDefinition();
      break;
    }

    case 7: {
      enterOuterAlt(_localctx, 7);
      setState(286);
      stateVariableDeclaration();
      break;
    }

    case 8: {
      enterOuterAlt(_localctx, 8);
      setState(287);
      eventDefinition();
      break;
    }

    case 9: {
      enterOuterAlt(_localctx, 9);
      setState(288);
      usingDirective();
      break;
    }

    }
   
  }
  catch (RecognitionException &e) {
    _errHandler->reportError(this, e);
    _localctx->exception = std::current_exception();
    _errHandler->recover(this, _localctx->exception);
  }

  return _localctx;
}

//----------------- NamedArgumentContext ------------------------------------------------------------------

SolidityParser::NamedArgumentContext::NamedArgumentContext(ParserRuleContext *parent, size_t invokingState)
  : ParserRuleContext(parent, invokingState) {
}

tree::TerminalNode* SolidityParser::NamedArgumentContext::Colon() {
  return getToken(SolidityParser::Colon, 0);
}

SolidityParser::IdentifierContext* SolidityParser::NamedArgumentContext::identifier() {
  return getRuleContext<SolidityParser::IdentifierContext>(0);
}

SolidityParser::ExpressionContext* SolidityParser::NamedArgumentContext::expression() {
  return getRuleContext<SolidityParser::ExpressionContext>(0);
}


size_t SolidityParser::NamedArgumentContext::getRuleIndex() const {
  return SolidityParser::RuleNamedArgument;
}


antlrcpp::Any SolidityParser::NamedArgumentContext::accept(tree::ParseTreeVisitor *visitor) {
  if (auto parserVisitor = dynamic_cast<SolidityVisitor*>(visitor))
    return parserVisitor->visitNamedArgument(this);
  else
    return visitor->visitChildren(this);
}

SolidityParser::NamedArgumentContext* SolidityParser::namedArgument() {
  NamedArgumentContext *_localctx = _tracker.createInstance<NamedArgumentContext>(_ctx, getState());
  enterRule(_localctx, 24, SolidityParser::RuleNamedArgument);

  auto onExit = finally([=] {
    exitRule();
  });
  try {
    enterOuterAlt(_localctx, 1);
    setState(291);
    dynamic_cast<NamedArgumentContext *>(_localctx)->name = identifier();
    setState(292);
    match(SolidityParser::Colon);
    setState(293);
    dynamic_cast<NamedArgumentContext *>(_localctx)->value = expression(0);
   
  }
  catch (RecognitionException &e) {
    _errHandler->reportError(this, e);
    _localctx->exception = std::current_exception();
    _errHandler->recover(this, _localctx->exception);
  }

  return _localctx;
}

//----------------- CallArgumentListContext ------------------------------------------------------------------

SolidityParser::CallArgumentListContext::CallArgumentListContext(ParserRuleContext *parent, size_t invokingState)
  : ParserRuleContext(parent, invokingState) {
}

tree::TerminalNode* SolidityParser::CallArgumentListContext::LParen() {
  return getToken(SolidityParser::LParen, 0);
}

tree::TerminalNode* SolidityParser::CallArgumentListContext::RParen() {
  return getToken(SolidityParser::RParen, 0);
}

tree::TerminalNode* SolidityParser::CallArgumentListContext::LBrace() {
  return getToken(SolidityParser::LBrace, 0);
}

tree::TerminalNode* SolidityParser::CallArgumentListContext::RBrace() {
  return getToken(SolidityParser::RBrace, 0);
}

std::vector<SolidityParser::ExpressionContext *> SolidityParser::CallArgumentListContext::expression() {
  return getRuleContexts<SolidityParser::ExpressionContext>();
}

SolidityParser::ExpressionContext* SolidityParser::CallArgumentListContext::expression(size_t i) {
  return getRuleContext<SolidityParser::ExpressionContext>(i);
}

std::vector<SolidityParser::NamedArgumentContext *> SolidityParser::CallArgumentListContext::namedArgument() {
  return getRuleContexts<SolidityParser::NamedArgumentContext>();
}

SolidityParser::NamedArgumentContext* SolidityParser::CallArgumentListContext::namedArgument(size_t i) {
  return getRuleContext<SolidityParser::NamedArgumentContext>(i);
}

std::vector<tree::TerminalNode *> SolidityParser::CallArgumentListContext::Comma() {
  return getTokens(SolidityParser::Comma);
}

tree::TerminalNode* SolidityParser::CallArgumentListContext::Comma(size_t i) {
  return getToken(SolidityParser::Comma, i);
}


size_t SolidityParser::CallArgumentListContext::getRuleIndex() const {
  return SolidityParser::RuleCallArgumentList;
}


antlrcpp::Any SolidityParser::CallArgumentListContext::accept(tree::ParseTreeVisitor *visitor) {
  if (auto parserVisitor = dynamic_cast<SolidityVisitor*>(visitor))
    return parserVisitor->visitCallArgumentList(this);
  else
    return visitor->visitChildren(this);
}

SolidityParser::CallArgumentListContext* SolidityParser::callArgumentList() {
  CallArgumentListContext *_localctx = _tracker.createInstance<CallArgumentListContext>(_ctx, getState());
  enterRule(_localctx, 26, SolidityParser::RuleCallArgumentList);
  size_t _la = 0;

  auto onExit = finally([=] {
    exitRule();
  });
  try {
    enterOuterAlt(_localctx, 1);
    setState(295);
    match(SolidityParser::LParen);
    setState(318);
    _errHandler->sync(this);
    switch (getInterpreter<atn::ParserATNSimulator>()->adaptivePredict(_input, 20, _ctx)) {
    case 1: {
      setState(304);
      _errHandler->sync(this);

      switch (getInterpreter<atn::ParserATNSimulator>()->adaptivePredict(_input, 17, _ctx)) {
      case 1: {
        setState(296);
        expression(0);
        setState(301);
        _errHandler->sync(this);
        _la = _input->LA(1);
        while (_la == SolidityParser::Comma) {
          setState(297);
          match(SolidityParser::Comma);
          setState(298);
          expression(0);
          setState(303);
          _errHandler->sync(this);
          _la = _input->LA(1);
        }
        break;
      }

      }
      break;
    }

    case 2: {
      setState(306);
      match(SolidityParser::LBrace);
      setState(315);
      _errHandler->sync(this);

      _la = _input->LA(1);
      if (_la == SolidityParser::From || _la == SolidityParser::Identifier) {
        setState(307);
        namedArgument();
        setState(312);
        _errHandler->sync(this);
        _la = _input->LA(1);
        while (_la == SolidityParser::Comma) {
          setState(308);
          match(SolidityParser::Comma);
          setState(309);
          namedArgument();
          setState(314);
          _errHandler->sync(this);
          _la = _input->LA(1);
        }
      }
      setState(317);
      match(SolidityParser::RBrace);
      break;
    }

    }
    setState(320);
    match(SolidityParser::RParen);
   
  }
  catch (RecognitionException &e) {
    _errHandler->reportError(this, e);
    _localctx->exception = std::current_exception();
    _errHandler->recover(this, _localctx->exception);
  }

  return _localctx;
}

//----------------- UserDefinedTypeNameContext ------------------------------------------------------------------

SolidityParser::UserDefinedTypeNameContext::UserDefinedTypeNameContext(ParserRuleContext *parent, size_t invokingState)
  : ParserRuleContext(parent, invokingState) {
}

std::vector<SolidityParser::IdentifierContext *> SolidityParser::UserDefinedTypeNameContext::identifier() {
  return getRuleContexts<SolidityParser::IdentifierContext>();
}

SolidityParser::IdentifierContext* SolidityParser::UserDefinedTypeNameContext::identifier(size_t i) {
  return getRuleContext<SolidityParser::IdentifierContext>(i);
}

std::vector<tree::TerminalNode *> SolidityParser::UserDefinedTypeNameContext::Period() {
  return getTokens(SolidityParser::Period);
}

tree::TerminalNode* SolidityParser::UserDefinedTypeNameContext::Period(size_t i) {
  return getToken(SolidityParser::Period, i);
}


size_t SolidityParser::UserDefinedTypeNameContext::getRuleIndex() const {
  return SolidityParser::RuleUserDefinedTypeName;
}


antlrcpp::Any SolidityParser::UserDefinedTypeNameContext::accept(tree::ParseTreeVisitor *visitor) {
  if (auto parserVisitor = dynamic_cast<SolidityVisitor*>(visitor))
    return parserVisitor->visitUserDefinedTypeName(this);
  else
    return visitor->visitChildren(this);
}

SolidityParser::UserDefinedTypeNameContext* SolidityParser::userDefinedTypeName() {
  UserDefinedTypeNameContext *_localctx = _tracker.createInstance<UserDefinedTypeNameContext>(_ctx, getState());
  enterRule(_localctx, 28, SolidityParser::RuleUserDefinedTypeName);

  auto onExit = finally([=] {
    exitRule();
  });
  try {
    size_t alt;
    enterOuterAlt(_localctx, 1);
    setState(322);
    identifier();
    setState(327);
    _errHandler->sync(this);
    alt = getInterpreter<atn::ParserATNSimulator>()->adaptivePredict(_input, 21, _ctx);
    while (alt != 2 && alt != atn::ATN::INVALID_ALT_NUMBER) {
      if (alt == 1) {
        setState(323);
        match(SolidityParser::Period);
        setState(324);
        identifier(); 
      }
      setState(329);
      _errHandler->sync(this);
      alt = getInterpreter<atn::ParserATNSimulator>()->adaptivePredict(_input, 21, _ctx);
    }
   
  }
  catch (RecognitionException &e) {
    _errHandler->reportError(this, e);
    _localctx->exception = std::current_exception();
    _errHandler->recover(this, _localctx->exception);
  }

  return _localctx;
}

//----------------- ModifierInvocationContext ------------------------------------------------------------------

SolidityParser::ModifierInvocationContext::ModifierInvocationContext(ParserRuleContext *parent, size_t invokingState)
  : ParserRuleContext(parent, invokingState) {
}

SolidityParser::IdentifierContext* SolidityParser::ModifierInvocationContext::identifier() {
  return getRuleContext<SolidityParser::IdentifierContext>(0);
}

SolidityParser::CallArgumentListContext* SolidityParser::ModifierInvocationContext::callArgumentList() {
  return getRuleContext<SolidityParser::CallArgumentListContext>(0);
}


size_t SolidityParser::ModifierInvocationContext::getRuleIndex() const {
  return SolidityParser::RuleModifierInvocation;
}


antlrcpp::Any SolidityParser::ModifierInvocationContext::accept(tree::ParseTreeVisitor *visitor) {
  if (auto parserVisitor = dynamic_cast<SolidityVisitor*>(visitor))
    return parserVisitor->visitModifierInvocation(this);
  else
    return visitor->visitChildren(this);
}

SolidityParser::ModifierInvocationContext* SolidityParser::modifierInvocation() {
  ModifierInvocationContext *_localctx = _tracker.createInstance<ModifierInvocationContext>(_ctx, getState());
  enterRule(_localctx, 30, SolidityParser::RuleModifierInvocation);

  auto onExit = finally([=] {
    exitRule();
  });
  try {
    enterOuterAlt(_localctx, 1);
    setState(330);
    identifier();
    setState(332);
    _errHandler->sync(this);

    switch (getInterpreter<atn::ParserATNSimulator>()->adaptivePredict(_input, 22, _ctx)) {
    case 1: {
      setState(331);
      callArgumentList();
      break;
    }

    }
   
  }
  catch (RecognitionException &e) {
    _errHandler->reportError(this, e);
    _localctx->exception = std::current_exception();
    _errHandler->recover(this, _localctx->exception);
  }

  return _localctx;
}

//----------------- VisibilityContext ------------------------------------------------------------------

SolidityParser::VisibilityContext::VisibilityContext(ParserRuleContext *parent, size_t invokingState)
  : ParserRuleContext(parent, invokingState) {
}

tree::TerminalNode* SolidityParser::VisibilityContext::Internal() {
  return getToken(SolidityParser::Internal, 0);
}

tree::TerminalNode* SolidityParser::VisibilityContext::External() {
  return getToken(SolidityParser::External, 0);
}

tree::TerminalNode* SolidityParser::VisibilityContext::Private() {
  return getToken(SolidityParser::Private, 0);
}

tree::TerminalNode* SolidityParser::VisibilityContext::Public() {
  return getToken(SolidityParser::Public, 0);
}


size_t SolidityParser::VisibilityContext::getRuleIndex() const {
  return SolidityParser::RuleVisibility;
}


antlrcpp::Any SolidityParser::VisibilityContext::accept(tree::ParseTreeVisitor *visitor) {
  if (auto parserVisitor = dynamic_cast<SolidityVisitor*>(visitor))
    return parserVisitor->visitVisibility(this);
  else
    return visitor->visitChildren(this);
}

SolidityParser::VisibilityContext* SolidityParser::visibility() {
  VisibilityContext *_localctx = _tracker.createInstance<VisibilityContext>(_ctx, getState());
  enterRule(_localctx, 32, SolidityParser::RuleVisibility);
  size_t _la = 0;

  auto onExit = finally([=] {
    exitRule();
  });
  try {
    enterOuterAlt(_localctx, 1);
    setState(334);
    _la = _input->LA(1);
    if (!((((_la & ~ 0x3fULL) == 0) &&
      ((1ULL << _la) & ((1ULL << SolidityParser::External)
      | (1ULL << SolidityParser::Internal)
      | (1ULL << SolidityParser::Private)
      | (1ULL << SolidityParser::Public))) != 0))) {
    _errHandler->recoverInline(this);
    }
    else {
      _errHandler->reportMatch(this);
      consume();
    }
   
  }
  catch (RecognitionException &e) {
    _errHandler->reportError(this, e);
    _localctx->exception = std::current_exception();
    _errHandler->recover(this, _localctx->exception);
  }

  return _localctx;
}

//----------------- ParameterListContext ------------------------------------------------------------------

SolidityParser::ParameterListContext::ParameterListContext(ParserRuleContext *parent, size_t invokingState)
  : ParserRuleContext(parent, invokingState) {
}

std::vector<SolidityParser::ParameterDeclarationContext *> SolidityParser::ParameterListContext::parameterDeclaration() {
  return getRuleContexts<SolidityParser::ParameterDeclarationContext>();
}

SolidityParser::ParameterDeclarationContext* SolidityParser::ParameterListContext::parameterDeclaration(size_t i) {
  return getRuleContext<SolidityParser::ParameterDeclarationContext>(i);
}

std::vector<tree::TerminalNode *> SolidityParser::ParameterListContext::Comma() {
  return getTokens(SolidityParser::Comma);
}

tree::TerminalNode* SolidityParser::ParameterListContext::Comma(size_t i) {
  return getToken(SolidityParser::Comma, i);
}


size_t SolidityParser::ParameterListContext::getRuleIndex() const {
  return SolidityParser::RuleParameterList;
}


antlrcpp::Any SolidityParser::ParameterListContext::accept(tree::ParseTreeVisitor *visitor) {
  if (auto parserVisitor = dynamic_cast<SolidityVisitor*>(visitor))
    return parserVisitor->visitParameterList(this);
  else
    return visitor->visitChildren(this);
}

SolidityParser::ParameterListContext* SolidityParser::parameterList() {
  ParameterListContext *_localctx = _tracker.createInstance<ParameterListContext>(_ctx, getState());
  enterRule(_localctx, 34, SolidityParser::RuleParameterList);
  size_t _la = 0;

  auto onExit = finally([=] {
    exitRule();
  });
  try {
    enterOuterAlt(_localctx, 1);
    setState(336);
    dynamic_cast<ParameterListContext *>(_localctx)->parameterDeclarationContext = parameterDeclaration();
    dynamic_cast<ParameterListContext *>(_localctx)->parameters.push_back(dynamic_cast<ParameterListContext *>(_localctx)->parameterDeclarationContext);
    setState(341);
    _errHandler->sync(this);
    _la = _input->LA(1);
    while (_la == SolidityParser::Comma) {
      setState(337);
      match(SolidityParser::Comma);
      setState(338);
      dynamic_cast<ParameterListContext *>(_localctx)->parameterDeclarationContext = parameterDeclaration();
      dynamic_cast<ParameterListContext *>(_localctx)->parameters.push_back(dynamic_cast<ParameterListContext *>(_localctx)->parameterDeclarationContext);
      setState(343);
      _errHandler->sync(this);
      _la = _input->LA(1);
    }
   
  }
  catch (RecognitionException &e) {
    _errHandler->reportError(this, e);
    _localctx->exception = std::current_exception();
    _errHandler->recover(this, _localctx->exception);
  }

  return _localctx;
}

//----------------- ParameterDeclarationContext ------------------------------------------------------------------

SolidityParser::ParameterDeclarationContext::ParameterDeclarationContext(ParserRuleContext *parent, size_t invokingState)
  : ParserRuleContext(parent, invokingState) {
}

SolidityParser::TypeNameContext* SolidityParser::ParameterDeclarationContext::typeName() {
  return getRuleContext<SolidityParser::TypeNameContext>(0);
}

SolidityParser::DataLocationContext* SolidityParser::ParameterDeclarationContext::dataLocation() {
  return getRuleContext<SolidityParser::DataLocationContext>(0);
}

SolidityParser::IdentifierContext* SolidityParser::ParameterDeclarationContext::identifier() {
  return getRuleContext<SolidityParser::IdentifierContext>(0);
}


size_t SolidityParser::ParameterDeclarationContext::getRuleIndex() const {
  return SolidityParser::RuleParameterDeclaration;
}


antlrcpp::Any SolidityParser::ParameterDeclarationContext::accept(tree::ParseTreeVisitor *visitor) {
  if (auto parserVisitor = dynamic_cast<SolidityVisitor*>(visitor))
    return parserVisitor->visitParameterDeclaration(this);
  else
    return visitor->visitChildren(this);
}

SolidityParser::ParameterDeclarationContext* SolidityParser::parameterDeclaration() {
  ParameterDeclarationContext *_localctx = _tracker.createInstance<ParameterDeclarationContext>(_ctx, getState());
  enterRule(_localctx, 36, SolidityParser::RuleParameterDeclaration);
  size_t _la = 0;

  auto onExit = finally([=] {
    exitRule();
  });
  try {
    enterOuterAlt(_localctx, 1);
    setState(344);
    dynamic_cast<ParameterDeclarationContext *>(_localctx)->type = typeName(0);
    setState(346);
    _errHandler->sync(this);

    _la = _input->LA(1);
    if ((((_la & ~ 0x3fULL) == 0) &&
      ((1ULL << _la) & ((1ULL << SolidityParser::Calldata)
      | (1ULL << SolidityParser::Memory)
      | (1ULL << SolidityParser::Storage))) != 0)) {
      setState(345);
      dynamic_cast<ParameterDeclarationContext *>(_localctx)->location = dataLocation();
    }
    setState(349);
    _errHandler->sync(this);

    _la = _input->LA(1);
    if (_la == SolidityParser::From || _la == SolidityParser::Identifier) {
      setState(348);
      dynamic_cast<ParameterDeclarationContext *>(_localctx)->name = identifier();
    }
   
  }
  catch (RecognitionException &e) {
    _errHandler->reportError(this, e);
    _localctx->exception = std::current_exception();
    _errHandler->recover(this, _localctx->exception);
  }

  return _localctx;
}

//----------------- ConstructorDefinitionContext ------------------------------------------------------------------

SolidityParser::ConstructorDefinitionContext::ConstructorDefinitionContext(ParserRuleContext *parent, size_t invokingState)
  : ParserRuleContext(parent, invokingState) {
}

tree::TerminalNode* SolidityParser::ConstructorDefinitionContext::Constructor() {
  return getToken(SolidityParser::Constructor, 0);
}

tree::TerminalNode* SolidityParser::ConstructorDefinitionContext::LParen() {
  return getToken(SolidityParser::LParen, 0);
}

tree::TerminalNode* SolidityParser::ConstructorDefinitionContext::RParen() {
  return getToken(SolidityParser::RParen, 0);
}

SolidityParser::BlockContext* SolidityParser::ConstructorDefinitionContext::block() {
  return getRuleContext<SolidityParser::BlockContext>(0);
}

std::vector<SolidityParser::ModifierInvocationContext *> SolidityParser::ConstructorDefinitionContext::modifierInvocation() {
  return getRuleContexts<SolidityParser::ModifierInvocationContext>();
}

SolidityParser::ModifierInvocationContext* SolidityParser::ConstructorDefinitionContext::modifierInvocation(size_t i) {
  return getRuleContext<SolidityParser::ModifierInvocationContext>(i);
}

std::vector<tree::TerminalNode *> SolidityParser::ConstructorDefinitionContext::Payable() {
  return getTokens(SolidityParser::Payable);
}

tree::TerminalNode* SolidityParser::ConstructorDefinitionContext::Payable(size_t i) {
  return getToken(SolidityParser::Payable, i);
}

std::vector<tree::TerminalNode *> SolidityParser::ConstructorDefinitionContext::Internal() {
  return getTokens(SolidityParser::Internal);
}

tree::TerminalNode* SolidityParser::ConstructorDefinitionContext::Internal(size_t i) {
  return getToken(SolidityParser::Internal, i);
}

std::vector<tree::TerminalNode *> SolidityParser::ConstructorDefinitionContext::Public() {
  return getTokens(SolidityParser::Public);
}

tree::TerminalNode* SolidityParser::ConstructorDefinitionContext::Public(size_t i) {
  return getToken(SolidityParser::Public, i);
}

SolidityParser::ParameterListContext* SolidityParser::ConstructorDefinitionContext::parameterList() {
  return getRuleContext<SolidityParser::ParameterListContext>(0);
}


size_t SolidityParser::ConstructorDefinitionContext::getRuleIndex() const {
  return SolidityParser::RuleConstructorDefinition;
}


antlrcpp::Any SolidityParser::ConstructorDefinitionContext::accept(tree::ParseTreeVisitor *visitor) {
  if (auto parserVisitor = dynamic_cast<SolidityVisitor*>(visitor))
    return parserVisitor->visitConstructorDefinition(this);
  else
    return visitor->visitChildren(this);
}

SolidityParser::ConstructorDefinitionContext* SolidityParser::constructorDefinition() {
  ConstructorDefinitionContext *_localctx = _tracker.createInstance<ConstructorDefinitionContext>(_ctx, getState());
  enterRule(_localctx, 38, SolidityParser::RuleConstructorDefinition);

  auto onExit = finally([=] {
    exitRule();
  });
  try {
    size_t alt;
    enterOuterAlt(_localctx, 1);
    setState(351);
    match(SolidityParser::Constructor);
    setState(352);
    match(SolidityParser::LParen);
    setState(354);
    _errHandler->sync(this);

    switch (getInterpreter<atn::ParserATNSimulator>()->adaptivePredict(_input, 26, _ctx)) {
    case 1: {
      setState(353);
      dynamic_cast<ConstructorDefinitionContext *>(_localctx)->arguments = parameterList();
      break;
    }

    }
    setState(356);
    match(SolidityParser::RParen);
    setState(369);
    _errHandler->sync(this);
    alt = getInterpreter<atn::ParserATNSimulator>()->adaptivePredict(_input, 28, _ctx);
    while (alt != 2 && alt != atn::ATN::INVALID_ALT_NUMBER) {
      if (alt == 1) {
        setState(367);
        _errHandler->sync(this);
        switch (getInterpreter<atn::ParserATNSimulator>()->adaptivePredict(_input, 27, _ctx)) {
        case 1: {
          setState(357);
          modifierInvocation();
          break;
        }

        case 2: {
          setState(358);

          if (!(!_localctx->payableSet)) throw FailedPredicateException(this, "!$payableSet");
          setState(359);
          match(SolidityParser::Payable);
          dynamic_cast<ConstructorDefinitionContext *>(_localctx)->payableSet =  true;
          break;
        }

        case 3: {
          setState(361);

          if (!(!_localctx->visibilitySet)) throw FailedPredicateException(this, "!$visibilitySet");
          setState(362);
          match(SolidityParser::Internal);
          dynamic_cast<ConstructorDefinitionContext *>(_localctx)->visibilitySet =  true;
          break;
        }

        case 4: {
          setState(364);

          if (!(!_localctx->visibilitySet)) throw FailedPredicateException(this, "!$visibilitySet");
          setState(365);
          match(SolidityParser::Public);
          dynamic_cast<ConstructorDefinitionContext *>(_localctx)->visibilitySet =  true;
          break;
        }

        } 
      }
      setState(371);
      _errHandler->sync(this);
      alt = getInterpreter<atn::ParserATNSimulator>()->adaptivePredict(_input, 28, _ctx);
    }
    setState(372);
    dynamic_cast<ConstructorDefinitionContext *>(_localctx)->body = block();
   
  }
  catch (RecognitionException &e) {
    _errHandler->reportError(this, e);
    _localctx->exception = std::current_exception();
    _errHandler->recover(this, _localctx->exception);
  }

  return _localctx;
}

//----------------- StateMutabilityContext ------------------------------------------------------------------

SolidityParser::StateMutabilityContext::StateMutabilityContext(ParserRuleContext *parent, size_t invokingState)
  : ParserRuleContext(parent, invokingState) {
}

tree::TerminalNode* SolidityParser::StateMutabilityContext::Pure() {
  return getToken(SolidityParser::Pure, 0);
}

tree::TerminalNode* SolidityParser::StateMutabilityContext::View() {
  return getToken(SolidityParser::View, 0);
}

tree::TerminalNode* SolidityParser::StateMutabilityContext::Payable() {
  return getToken(SolidityParser::Payable, 0);
}


size_t SolidityParser::StateMutabilityContext::getRuleIndex() const {
  return SolidityParser::RuleStateMutability;
}


antlrcpp::Any SolidityParser::StateMutabilityContext::accept(tree::ParseTreeVisitor *visitor) {
  if (auto parserVisitor = dynamic_cast<SolidityVisitor*>(visitor))
    return parserVisitor->visitStateMutability(this);
  else
    return visitor->visitChildren(this);
}

SolidityParser::StateMutabilityContext* SolidityParser::stateMutability() {
  StateMutabilityContext *_localctx = _tracker.createInstance<StateMutabilityContext>(_ctx, getState());
  enterRule(_localctx, 40, SolidityParser::RuleStateMutability);
  size_t _la = 0;

  auto onExit = finally([=] {
    exitRule();
  });
  try {
    enterOuterAlt(_localctx, 1);
    setState(374);
    _la = _input->LA(1);
    if (!((((_la & ~ 0x3fULL) == 0) &&
      ((1ULL << _la) & ((1ULL << SolidityParser::Payable)
      | (1ULL << SolidityParser::Pure)
      | (1ULL << SolidityParser::View))) != 0))) {
    _errHandler->recoverInline(this);
    }
    else {
      _errHandler->reportMatch(this);
      consume();
    }
   
  }
  catch (RecognitionException &e) {
    _errHandler->reportError(this, e);
    _localctx->exception = std::current_exception();
    _errHandler->recover(this, _localctx->exception);
  }

  return _localctx;
}

//----------------- OverrideSpecifierContext ------------------------------------------------------------------

SolidityParser::OverrideSpecifierContext::OverrideSpecifierContext(ParserRuleContext *parent, size_t invokingState)
  : ParserRuleContext(parent, invokingState) {
}

tree::TerminalNode* SolidityParser::OverrideSpecifierContext::Override() {
  return getToken(SolidityParser::Override, 0);
}

tree::TerminalNode* SolidityParser::OverrideSpecifierContext::LParen() {
  return getToken(SolidityParser::LParen, 0);
}

tree::TerminalNode* SolidityParser::OverrideSpecifierContext::RParen() {
  return getToken(SolidityParser::RParen, 0);
}

std::vector<SolidityParser::UserDefinedTypeNameContext *> SolidityParser::OverrideSpecifierContext::userDefinedTypeName() {
  return getRuleContexts<SolidityParser::UserDefinedTypeNameContext>();
}

SolidityParser::UserDefinedTypeNameContext* SolidityParser::OverrideSpecifierContext::userDefinedTypeName(size_t i) {
  return getRuleContext<SolidityParser::UserDefinedTypeNameContext>(i);
}

std::vector<tree::TerminalNode *> SolidityParser::OverrideSpecifierContext::Comma() {
  return getTokens(SolidityParser::Comma);
}

tree::TerminalNode* SolidityParser::OverrideSpecifierContext::Comma(size_t i) {
  return getToken(SolidityParser::Comma, i);
}


size_t SolidityParser::OverrideSpecifierContext::getRuleIndex() const {
  return SolidityParser::RuleOverrideSpecifier;
}


antlrcpp::Any SolidityParser::OverrideSpecifierContext::accept(tree::ParseTreeVisitor *visitor) {
  if (auto parserVisitor = dynamic_cast<SolidityVisitor*>(visitor))
    return parserVisitor->visitOverrideSpecifier(this);
  else
    return visitor->visitChildren(this);
}

SolidityParser::OverrideSpecifierContext* SolidityParser::overrideSpecifier() {
  OverrideSpecifierContext *_localctx = _tracker.createInstance<OverrideSpecifierContext>(_ctx, getState());
  enterRule(_localctx, 42, SolidityParser::RuleOverrideSpecifier);
  size_t _la = 0;

  auto onExit = finally([=] {
    exitRule();
  });
  try {
    enterOuterAlt(_localctx, 1);
    setState(376);
    match(SolidityParser::Override);
    setState(388);
    _errHandler->sync(this);

    switch (getInterpreter<atn::ParserATNSimulator>()->adaptivePredict(_input, 30, _ctx)) {
    case 1: {
      setState(377);
      match(SolidityParser::LParen);
      setState(378);
      dynamic_cast<OverrideSpecifierContext *>(_localctx)->userDefinedTypeNameContext = userDefinedTypeName();
      dynamic_cast<OverrideSpecifierContext *>(_localctx)->overrides.push_back(dynamic_cast<OverrideSpecifierContext *>(_localctx)->userDefinedTypeNameContext);
      setState(383);
      _errHandler->sync(this);
      _la = _input->LA(1);
      while (_la == SolidityParser::Comma) {
        setState(379);
        match(SolidityParser::Comma);
        setState(380);
        dynamic_cast<OverrideSpecifierContext *>(_localctx)->userDefinedTypeNameContext = userDefinedTypeName();
        dynamic_cast<OverrideSpecifierContext *>(_localctx)->overrides.push_back(dynamic_cast<OverrideSpecifierContext *>(_localctx)->userDefinedTypeNameContext);
        setState(385);
        _errHandler->sync(this);
        _la = _input->LA(1);
      }
      setState(386);
      match(SolidityParser::RParen);
      break;
    }

    }
   
  }
  catch (RecognitionException &e) {
    _errHandler->reportError(this, e);
    _localctx->exception = std::current_exception();
    _errHandler->recover(this, _localctx->exception);
  }

  return _localctx;
}

//----------------- FunctionDefinitionContext ------------------------------------------------------------------

SolidityParser::FunctionDefinitionContext::FunctionDefinitionContext(ParserRuleContext *parent, size_t invokingState)
  : ParserRuleContext(parent, invokingState) {
}

tree::TerminalNode* SolidityParser::FunctionDefinitionContext::Function() {
  return getToken(SolidityParser::Function, 0);
}

std::vector<tree::TerminalNode *> SolidityParser::FunctionDefinitionContext::LParen() {
  return getTokens(SolidityParser::LParen);
}

tree::TerminalNode* SolidityParser::FunctionDefinitionContext::LParen(size_t i) {
  return getToken(SolidityParser::LParen, i);
}

std::vector<tree::TerminalNode *> SolidityParser::FunctionDefinitionContext::RParen() {
  return getTokens(SolidityParser::RParen);
}

tree::TerminalNode* SolidityParser::FunctionDefinitionContext::RParen(size_t i) {
  return getToken(SolidityParser::RParen, i);
}

SolidityParser::IdentifierContext* SolidityParser::FunctionDefinitionContext::identifier() {
  return getRuleContext<SolidityParser::IdentifierContext>(0);
}

tree::TerminalNode* SolidityParser::FunctionDefinitionContext::Fallback() {
  return getToken(SolidityParser::Fallback, 0);
}

tree::TerminalNode* SolidityParser::FunctionDefinitionContext::Receive() {
  return getToken(SolidityParser::Receive, 0);
}

tree::TerminalNode* SolidityParser::FunctionDefinitionContext::Semicolon() {
  return getToken(SolidityParser::Semicolon, 0);
}

std::vector<SolidityParser::VisibilityContext *> SolidityParser::FunctionDefinitionContext::visibility() {
  return getRuleContexts<SolidityParser::VisibilityContext>();
}

SolidityParser::VisibilityContext* SolidityParser::FunctionDefinitionContext::visibility(size_t i) {
  return getRuleContext<SolidityParser::VisibilityContext>(i);
}

std::vector<SolidityParser::StateMutabilityContext *> SolidityParser::FunctionDefinitionContext::stateMutability() {
  return getRuleContexts<SolidityParser::StateMutabilityContext>();
}

SolidityParser::StateMutabilityContext* SolidityParser::FunctionDefinitionContext::stateMutability(size_t i) {
  return getRuleContext<SolidityParser::StateMutabilityContext>(i);
}

std::vector<SolidityParser::ModifierInvocationContext *> SolidityParser::FunctionDefinitionContext::modifierInvocation() {
  return getRuleContexts<SolidityParser::ModifierInvocationContext>();
}

SolidityParser::ModifierInvocationContext* SolidityParser::FunctionDefinitionContext::modifierInvocation(size_t i) {
  return getRuleContext<SolidityParser::ModifierInvocationContext>(i);
}

std::vector<tree::TerminalNode *> SolidityParser::FunctionDefinitionContext::Virtual() {
  return getTokens(SolidityParser::Virtual);
}

tree::TerminalNode* SolidityParser::FunctionDefinitionContext::Virtual(size_t i) {
  return getToken(SolidityParser::Virtual, i);
}

std::vector<SolidityParser::OverrideSpecifierContext *> SolidityParser::FunctionDefinitionContext::overrideSpecifier() {
  return getRuleContexts<SolidityParser::OverrideSpecifierContext>();
}

SolidityParser::OverrideSpecifierContext* SolidityParser::FunctionDefinitionContext::overrideSpecifier(size_t i) {
  return getRuleContext<SolidityParser::OverrideSpecifierContext>(i);
}

tree::TerminalNode* SolidityParser::FunctionDefinitionContext::Returns() {
  return getToken(SolidityParser::Returns, 0);
}

SolidityParser::BlockContext* SolidityParser::FunctionDefinitionContext::block() {
  return getRuleContext<SolidityParser::BlockContext>(0);
}

std::vector<SolidityParser::ParameterListContext *> SolidityParser::FunctionDefinitionContext::parameterList() {
  return getRuleContexts<SolidityParser::ParameterListContext>();
}

SolidityParser::ParameterListContext* SolidityParser::FunctionDefinitionContext::parameterList(size_t i) {
  return getRuleContext<SolidityParser::ParameterListContext>(i);
}


size_t SolidityParser::FunctionDefinitionContext::getRuleIndex() const {
  return SolidityParser::RuleFunctionDefinition;
}


antlrcpp::Any SolidityParser::FunctionDefinitionContext::accept(tree::ParseTreeVisitor *visitor) {
  if (auto parserVisitor = dynamic_cast<SolidityVisitor*>(visitor))
    return parserVisitor->visitFunctionDefinition(this);
  else
    return visitor->visitChildren(this);
}

SolidityParser::FunctionDefinitionContext* SolidityParser::functionDefinition() {
  FunctionDefinitionContext *_localctx = _tracker.createInstance<FunctionDefinitionContext>(_ctx, getState());
  enterRule(_localctx, 44, SolidityParser::RuleFunctionDefinition);
  size_t _la = 0;

  auto onExit = finally([=] {
    exitRule();
  });
  try {
    size_t alt;
    enterOuterAlt(_localctx, 1);
    setState(390);
    match(SolidityParser::Function);
    setState(394);
    _errHandler->sync(this);
    switch (_input->LA(1)) {
      case SolidityParser::From:
      case SolidityParser::Identifier: {
        setState(391);
        identifier();
        break;
      }

      case SolidityParser::Fallback: {
        setState(392);
        match(SolidityParser::Fallback);
        break;
      }

      case SolidityParser::Receive: {
        setState(393);
        match(SolidityParser::Receive);
        break;
      }

    default:
      throw NoViableAltException(this);
    }
    setState(396);
    match(SolidityParser::LParen);
    setState(398);
    _errHandler->sync(this);

    switch (getInterpreter<atn::ParserATNSimulator>()->adaptivePredict(_input, 32, _ctx)) {
    case 1: {
      setState(397);
      dynamic_cast<FunctionDefinitionContext *>(_localctx)->arguments = parameterList();
      break;
    }

    }
    setState(400);
    match(SolidityParser::RParen);
    setState(419);
    _errHandler->sync(this);
    alt = getInterpreter<atn::ParserATNSimulator>()->adaptivePredict(_input, 34, _ctx);
    while (alt != 2 && alt != atn::ATN::INVALID_ALT_NUMBER) {
      if (alt == 1) {
        setState(417);
        _errHandler->sync(this);
        switch (getInterpreter<atn::ParserATNSimulator>()->adaptivePredict(_input, 33, _ctx)) {
        case 1: {
          setState(401);

          if (!(!_localctx->visibilitySet)) throw FailedPredicateException(this, "!$visibilitySet");
          setState(402);
          visibility();
          dynamic_cast<FunctionDefinitionContext *>(_localctx)->visibilitySet =  true;
          break;
        }

        case 2: {
          setState(405);

          if (!(!_localctx->mutabilitySet)) throw FailedPredicateException(this, "!$mutabilitySet");
          setState(406);
          stateMutability();
          dynamic_cast<FunctionDefinitionContext *>(_localctx)->mutabilitySet =  true;
          break;
        }

        case 3: {
          setState(409);
          modifierInvocation();
          break;
        }

        case 4: {
          setState(410);

          if (!(!_localctx->virtualSet)) throw FailedPredicateException(this, "!$virtualSet");
          setState(411);
          match(SolidityParser::Virtual);
          dynamic_cast<FunctionDefinitionContext *>(_localctx)->virtualSet =  true;
          break;
        }

        case 5: {
          setState(413);

          if (!(!_localctx->overrideSpecifierSet)) throw FailedPredicateException(this, "!$overrideSpecifierSet");
          setState(414);
          overrideSpecifier();
          dynamic_cast<FunctionDefinitionContext *>(_localctx)->overrideSpecifierSet =  true;
          break;
        }

        } 
      }
      setState(421);
      _errHandler->sync(this);
      alt = getInterpreter<atn::ParserATNSimulator>()->adaptivePredict(_input, 34, _ctx);
    }
    setState(427);
    _errHandler->sync(this);

    _la = _input->LA(1);
    if (_la == SolidityParser::Returns) {
      setState(422);
      match(SolidityParser::Returns);
      setState(423);
      match(SolidityParser::LParen);
      setState(424);
      dynamic_cast<FunctionDefinitionContext *>(_localctx)->returnParameters = parameterList();
      setState(425);
      match(SolidityParser::RParen);
    }
    setState(431);
    _errHandler->sync(this);
    switch (_input->LA(1)) {
      case SolidityParser::Semicolon: {
        setState(429);
        match(SolidityParser::Semicolon);
        break;
      }

      case SolidityParser::LBrace: {
        setState(430);
        dynamic_cast<FunctionDefinitionContext *>(_localctx)->body = block();
        break;
      }

    default:
      throw NoViableAltException(this);
    }
   
  }
  catch (RecognitionException &e) {
    _errHandler->reportError(this, e);
    _localctx->exception = std::current_exception();
    _errHandler->recover(this, _localctx->exception);
  }

  return _localctx;
}

//----------------- ModifierDefinitionContext ------------------------------------------------------------------

SolidityParser::ModifierDefinitionContext::ModifierDefinitionContext(ParserRuleContext *parent, size_t invokingState)
  : ParserRuleContext(parent, invokingState) {
}

tree::TerminalNode* SolidityParser::ModifierDefinitionContext::Modifier() {
  return getToken(SolidityParser::Modifier, 0);
}

SolidityParser::IdentifierContext* SolidityParser::ModifierDefinitionContext::identifier() {
  return getRuleContext<SolidityParser::IdentifierContext>(0);
}

tree::TerminalNode* SolidityParser::ModifierDefinitionContext::Semicolon() {
  return getToken(SolidityParser::Semicolon, 0);
}

tree::TerminalNode* SolidityParser::ModifierDefinitionContext::LParen() {
  return getToken(SolidityParser::LParen, 0);
}

tree::TerminalNode* SolidityParser::ModifierDefinitionContext::RParen() {
  return getToken(SolidityParser::RParen, 0);
}

std::vector<tree::TerminalNode *> SolidityParser::ModifierDefinitionContext::Virtual() {
  return getTokens(SolidityParser::Virtual);
}

tree::TerminalNode* SolidityParser::ModifierDefinitionContext::Virtual(size_t i) {
  return getToken(SolidityParser::Virtual, i);
}

std::vector<SolidityParser::OverrideSpecifierContext *> SolidityParser::ModifierDefinitionContext::overrideSpecifier() {
  return getRuleContexts<SolidityParser::OverrideSpecifierContext>();
}

SolidityParser::OverrideSpecifierContext* SolidityParser::ModifierDefinitionContext::overrideSpecifier(size_t i) {
  return getRuleContext<SolidityParser::OverrideSpecifierContext>(i);
}

SolidityParser::BlockContext* SolidityParser::ModifierDefinitionContext::block() {
  return getRuleContext<SolidityParser::BlockContext>(0);
}

SolidityParser::ParameterListContext* SolidityParser::ModifierDefinitionContext::parameterList() {
  return getRuleContext<SolidityParser::ParameterListContext>(0);
}


size_t SolidityParser::ModifierDefinitionContext::getRuleIndex() const {
  return SolidityParser::RuleModifierDefinition;
}


antlrcpp::Any SolidityParser::ModifierDefinitionContext::accept(tree::ParseTreeVisitor *visitor) {
  if (auto parserVisitor = dynamic_cast<SolidityVisitor*>(visitor))
    return parserVisitor->visitModifierDefinition(this);
  else
    return visitor->visitChildren(this);
}

SolidityParser::ModifierDefinitionContext* SolidityParser::modifierDefinition() {
  ModifierDefinitionContext *_localctx = _tracker.createInstance<ModifierDefinitionContext>(_ctx, getState());
  enterRule(_localctx, 46, SolidityParser::RuleModifierDefinition);

  auto onExit = finally([=] {
    exitRule();
  });
  try {
    size_t alt;
    enterOuterAlt(_localctx, 1);
    setState(433);
    match(SolidityParser::Modifier);
    setState(434);
    dynamic_cast<ModifierDefinitionContext *>(_localctx)->name = identifier();
    setState(440);
    _errHandler->sync(this);

    switch (getInterpreter<atn::ParserATNSimulator>()->adaptivePredict(_input, 38, _ctx)) {
    case 1: {
      setState(435);
      match(SolidityParser::LParen);
      setState(437);
      _errHandler->sync(this);

      switch (getInterpreter<atn::ParserATNSimulator>()->adaptivePredict(_input, 37, _ctx)) {
      case 1: {
        setState(436);
        dynamic_cast<ModifierDefinitionContext *>(_localctx)->arguments = parameterList();
        break;
      }

      }
      setState(439);
      match(SolidityParser::RParen);
      break;
    }

    }
    setState(451);
    _errHandler->sync(this);
    alt = getInterpreter<atn::ParserATNSimulator>()->adaptivePredict(_input, 40, _ctx);
    while (alt != 2 && alt != atn::ATN::INVALID_ALT_NUMBER) {
      if (alt == 1) {
        setState(449);
        _errHandler->sync(this);
        switch (getInterpreter<atn::ParserATNSimulator>()->adaptivePredict(_input, 39, _ctx)) {
        case 1: {
          setState(442);

          if (!(!_localctx->virtualSet)) throw FailedPredicateException(this, "!$virtualSet");
          setState(443);
          match(SolidityParser::Virtual);
          dynamic_cast<ModifierDefinitionContext *>(_localctx)->virtualSet =  true;
          break;
        }

        case 2: {
          setState(445);

          if (!(!_localctx->overrideSpecifierSet)) throw FailedPredicateException(this, "!$overrideSpecifierSet");
          setState(446);
          overrideSpecifier();
          dynamic_cast<ModifierDefinitionContext *>(_localctx)->overrideSpecifierSet =  true;
          break;
        }

        } 
      }
      setState(453);
      _errHandler->sync(this);
      alt = getInterpreter<atn::ParserATNSimulator>()->adaptivePredict(_input, 40, _ctx);
    }
    setState(456);
    _errHandler->sync(this);
    switch (_input->LA(1)) {
      case SolidityParser::Semicolon: {
        setState(454);
        match(SolidityParser::Semicolon);
        break;
      }

      case SolidityParser::LBrace: {
        setState(455);
        dynamic_cast<ModifierDefinitionContext *>(_localctx)->body = block();
        break;
      }

    default:
      throw NoViableAltException(this);
    }
   
  }
  catch (RecognitionException &e) {
    _errHandler->reportError(this, e);
    _localctx->exception = std::current_exception();
    _errHandler->recover(this, _localctx->exception);
  }

  return _localctx;
}

//----------------- FallbackReceiveFunctionDefinitionContext ------------------------------------------------------------------

SolidityParser::FallbackReceiveFunctionDefinitionContext::FallbackReceiveFunctionDefinitionContext(ParserRuleContext *parent, size_t invokingState)
  : ParserRuleContext(parent, invokingState) {
}

tree::TerminalNode* SolidityParser::FallbackReceiveFunctionDefinitionContext::LParen() {
  return getToken(SolidityParser::LParen, 0);
}

tree::TerminalNode* SolidityParser::FallbackReceiveFunctionDefinitionContext::RParen() {
  return getToken(SolidityParser::RParen, 0);
}

tree::TerminalNode* SolidityParser::FallbackReceiveFunctionDefinitionContext::Fallback() {
  return getToken(SolidityParser::Fallback, 0);
}

tree::TerminalNode* SolidityParser::FallbackReceiveFunctionDefinitionContext::Receive() {
  return getToken(SolidityParser::Receive, 0);
}

tree::TerminalNode* SolidityParser::FallbackReceiveFunctionDefinitionContext::Semicolon() {
  return getToken(SolidityParser::Semicolon, 0);
}

std::vector<SolidityParser::VisibilityContext *> SolidityParser::FallbackReceiveFunctionDefinitionContext::visibility() {
  return getRuleContexts<SolidityParser::VisibilityContext>();
}

SolidityParser::VisibilityContext* SolidityParser::FallbackReceiveFunctionDefinitionContext::visibility(size_t i) {
  return getRuleContext<SolidityParser::VisibilityContext>(i);
}

std::vector<SolidityParser::StateMutabilityContext *> SolidityParser::FallbackReceiveFunctionDefinitionContext::stateMutability() {
  return getRuleContexts<SolidityParser::StateMutabilityContext>();
}

SolidityParser::StateMutabilityContext* SolidityParser::FallbackReceiveFunctionDefinitionContext::stateMutability(size_t i) {
  return getRuleContext<SolidityParser::StateMutabilityContext>(i);
}

std::vector<SolidityParser::ModifierInvocationContext *> SolidityParser::FallbackReceiveFunctionDefinitionContext::modifierInvocation() {
  return getRuleContexts<SolidityParser::ModifierInvocationContext>();
}

SolidityParser::ModifierInvocationContext* SolidityParser::FallbackReceiveFunctionDefinitionContext::modifierInvocation(size_t i) {
  return getRuleContext<SolidityParser::ModifierInvocationContext>(i);
}

std::vector<tree::TerminalNode *> SolidityParser::FallbackReceiveFunctionDefinitionContext::Virtual() {
  return getTokens(SolidityParser::Virtual);
}

tree::TerminalNode* SolidityParser::FallbackReceiveFunctionDefinitionContext::Virtual(size_t i) {
  return getToken(SolidityParser::Virtual, i);
}

std::vector<SolidityParser::OverrideSpecifierContext *> SolidityParser::FallbackReceiveFunctionDefinitionContext::overrideSpecifier() {
  return getRuleContexts<SolidityParser::OverrideSpecifierContext>();
}

SolidityParser::OverrideSpecifierContext* SolidityParser::FallbackReceiveFunctionDefinitionContext::overrideSpecifier(size_t i) {
  return getRuleContext<SolidityParser::OverrideSpecifierContext>(i);
}

SolidityParser::BlockContext* SolidityParser::FallbackReceiveFunctionDefinitionContext::block() {
  return getRuleContext<SolidityParser::BlockContext>(0);
}


size_t SolidityParser::FallbackReceiveFunctionDefinitionContext::getRuleIndex() const {
  return SolidityParser::RuleFallbackReceiveFunctionDefinition;
}


antlrcpp::Any SolidityParser::FallbackReceiveFunctionDefinitionContext::accept(tree::ParseTreeVisitor *visitor) {
  if (auto parserVisitor = dynamic_cast<SolidityVisitor*>(visitor))
    return parserVisitor->visitFallbackReceiveFunctionDefinition(this);
  else
    return visitor->visitChildren(this);
}

SolidityParser::FallbackReceiveFunctionDefinitionContext* SolidityParser::fallbackReceiveFunctionDefinition() {
  FallbackReceiveFunctionDefinitionContext *_localctx = _tracker.createInstance<FallbackReceiveFunctionDefinitionContext>(_ctx, getState());
  enterRule(_localctx, 48, SolidityParser::RuleFallbackReceiveFunctionDefinition);
  size_t _la = 0;

  auto onExit = finally([=] {
    exitRule();
  });
  try {
    size_t alt;
    enterOuterAlt(_localctx, 1);
    setState(458);
    dynamic_cast<FallbackReceiveFunctionDefinitionContext *>(_localctx)->kind = _input->LT(1);
    _la = _input->LA(1);
    if (!(_la == SolidityParser::Fallback

    || _la == SolidityParser::Receive)) {
      dynamic_cast<FallbackReceiveFunctionDefinitionContext *>(_localctx)->kind = _errHandler->recoverInline(this);
    }
    else {
      _errHandler->reportMatch(this);
      consume();
    }
    setState(459);
    match(SolidityParser::LParen);
    setState(460);
    match(SolidityParser::RParen);
    setState(479);
    _errHandler->sync(this);
    alt = getInterpreter<atn::ParserATNSimulator>()->adaptivePredict(_input, 43, _ctx);
    while (alt != 2 && alt != atn::ATN::INVALID_ALT_NUMBER) {
      if (alt == 1) {
        setState(477);
        _errHandler->sync(this);
        switch (getInterpreter<atn::ParserATNSimulator>()->adaptivePredict(_input, 42, _ctx)) {
        case 1: {
          setState(461);

          if (!(!_localctx->visibilitySet)) throw FailedPredicateException(this, "!$visibilitySet");
          setState(462);
          visibility();
          dynamic_cast<FallbackReceiveFunctionDefinitionContext *>(_localctx)->visibilitySet =  true;
          break;
        }

        case 2: {
          setState(465);

          if (!(!_localctx->mutabilitySet)) throw FailedPredicateException(this, "!$mutabilitySet");
          setState(466);
          stateMutability();
          dynamic_cast<FallbackReceiveFunctionDefinitionContext *>(_localctx)->mutabilitySet =  true;
          break;
        }

        case 3: {
          setState(469);
          modifierInvocation();
          break;
        }

        case 4: {
          setState(470);

          if (!(!_localctx->virtualSet)) throw FailedPredicateException(this, "!$virtualSet");
          setState(471);
          match(SolidityParser::Virtual);
          dynamic_cast<FallbackReceiveFunctionDefinitionContext *>(_localctx)->virtualSet =  true;
          break;
        }

        case 5: {
          setState(473);

          if (!(!_localctx->overrideSpecifierSet)) throw FailedPredicateException(this, "!$overrideSpecifierSet");
          setState(474);
          overrideSpecifier();
          dynamic_cast<FallbackReceiveFunctionDefinitionContext *>(_localctx)->overrideSpecifierSet =  true;
          break;
        }

        } 
      }
      setState(481);
      _errHandler->sync(this);
      alt = getInterpreter<atn::ParserATNSimulator>()->adaptivePredict(_input, 43, _ctx);
    }
    setState(484);
    _errHandler->sync(this);
    switch (_input->LA(1)) {
      case SolidityParser::Semicolon: {
        setState(482);
        match(SolidityParser::Semicolon);
        break;
      }

      case SolidityParser::LBrace: {
        setState(483);
        dynamic_cast<FallbackReceiveFunctionDefinitionContext *>(_localctx)->body = block();
        break;
      }

    default:
      throw NoViableAltException(this);
    }
   
  }
  catch (RecognitionException &e) {
    _errHandler->reportError(this, e);
    _localctx->exception = std::current_exception();
    _errHandler->recover(this, _localctx->exception);
  }

  return _localctx;
}

//----------------- StructDefinitionContext ------------------------------------------------------------------

SolidityParser::StructDefinitionContext::StructDefinitionContext(ParserRuleContext *parent, size_t invokingState)
  : ParserRuleContext(parent, invokingState) {
}

tree::TerminalNode* SolidityParser::StructDefinitionContext::Struct() {
  return getToken(SolidityParser::Struct, 0);
}

tree::TerminalNode* SolidityParser::StructDefinitionContext::LBrace() {
  return getToken(SolidityParser::LBrace, 0);
}

tree::TerminalNode* SolidityParser::StructDefinitionContext::RBrace() {
  return getToken(SolidityParser::RBrace, 0);
}

SolidityParser::IdentifierContext* SolidityParser::StructDefinitionContext::identifier() {
  return getRuleContext<SolidityParser::IdentifierContext>(0);
}

std::vector<SolidityParser::StructMemberContext *> SolidityParser::StructDefinitionContext::structMember() {
  return getRuleContexts<SolidityParser::StructMemberContext>();
}

SolidityParser::StructMemberContext* SolidityParser::StructDefinitionContext::structMember(size_t i) {
  return getRuleContext<SolidityParser::StructMemberContext>(i);
}


size_t SolidityParser::StructDefinitionContext::getRuleIndex() const {
  return SolidityParser::RuleStructDefinition;
}


antlrcpp::Any SolidityParser::StructDefinitionContext::accept(tree::ParseTreeVisitor *visitor) {
  if (auto parserVisitor = dynamic_cast<SolidityVisitor*>(visitor))
    return parserVisitor->visitStructDefinition(this);
  else
    return visitor->visitChildren(this);
}

SolidityParser::StructDefinitionContext* SolidityParser::structDefinition() {
  StructDefinitionContext *_localctx = _tracker.createInstance<StructDefinitionContext>(_ctx, getState());
  enterRule(_localctx, 50, SolidityParser::RuleStructDefinition);

  auto onExit = finally([=] {
    exitRule();
  });
  try {
    size_t alt;
    enterOuterAlt(_localctx, 1);
    setState(486);
    match(SolidityParser::Struct);
    setState(487);
    dynamic_cast<StructDefinitionContext *>(_localctx)->name = identifier();
    setState(488);
    match(SolidityParser::LBrace);
    setState(490); 
    _errHandler->sync(this);
    alt = 1;
    do {
      switch (alt) {
        case 1: {
              setState(489);
              dynamic_cast<StructDefinitionContext *>(_localctx)->members = structMember();
              break;
            }

      default:
        throw NoViableAltException(this);
      }
      setState(492); 
      _errHandler->sync(this);
      alt = getInterpreter<atn::ParserATNSimulator>()->adaptivePredict(_input, 45, _ctx);
    } while (alt != 2 && alt != atn::ATN::INVALID_ALT_NUMBER);
    setState(494);
    match(SolidityParser::RBrace);
   
  }
  catch (RecognitionException &e) {
    _errHandler->reportError(this, e);
    _localctx->exception = std::current_exception();
    _errHandler->recover(this, _localctx->exception);
  }

  return _localctx;
}

//----------------- StructMemberContext ------------------------------------------------------------------

SolidityParser::StructMemberContext::StructMemberContext(ParserRuleContext *parent, size_t invokingState)
  : ParserRuleContext(parent, invokingState) {
}

tree::TerminalNode* SolidityParser::StructMemberContext::Semicolon() {
  return getToken(SolidityParser::Semicolon, 0);
}

SolidityParser::TypeNameContext* SolidityParser::StructMemberContext::typeName() {
  return getRuleContext<SolidityParser::TypeNameContext>(0);
}

SolidityParser::IdentifierContext* SolidityParser::StructMemberContext::identifier() {
  return getRuleContext<SolidityParser::IdentifierContext>(0);
}


size_t SolidityParser::StructMemberContext::getRuleIndex() const {
  return SolidityParser::RuleStructMember;
}


antlrcpp::Any SolidityParser::StructMemberContext::accept(tree::ParseTreeVisitor *visitor) {
  if (auto parserVisitor = dynamic_cast<SolidityVisitor*>(visitor))
    return parserVisitor->visitStructMember(this);
  else
    return visitor->visitChildren(this);
}

SolidityParser::StructMemberContext* SolidityParser::structMember() {
  StructMemberContext *_localctx = _tracker.createInstance<StructMemberContext>(_ctx, getState());
  enterRule(_localctx, 52, SolidityParser::RuleStructMember);

  auto onExit = finally([=] {
    exitRule();
  });
  try {
    enterOuterAlt(_localctx, 1);
    setState(496);
    dynamic_cast<StructMemberContext *>(_localctx)->type = typeName(0);
    setState(497);
    dynamic_cast<StructMemberContext *>(_localctx)->name = identifier();
    setState(498);
    match(SolidityParser::Semicolon);
   
  }
  catch (RecognitionException &e) {
    _errHandler->reportError(this, e);
    _localctx->exception = std::current_exception();
    _errHandler->recover(this, _localctx->exception);
  }

  return _localctx;
}

//----------------- EnumDefinitionContext ------------------------------------------------------------------

SolidityParser::EnumDefinitionContext::EnumDefinitionContext(ParserRuleContext *parent, size_t invokingState)
  : ParserRuleContext(parent, invokingState) {
}

tree::TerminalNode* SolidityParser::EnumDefinitionContext::Enum() {
  return getToken(SolidityParser::Enum, 0);
}

tree::TerminalNode* SolidityParser::EnumDefinitionContext::LBrace() {
  return getToken(SolidityParser::LBrace, 0);
}

tree::TerminalNode* SolidityParser::EnumDefinitionContext::RBrace() {
  return getToken(SolidityParser::RBrace, 0);
}

std::vector<SolidityParser::IdentifierContext *> SolidityParser::EnumDefinitionContext::identifier() {
  return getRuleContexts<SolidityParser::IdentifierContext>();
}

SolidityParser::IdentifierContext* SolidityParser::EnumDefinitionContext::identifier(size_t i) {
  return getRuleContext<SolidityParser::IdentifierContext>(i);
}

std::vector<tree::TerminalNode *> SolidityParser::EnumDefinitionContext::Comma() {
  return getTokens(SolidityParser::Comma);
}

tree::TerminalNode* SolidityParser::EnumDefinitionContext::Comma(size_t i) {
  return getToken(SolidityParser::Comma, i);
}


size_t SolidityParser::EnumDefinitionContext::getRuleIndex() const {
  return SolidityParser::RuleEnumDefinition;
}


antlrcpp::Any SolidityParser::EnumDefinitionContext::accept(tree::ParseTreeVisitor *visitor) {
  if (auto parserVisitor = dynamic_cast<SolidityVisitor*>(visitor))
    return parserVisitor->visitEnumDefinition(this);
  else
    return visitor->visitChildren(this);
}

SolidityParser::EnumDefinitionContext* SolidityParser::enumDefinition() {
  EnumDefinitionContext *_localctx = _tracker.createInstance<EnumDefinitionContext>(_ctx, getState());
  enterRule(_localctx, 54, SolidityParser::RuleEnumDefinition);
  size_t _la = 0;

  auto onExit = finally([=] {
    exitRule();
  });
  try {
    enterOuterAlt(_localctx, 1);
    setState(500);
    match(SolidityParser::Enum);
    setState(501);
    dynamic_cast<EnumDefinitionContext *>(_localctx)->name = identifier();
    setState(502);
    match(SolidityParser::LBrace);
    setState(503);
    dynamic_cast<EnumDefinitionContext *>(_localctx)->identifierContext = identifier();
    dynamic_cast<EnumDefinitionContext *>(_localctx)->enumValues.push_back(dynamic_cast<EnumDefinitionContext *>(_localctx)->identifierContext);
    setState(508);
    _errHandler->sync(this);
    _la = _input->LA(1);
    while (_la == SolidityParser::Comma) {
      setState(504);
      match(SolidityParser::Comma);
      setState(505);
      dynamic_cast<EnumDefinitionContext *>(_localctx)->identifierContext = identifier();
      dynamic_cast<EnumDefinitionContext *>(_localctx)->enumValues.push_back(dynamic_cast<EnumDefinitionContext *>(_localctx)->identifierContext);
      setState(510);
      _errHandler->sync(this);
      _la = _input->LA(1);
    }
    setState(511);
    match(SolidityParser::RBrace);
   
  }
  catch (RecognitionException &e) {
    _errHandler->reportError(this, e);
    _localctx->exception = std::current_exception();
    _errHandler->recover(this, _localctx->exception);
  }

  return _localctx;
}

//----------------- StateVariableDeclarationContext ------------------------------------------------------------------

SolidityParser::StateVariableDeclarationContext::StateVariableDeclarationContext(ParserRuleContext *parent, size_t invokingState)
  : ParserRuleContext(parent, invokingState) {
}

tree::TerminalNode* SolidityParser::StateVariableDeclarationContext::Semicolon() {
  return getToken(SolidityParser::Semicolon, 0);
}

SolidityParser::TypeNameContext* SolidityParser::StateVariableDeclarationContext::typeName() {
  return getRuleContext<SolidityParser::TypeNameContext>(0);
}

SolidityParser::IdentifierContext* SolidityParser::StateVariableDeclarationContext::identifier() {
  return getRuleContext<SolidityParser::IdentifierContext>(0);
}

std::vector<tree::TerminalNode *> SolidityParser::StateVariableDeclarationContext::Public() {
  return getTokens(SolidityParser::Public);
}

tree::TerminalNode* SolidityParser::StateVariableDeclarationContext::Public(size_t i) {
  return getToken(SolidityParser::Public, i);
}

std::vector<tree::TerminalNode *> SolidityParser::StateVariableDeclarationContext::Private() {
  return getTokens(SolidityParser::Private);
}

tree::TerminalNode* SolidityParser::StateVariableDeclarationContext::Private(size_t i) {
  return getToken(SolidityParser::Private, i);
}

std::vector<tree::TerminalNode *> SolidityParser::StateVariableDeclarationContext::Internal() {
  return getTokens(SolidityParser::Internal);
}

tree::TerminalNode* SolidityParser::StateVariableDeclarationContext::Internal(size_t i) {
  return getToken(SolidityParser::Internal, i);
}

std::vector<tree::TerminalNode *> SolidityParser::StateVariableDeclarationContext::Constant() {
  return getTokens(SolidityParser::Constant);
}

tree::TerminalNode* SolidityParser::StateVariableDeclarationContext::Constant(size_t i) {
  return getToken(SolidityParser::Constant, i);
}

std::vector<SolidityParser::OverrideSpecifierContext *> SolidityParser::StateVariableDeclarationContext::overrideSpecifier() {
  return getRuleContexts<SolidityParser::OverrideSpecifierContext>();
}

SolidityParser::OverrideSpecifierContext* SolidityParser::StateVariableDeclarationContext::overrideSpecifier(size_t i) {
  return getRuleContext<SolidityParser::OverrideSpecifierContext>(i);
}

std::vector<tree::TerminalNode *> SolidityParser::StateVariableDeclarationContext::Immutable() {
  return getTokens(SolidityParser::Immutable);
}

tree::TerminalNode* SolidityParser::StateVariableDeclarationContext::Immutable(size_t i) {
  return getToken(SolidityParser::Immutable, i);
}

tree::TerminalNode* SolidityParser::StateVariableDeclarationContext::Assign() {
  return getToken(SolidityParser::Assign, 0);
}

SolidityParser::ExpressionContext* SolidityParser::StateVariableDeclarationContext::expression() {
  return getRuleContext<SolidityParser::ExpressionContext>(0);
}


size_t SolidityParser::StateVariableDeclarationContext::getRuleIndex() const {
  return SolidityParser::RuleStateVariableDeclaration;
}


antlrcpp::Any SolidityParser::StateVariableDeclarationContext::accept(tree::ParseTreeVisitor *visitor) {
  if (auto parserVisitor = dynamic_cast<SolidityVisitor*>(visitor))
    return parserVisitor->visitStateVariableDeclaration(this);
  else
    return visitor->visitChildren(this);
}

SolidityParser::StateVariableDeclarationContext* SolidityParser::stateVariableDeclaration() {
  StateVariableDeclarationContext *_localctx = _tracker.createInstance<StateVariableDeclarationContext>(_ctx, getState());
  enterRule(_localctx, 56, SolidityParser::RuleStateVariableDeclaration);
  size_t _la = 0;

  auto onExit = finally([=] {
    exitRule();
  });
  try {
    size_t alt;
    enterOuterAlt(_localctx, 1);
    setState(513);
    dynamic_cast<StateVariableDeclarationContext *>(_localctx)->type = typeName(0);
    setState(535);
    _errHandler->sync(this);
    alt = getInterpreter<atn::ParserATNSimulator>()->adaptivePredict(_input, 48, _ctx);
    while (alt != 2 && alt != atn::ATN::INVALID_ALT_NUMBER) {
      if (alt == 1) {
        setState(533);
        _errHandler->sync(this);
        switch (getInterpreter<atn::ParserATNSimulator>()->adaptivePredict(_input, 47, _ctx)) {
        case 1: {
          setState(514);

          if (!(!_localctx->visibilitySet)) throw FailedPredicateException(this, "!$visibilitySet");
          setState(515);
          match(SolidityParser::Public);
          dynamic_cast<StateVariableDeclarationContext *>(_localctx)->visibilitySet =  true;
          break;
        }

        case 2: {
          setState(517);

          if (!(!_localctx->visibilitySet)) throw FailedPredicateException(this, "!$visibilitySet");
          setState(518);
          match(SolidityParser::Private);
          dynamic_cast<StateVariableDeclarationContext *>(_localctx)->visibilitySet =  true;
          break;
        }

        case 3: {
          setState(520);

          if (!(!_localctx->visibilitySet)) throw FailedPredicateException(this, "!$visibilitySet");
          setState(521);
          match(SolidityParser::Internal);
          dynamic_cast<StateVariableDeclarationContext *>(_localctx)->visibilitySet =  true;
          break;
        }

        case 4: {
          setState(523);

          if (!(!_localctx->constantnessSet)) throw FailedPredicateException(this, "!$constantnessSet");
          setState(524);
          match(SolidityParser::Constant);
          dynamic_cast<StateVariableDeclarationContext *>(_localctx)->constantnessSet =  true;
          break;
        }

        case 5: {
          setState(526);

          if (!(!_localctx->overrideSpecifierSet)) throw FailedPredicateException(this, "!$overrideSpecifierSet");
          setState(527);
          overrideSpecifier();
          dynamic_cast<StateVariableDeclarationContext *>(_localctx)->overrideSpecifierSet =  true;
          break;
        }

        case 6: {
          setState(530);

          if (!(!_localctx->constantnessSet)) throw FailedPredicateException(this, "!$constantnessSet");
          setState(531);
          match(SolidityParser::Immutable);
          dynamic_cast<StateVariableDeclarationContext *>(_localctx)->constantnessSet =  true;
          break;
        }

        } 
      }
      setState(537);
      _errHandler->sync(this);
      alt = getInterpreter<atn::ParserATNSimulator>()->adaptivePredict(_input, 48, _ctx);
    }
    setState(538);
    dynamic_cast<StateVariableDeclarationContext *>(_localctx)->name = identifier();
    setState(541);
    _errHandler->sync(this);

    _la = _input->LA(1);
    if (_la == SolidityParser::Assign) {
      setState(539);
      match(SolidityParser::Assign);
      setState(540);
      dynamic_cast<StateVariableDeclarationContext *>(_localctx)->initialValue = expression(0);
    }
    setState(543);
    match(SolidityParser::Semicolon);
   
  }
  catch (RecognitionException &e) {
    _errHandler->reportError(this, e);
    _localctx->exception = std::current_exception();
    _errHandler->recover(this, _localctx->exception);
  }

  return _localctx;
}

//----------------- EventParameterContext ------------------------------------------------------------------

SolidityParser::EventParameterContext::EventParameterContext(ParserRuleContext *parent, size_t invokingState)
  : ParserRuleContext(parent, invokingState) {
}

SolidityParser::TypeNameContext* SolidityParser::EventParameterContext::typeName() {
  return getRuleContext<SolidityParser::TypeNameContext>(0);
}

tree::TerminalNode* SolidityParser::EventParameterContext::Indexed() {
  return getToken(SolidityParser::Indexed, 0);
}

SolidityParser::IdentifierContext* SolidityParser::EventParameterContext::identifier() {
  return getRuleContext<SolidityParser::IdentifierContext>(0);
}


size_t SolidityParser::EventParameterContext::getRuleIndex() const {
  return SolidityParser::RuleEventParameter;
}


antlrcpp::Any SolidityParser::EventParameterContext::accept(tree::ParseTreeVisitor *visitor) {
  if (auto parserVisitor = dynamic_cast<SolidityVisitor*>(visitor))
    return parserVisitor->visitEventParameter(this);
  else
    return visitor->visitChildren(this);
}

SolidityParser::EventParameterContext* SolidityParser::eventParameter() {
  EventParameterContext *_localctx = _tracker.createInstance<EventParameterContext>(_ctx, getState());
  enterRule(_localctx, 58, SolidityParser::RuleEventParameter);
  size_t _la = 0;

  auto onExit = finally([=] {
    exitRule();
  });
  try {
    enterOuterAlt(_localctx, 1);
    setState(545);
    dynamic_cast<EventParameterContext *>(_localctx)->type = typeName(0);
    setState(547);
    _errHandler->sync(this);

    _la = _input->LA(1);
    if (_la == SolidityParser::Indexed) {
      setState(546);
      match(SolidityParser::Indexed);
    }
    setState(550);
    _errHandler->sync(this);

    _la = _input->LA(1);
    if (_la == SolidityParser::From || _la == SolidityParser::Identifier) {
      setState(549);
      dynamic_cast<EventParameterContext *>(_localctx)->name = identifier();
    }
   
  }
  catch (RecognitionException &e) {
    _errHandler->reportError(this, e);
    _localctx->exception = std::current_exception();
    _errHandler->recover(this, _localctx->exception);
  }

  return _localctx;
}

//----------------- EventDefinitionContext ------------------------------------------------------------------

SolidityParser::EventDefinitionContext::EventDefinitionContext(ParserRuleContext *parent, size_t invokingState)
  : ParserRuleContext(parent, invokingState) {
}

tree::TerminalNode* SolidityParser::EventDefinitionContext::Event() {
  return getToken(SolidityParser::Event, 0);
}

tree::TerminalNode* SolidityParser::EventDefinitionContext::LParen() {
  return getToken(SolidityParser::LParen, 0);
}

tree::TerminalNode* SolidityParser::EventDefinitionContext::RParen() {
  return getToken(SolidityParser::RParen, 0);
}

tree::TerminalNode* SolidityParser::EventDefinitionContext::Semicolon() {
  return getToken(SolidityParser::Semicolon, 0);
}

SolidityParser::IdentifierContext* SolidityParser::EventDefinitionContext::identifier() {
  return getRuleContext<SolidityParser::IdentifierContext>(0);
}

tree::TerminalNode* SolidityParser::EventDefinitionContext::Anonymous() {
  return getToken(SolidityParser::Anonymous, 0);
}

std::vector<SolidityParser::EventParameterContext *> SolidityParser::EventDefinitionContext::eventParameter() {
  return getRuleContexts<SolidityParser::EventParameterContext>();
}

SolidityParser::EventParameterContext* SolidityParser::EventDefinitionContext::eventParameter(size_t i) {
  return getRuleContext<SolidityParser::EventParameterContext>(i);
}

std::vector<tree::TerminalNode *> SolidityParser::EventDefinitionContext::Comma() {
  return getTokens(SolidityParser::Comma);
}

tree::TerminalNode* SolidityParser::EventDefinitionContext::Comma(size_t i) {
  return getToken(SolidityParser::Comma, i);
}


size_t SolidityParser::EventDefinitionContext::getRuleIndex() const {
  return SolidityParser::RuleEventDefinition;
}


antlrcpp::Any SolidityParser::EventDefinitionContext::accept(tree::ParseTreeVisitor *visitor) {
  if (auto parserVisitor = dynamic_cast<SolidityVisitor*>(visitor))
    return parserVisitor->visitEventDefinition(this);
  else
    return visitor->visitChildren(this);
}

SolidityParser::EventDefinitionContext* SolidityParser::eventDefinition() {
  EventDefinitionContext *_localctx = _tracker.createInstance<EventDefinitionContext>(_ctx, getState());
  enterRule(_localctx, 60, SolidityParser::RuleEventDefinition);
  size_t _la = 0;

  auto onExit = finally([=] {
    exitRule();
  });
  try {
    enterOuterAlt(_localctx, 1);
    setState(552);
    match(SolidityParser::Event);
    setState(553);
    dynamic_cast<EventDefinitionContext *>(_localctx)->name = identifier();
    setState(554);
    match(SolidityParser::LParen);
    setState(563);
    _errHandler->sync(this);

    switch (getInterpreter<atn::ParserATNSimulator>()->adaptivePredict(_input, 53, _ctx)) {
    case 1: {
      setState(555);
      dynamic_cast<EventDefinitionContext *>(_localctx)->eventParameterContext = eventParameter();
      dynamic_cast<EventDefinitionContext *>(_localctx)->parameters.push_back(dynamic_cast<EventDefinitionContext *>(_localctx)->eventParameterContext);
      setState(560);
      _errHandler->sync(this);
      _la = _input->LA(1);
      while (_la == SolidityParser::Comma) {
        setState(556);
        match(SolidityParser::Comma);
        setState(557);
        dynamic_cast<EventDefinitionContext *>(_localctx)->eventParameterContext = eventParameter();
        dynamic_cast<EventDefinitionContext *>(_localctx)->parameters.push_back(dynamic_cast<EventDefinitionContext *>(_localctx)->eventParameterContext);
        setState(562);
        _errHandler->sync(this);
        _la = _input->LA(1);
      }
      break;
    }

    }
    setState(565);
    match(SolidityParser::RParen);
    setState(567);
    _errHandler->sync(this);

    _la = _input->LA(1);
    if (_la == SolidityParser::Anonymous) {
      setState(566);
      match(SolidityParser::Anonymous);
    }
    setState(569);
    match(SolidityParser::Semicolon);
   
  }
  catch (RecognitionException &e) {
    _errHandler->reportError(this, e);
    _localctx->exception = std::current_exception();
    _errHandler->recover(this, _localctx->exception);
  }

  return _localctx;
}

//----------------- UsingDirectiveContext ------------------------------------------------------------------

SolidityParser::UsingDirectiveContext::UsingDirectiveContext(ParserRuleContext *parent, size_t invokingState)
  : ParserRuleContext(parent, invokingState) {
}

tree::TerminalNode* SolidityParser::UsingDirectiveContext::Using() {
  return getToken(SolidityParser::Using, 0);
}

SolidityParser::UserDefinedTypeNameContext* SolidityParser::UsingDirectiveContext::userDefinedTypeName() {
  return getRuleContext<SolidityParser::UserDefinedTypeNameContext>(0);
}

tree::TerminalNode* SolidityParser::UsingDirectiveContext::For() {
  return getToken(SolidityParser::For, 0);
}

tree::TerminalNode* SolidityParser::UsingDirectiveContext::Semicolon() {
  return getToken(SolidityParser::Semicolon, 0);
}

tree::TerminalNode* SolidityParser::UsingDirectiveContext::Mul() {
  return getToken(SolidityParser::Mul, 0);
}

SolidityParser::TypeNameContext* SolidityParser::UsingDirectiveContext::typeName() {
  return getRuleContext<SolidityParser::TypeNameContext>(0);
}


size_t SolidityParser::UsingDirectiveContext::getRuleIndex() const {
  return SolidityParser::RuleUsingDirective;
}


antlrcpp::Any SolidityParser::UsingDirectiveContext::accept(tree::ParseTreeVisitor *visitor) {
  if (auto parserVisitor = dynamic_cast<SolidityVisitor*>(visitor))
    return parserVisitor->visitUsingDirective(this);
  else
    return visitor->visitChildren(this);
}

SolidityParser::UsingDirectiveContext* SolidityParser::usingDirective() {
  UsingDirectiveContext *_localctx = _tracker.createInstance<UsingDirectiveContext>(_ctx, getState());
  enterRule(_localctx, 62, SolidityParser::RuleUsingDirective);

  auto onExit = finally([=] {
    exitRule();
  });
  try {
    enterOuterAlt(_localctx, 1);
    setState(571);
    match(SolidityParser::Using);
    setState(572);
    userDefinedTypeName();
    setState(573);
    match(SolidityParser::For);
    setState(576);
    _errHandler->sync(this);
    switch (getInterpreter<atn::ParserATNSimulator>()->adaptivePredict(_input, 55, _ctx)) {
    case 1: {
      setState(574);
      match(SolidityParser::Mul);
      break;
    }

    case 2: {
      setState(575);
      typeName(0);
      break;
    }

    }
    setState(578);
    match(SolidityParser::Semicolon);
   
  }
  catch (RecognitionException &e) {
    _errHandler->reportError(this, e);
    _localctx->exception = std::current_exception();
    _errHandler->recover(this, _localctx->exception);
  }

  return _localctx;
}

//----------------- TypeNameContext ------------------------------------------------------------------

SolidityParser::TypeNameContext::TypeNameContext(ParserRuleContext *parent, size_t invokingState)
  : ParserRuleContext(parent, invokingState) {
}

SolidityParser::ElementaryTypeNameContext* SolidityParser::TypeNameContext::elementaryTypeName() {
  return getRuleContext<SolidityParser::ElementaryTypeNameContext>(0);
}

SolidityParser::FunctionTypeNameContext* SolidityParser::TypeNameContext::functionTypeName() {
  return getRuleContext<SolidityParser::FunctionTypeNameContext>(0);
}

SolidityParser::MappingTypeContext* SolidityParser::TypeNameContext::mappingType() {
  return getRuleContext<SolidityParser::MappingTypeContext>(0);
}

SolidityParser::UserDefinedTypeNameContext* SolidityParser::TypeNameContext::userDefinedTypeName() {
  return getRuleContext<SolidityParser::UserDefinedTypeNameContext>(0);
}

SolidityParser::TypeNameContext* SolidityParser::TypeNameContext::typeName() {
  return getRuleContext<SolidityParser::TypeNameContext>(0);
}

tree::TerminalNode* SolidityParser::TypeNameContext::LBrack() {
  return getToken(SolidityParser::LBrack, 0);
}

tree::TerminalNode* SolidityParser::TypeNameContext::RBrack() {
  return getToken(SolidityParser::RBrack, 0);
}

SolidityParser::ExpressionContext* SolidityParser::TypeNameContext::expression() {
  return getRuleContext<SolidityParser::ExpressionContext>(0);
}


size_t SolidityParser::TypeNameContext::getRuleIndex() const {
  return SolidityParser::RuleTypeName;
}


antlrcpp::Any SolidityParser::TypeNameContext::accept(tree::ParseTreeVisitor *visitor) {
  if (auto parserVisitor = dynamic_cast<SolidityVisitor*>(visitor))
    return parserVisitor->visitTypeName(this);
  else
    return visitor->visitChildren(this);
}


SolidityParser::TypeNameContext* SolidityParser::typeName() {
   return typeName(0);
}

SolidityParser::TypeNameContext* SolidityParser::typeName(int precedence) {
  ParserRuleContext *parentContext = _ctx;
  size_t parentState = getState();
  SolidityParser::TypeNameContext *_localctx = _tracker.createInstance<TypeNameContext>(_ctx, parentState);
  SolidityParser::TypeNameContext *previousContext = _localctx;
  (void)previousContext; // Silence compiler, in case the context is not used by generated code.
  size_t startState = 64;
  enterRecursionRule(_localctx, 64, SolidityParser::RuleTypeName, precedence);

    

  auto onExit = finally([=] {
    unrollRecursionContexts(parentContext);
  });
  try {
    size_t alt;
    enterOuterAlt(_localctx, 1);
    setState(585);
    _errHandler->sync(this);
    switch (getInterpreter<atn::ParserATNSimulator>()->adaptivePredict(_input, 56, _ctx)) {
    case 1: {
      setState(581);
      elementaryTypeName(true);
      break;
    }

    case 2: {
      setState(582);
      functionTypeName();
      break;
    }

    case 3: {
      setState(583);
      mappingType();
      break;
    }

    case 4: {
      setState(584);
      userDefinedTypeName();
      break;
    }

    }
    _ctx->stop = _input->LT(-1);
    setState(595);
    _errHandler->sync(this);
    alt = getInterpreter<atn::ParserATNSimulator>()->adaptivePredict(_input, 58, _ctx);
    while (alt != 2 && alt != atn::ATN::INVALID_ALT_NUMBER) {
      if (alt == 1) {
        if (!_parseListeners.empty())
          triggerExitRuleEvent();
        previousContext = _localctx;
        _localctx = _tracker.createInstance<TypeNameContext>(parentContext, parentState);
        pushNewRecursionContext(_localctx, startState, RuleTypeName);
        setState(587);

        if (!(precpred(_ctx, 1))) throw FailedPredicateException(this, "precpred(_ctx, 1)");
        setState(588);
        match(SolidityParser::LBrack);
        setState(590);
        _errHandler->sync(this);

        switch (getInterpreter<atn::ParserATNSimulator>()->adaptivePredict(_input, 57, _ctx)) {
        case 1: {
          setState(589);
          expression(0);
          break;
        }

        }
        setState(592);
        match(SolidityParser::RBrack); 
      }
      setState(597);
      _errHandler->sync(this);
      alt = getInterpreter<atn::ParserATNSimulator>()->adaptivePredict(_input, 58, _ctx);
    }
  }
  catch (RecognitionException &e) {
    _errHandler->reportError(this, e);
    _localctx->exception = std::current_exception();
    _errHandler->recover(this, _localctx->exception);
  }
  return _localctx;
}

//----------------- ElementaryTypeNameContext ------------------------------------------------------------------

SolidityParser::ElementaryTypeNameContext::ElementaryTypeNameContext(ParserRuleContext *parent, size_t invokingState)
  : ParserRuleContext(parent, invokingState) {
}

SolidityParser::ElementaryTypeNameContext::ElementaryTypeNameContext(ParserRuleContext *parent, size_t invokingState, bool allowAddressPayable)
  : ParserRuleContext(parent, invokingState) {
  this->allowAddressPayable = allowAddressPayable;
}

tree::TerminalNode* SolidityParser::ElementaryTypeNameContext::Address() {
  return getToken(SolidityParser::Address, 0);
}

tree::TerminalNode* SolidityParser::ElementaryTypeNameContext::Payable() {
  return getToken(SolidityParser::Payable, 0);
}

tree::TerminalNode* SolidityParser::ElementaryTypeNameContext::Bool() {
  return getToken(SolidityParser::Bool, 0);
}

tree::TerminalNode* SolidityParser::ElementaryTypeNameContext::String() {
  return getToken(SolidityParser::String, 0);
}

tree::TerminalNode* SolidityParser::ElementaryTypeNameContext::Bytes() {
  return getToken(SolidityParser::Bytes, 0);
}

tree::TerminalNode* SolidityParser::ElementaryTypeNameContext::SignedIntegerType() {
  return getToken(SolidityParser::SignedIntegerType, 0);
}

tree::TerminalNode* SolidityParser::ElementaryTypeNameContext::UnsignedIntegerType() {
  return getToken(SolidityParser::UnsignedIntegerType, 0);
}

tree::TerminalNode* SolidityParser::ElementaryTypeNameContext::FixedBytes() {
  return getToken(SolidityParser::FixedBytes, 0);
}

tree::TerminalNode* SolidityParser::ElementaryTypeNameContext::Fixed() {
  return getToken(SolidityParser::Fixed, 0);
}

tree::TerminalNode* SolidityParser::ElementaryTypeNameContext::Ufixed() {
  return getToken(SolidityParser::Ufixed, 0);
}


size_t SolidityParser::ElementaryTypeNameContext::getRuleIndex() const {
  return SolidityParser::RuleElementaryTypeName;
}


antlrcpp::Any SolidityParser::ElementaryTypeNameContext::accept(tree::ParseTreeVisitor *visitor) {
  if (auto parserVisitor = dynamic_cast<SolidityVisitor*>(visitor))
    return parserVisitor->visitElementaryTypeName(this);
  else
    return visitor->visitChildren(this);
}

SolidityParser::ElementaryTypeNameContext* SolidityParser::elementaryTypeName(bool allowAddressPayable) {
  ElementaryTypeNameContext *_localctx = _tracker.createInstance<ElementaryTypeNameContext>(_ctx, getState(), allowAddressPayable);
  enterRule(_localctx, 66, SolidityParser::RuleElementaryTypeName);

  auto onExit = finally([=] {
    exitRule();
  });
  try {
    setState(610);
    _errHandler->sync(this);
    switch (getInterpreter<atn::ParserATNSimulator>()->adaptivePredict(_input, 59, _ctx)) {
    case 1: {
      enterOuterAlt(_localctx, 1);
      setState(598);
      match(SolidityParser::Address);
      break;
    }

    case 2: {
      enterOuterAlt(_localctx, 2);
      setState(599);

      if (!(_localctx->allowAddressPayable)) throw FailedPredicateException(this, "$allowAddressPayable");
      setState(600);
      match(SolidityParser::Address);
      setState(601);
      match(SolidityParser::Payable);
      break;
    }

    case 3: {
      enterOuterAlt(_localctx, 3);
      setState(602);
      match(SolidityParser::Bool);
      break;
    }

    case 4: {
      enterOuterAlt(_localctx, 4);
      setState(603);
      match(SolidityParser::String);
      break;
    }

    case 5: {
      enterOuterAlt(_localctx, 5);
      setState(604);
      match(SolidityParser::Bytes);
      break;
    }

    case 6: {
      enterOuterAlt(_localctx, 6);
      setState(605);
      match(SolidityParser::SignedIntegerType);
      break;
    }

    case 7: {
      enterOuterAlt(_localctx, 7);
      setState(606);
      match(SolidityParser::UnsignedIntegerType);
      break;
    }

    case 8: {
      enterOuterAlt(_localctx, 8);
      setState(607);
      match(SolidityParser::FixedBytes);
      break;
    }

    case 9: {
      enterOuterAlt(_localctx, 9);
      setState(608);
      match(SolidityParser::Fixed);
      break;
    }

    case 10: {
      enterOuterAlt(_localctx, 10);
      setState(609);
      match(SolidityParser::Ufixed);
      break;
    }

    }
   
  }
  catch (RecognitionException &e) {
    _errHandler->reportError(this, e);
    _localctx->exception = std::current_exception();
    _errHandler->recover(this, _localctx->exception);
  }

  return _localctx;
}

//----------------- FunctionTypeNameContext ------------------------------------------------------------------

SolidityParser::FunctionTypeNameContext::FunctionTypeNameContext(ParserRuleContext *parent, size_t invokingState)
  : ParserRuleContext(parent, invokingState) {
}

tree::TerminalNode* SolidityParser::FunctionTypeNameContext::Function() {
  return getToken(SolidityParser::Function, 0);
}

std::vector<tree::TerminalNode *> SolidityParser::FunctionTypeNameContext::LParen() {
  return getTokens(SolidityParser::LParen);
}

tree::TerminalNode* SolidityParser::FunctionTypeNameContext::LParen(size_t i) {
  return getToken(SolidityParser::LParen, i);
}

std::vector<tree::TerminalNode *> SolidityParser::FunctionTypeNameContext::RParen() {
  return getTokens(SolidityParser::RParen);
}

tree::TerminalNode* SolidityParser::FunctionTypeNameContext::RParen(size_t i) {
  return getToken(SolidityParser::RParen, i);
}

std::vector<SolidityParser::VisibilityContext *> SolidityParser::FunctionTypeNameContext::visibility() {
  return getRuleContexts<SolidityParser::VisibilityContext>();
}

SolidityParser::VisibilityContext* SolidityParser::FunctionTypeNameContext::visibility(size_t i) {
  return getRuleContext<SolidityParser::VisibilityContext>(i);
}

std::vector<SolidityParser::StateMutabilityContext *> SolidityParser::FunctionTypeNameContext::stateMutability() {
  return getRuleContexts<SolidityParser::StateMutabilityContext>();
}

SolidityParser::StateMutabilityContext* SolidityParser::FunctionTypeNameContext::stateMutability(size_t i) {
  return getRuleContext<SolidityParser::StateMutabilityContext>(i);
}

tree::TerminalNode* SolidityParser::FunctionTypeNameContext::Returns() {
  return getToken(SolidityParser::Returns, 0);
}

std::vector<SolidityParser::ParameterListContext *> SolidityParser::FunctionTypeNameContext::parameterList() {
  return getRuleContexts<SolidityParser::ParameterListContext>();
}

SolidityParser::ParameterListContext* SolidityParser::FunctionTypeNameContext::parameterList(size_t i) {
  return getRuleContext<SolidityParser::ParameterListContext>(i);
}


size_t SolidityParser::FunctionTypeNameContext::getRuleIndex() const {
  return SolidityParser::RuleFunctionTypeName;
}


antlrcpp::Any SolidityParser::FunctionTypeNameContext::accept(tree::ParseTreeVisitor *visitor) {
  if (auto parserVisitor = dynamic_cast<SolidityVisitor*>(visitor))
    return parserVisitor->visitFunctionTypeName(this);
  else
    return visitor->visitChildren(this);
}

SolidityParser::FunctionTypeNameContext* SolidityParser::functionTypeName() {
  FunctionTypeNameContext *_localctx = _tracker.createInstance<FunctionTypeNameContext>(_ctx, getState());
  enterRule(_localctx, 68, SolidityParser::RuleFunctionTypeName);

  auto onExit = finally([=] {
    exitRule();
  });
  try {
    size_t alt;
    enterOuterAlt(_localctx, 1);
    setState(612);
    match(SolidityParser::Function);
    setState(613);
    match(SolidityParser::LParen);
    setState(615);
    _errHandler->sync(this);

    switch (getInterpreter<atn::ParserATNSimulator>()->adaptivePredict(_input, 60, _ctx)) {
    case 1: {
      setState(614);
      dynamic_cast<FunctionTypeNameContext *>(_localctx)->arguments = parameterList();
      break;
    }

    }
    setState(617);
    match(SolidityParser::RParen);
    setState(628);
    _errHandler->sync(this);
    alt = getInterpreter<atn::ParserATNSimulator>()->adaptivePredict(_input, 62, _ctx);
    while (alt != 2 && alt != atn::ATN::INVALID_ALT_NUMBER) {
      if (alt == 1) {
        setState(626);
        _errHandler->sync(this);
        switch (getInterpreter<atn::ParserATNSimulator>()->adaptivePredict(_input, 61, _ctx)) {
        case 1: {
          setState(618);

          if (!(!_localctx->visibilitySet)) throw FailedPredicateException(this, "!$visibilitySet");
          setState(619);
          visibility();
          dynamic_cast<FunctionTypeNameContext *>(_localctx)->visibilitySet =  true;
          break;
        }

        case 2: {
          setState(622);

          if (!(!_localctx->mutabilitySet)) throw FailedPredicateException(this, "!$mutabilitySet");
          setState(623);
          stateMutability();
          dynamic_cast<FunctionTypeNameContext *>(_localctx)->mutabilitySet =  true;
          break;
        }

        } 
      }
      setState(630);
      _errHandler->sync(this);
      alt = getInterpreter<atn::ParserATNSimulator>()->adaptivePredict(_input, 62, _ctx);
    }
    setState(636);
    _errHandler->sync(this);

    switch (getInterpreter<atn::ParserATNSimulator>()->adaptivePredict(_input, 63, _ctx)) {
    case 1: {
      setState(631);
      match(SolidityParser::Returns);
      setState(632);
      match(SolidityParser::LParen);
      setState(633);
      dynamic_cast<FunctionTypeNameContext *>(_localctx)->returnParameters = parameterList();
      setState(634);
      match(SolidityParser::RParen);
      break;
    }

    }
   
  }
  catch (RecognitionException &e) {
    _errHandler->reportError(this, e);
    _localctx->exception = std::current_exception();
    _errHandler->recover(this, _localctx->exception);
  }

  return _localctx;
}

//----------------- VariableDeclarationContext ------------------------------------------------------------------

SolidityParser::VariableDeclarationContext::VariableDeclarationContext(ParserRuleContext *parent, size_t invokingState)
  : ParserRuleContext(parent, invokingState) {
}

SolidityParser::TypeNameContext* SolidityParser::VariableDeclarationContext::typeName() {
  return getRuleContext<SolidityParser::TypeNameContext>(0);
}

SolidityParser::IdentifierContext* SolidityParser::VariableDeclarationContext::identifier() {
  return getRuleContext<SolidityParser::IdentifierContext>(0);
}

SolidityParser::DataLocationContext* SolidityParser::VariableDeclarationContext::dataLocation() {
  return getRuleContext<SolidityParser::DataLocationContext>(0);
}


size_t SolidityParser::VariableDeclarationContext::getRuleIndex() const {
  return SolidityParser::RuleVariableDeclaration;
}


antlrcpp::Any SolidityParser::VariableDeclarationContext::accept(tree::ParseTreeVisitor *visitor) {
  if (auto parserVisitor = dynamic_cast<SolidityVisitor*>(visitor))
    return parserVisitor->visitVariableDeclaration(this);
  else
    return visitor->visitChildren(this);
}

SolidityParser::VariableDeclarationContext* SolidityParser::variableDeclaration() {
  VariableDeclarationContext *_localctx = _tracker.createInstance<VariableDeclarationContext>(_ctx, getState());
  enterRule(_localctx, 70, SolidityParser::RuleVariableDeclaration);
  size_t _la = 0;

  auto onExit = finally([=] {
    exitRule();
  });
  try {
    enterOuterAlt(_localctx, 1);
    setState(638);
    dynamic_cast<VariableDeclarationContext *>(_localctx)->type = typeName(0);
    setState(640);
    _errHandler->sync(this);

    _la = _input->LA(1);
    if ((((_la & ~ 0x3fULL) == 0) &&
      ((1ULL << _la) & ((1ULL << SolidityParser::Calldata)
      | (1ULL << SolidityParser::Memory)
      | (1ULL << SolidityParser::Storage))) != 0)) {
      setState(639);
      dynamic_cast<VariableDeclarationContext *>(_localctx)->location = dataLocation();
    }
    setState(642);
    dynamic_cast<VariableDeclarationContext *>(_localctx)->name = identifier();
   
  }
  catch (RecognitionException &e) {
    _errHandler->reportError(this, e);
    _localctx->exception = std::current_exception();
    _errHandler->recover(this, _localctx->exception);
  }

  return _localctx;
}

//----------------- DataLocationContext ------------------------------------------------------------------

SolidityParser::DataLocationContext::DataLocationContext(ParserRuleContext *parent, size_t invokingState)
  : ParserRuleContext(parent, invokingState) {
}

tree::TerminalNode* SolidityParser::DataLocationContext::Memory() {
  return getToken(SolidityParser::Memory, 0);
}

tree::TerminalNode* SolidityParser::DataLocationContext::Storage() {
  return getToken(SolidityParser::Storage, 0);
}

tree::TerminalNode* SolidityParser::DataLocationContext::Calldata() {
  return getToken(SolidityParser::Calldata, 0);
}


size_t SolidityParser::DataLocationContext::getRuleIndex() const {
  return SolidityParser::RuleDataLocation;
}


antlrcpp::Any SolidityParser::DataLocationContext::accept(tree::ParseTreeVisitor *visitor) {
  if (auto parserVisitor = dynamic_cast<SolidityVisitor*>(visitor))
    return parserVisitor->visitDataLocation(this);
  else
    return visitor->visitChildren(this);
}

SolidityParser::DataLocationContext* SolidityParser::dataLocation() {
  DataLocationContext *_localctx = _tracker.createInstance<DataLocationContext>(_ctx, getState());
  enterRule(_localctx, 72, SolidityParser::RuleDataLocation);
  size_t _la = 0;

  auto onExit = finally([=] {
    exitRule();
  });
  try {
    enterOuterAlt(_localctx, 1);
    setState(644);
    _la = _input->LA(1);
    if (!((((_la & ~ 0x3fULL) == 0) &&
      ((1ULL << _la) & ((1ULL << SolidityParser::Calldata)
      | (1ULL << SolidityParser::Memory)
      | (1ULL << SolidityParser::Storage))) != 0))) {
    _errHandler->recoverInline(this);
    }
    else {
      _errHandler->reportMatch(this);
      consume();
    }
   
  }
  catch (RecognitionException &e) {
    _errHandler->reportError(this, e);
    _localctx->exception = std::current_exception();
    _errHandler->recover(this, _localctx->exception);
  }

  return _localctx;
}

//----------------- ExpressionContext ------------------------------------------------------------------

SolidityParser::ExpressionContext::ExpressionContext(ParserRuleContext *parent, size_t invokingState)
  : ParserRuleContext(parent, invokingState) {
}


size_t SolidityParser::ExpressionContext::getRuleIndex() const {
  return SolidityParser::RuleExpression;
}

void SolidityParser::ExpressionContext::copyFrom(ExpressionContext *ctx) {
  ParserRuleContext::copyFrom(ctx);
}

//----------------- UnaryPrefixOperationContext ------------------------------------------------------------------

SolidityParser::ExpressionContext* SolidityParser::UnaryPrefixOperationContext::expression() {
  return getRuleContext<SolidityParser::ExpressionContext>(0);
}

tree::TerminalNode* SolidityParser::UnaryPrefixOperationContext::Inc() {
  return getToken(SolidityParser::Inc, 0);
}

tree::TerminalNode* SolidityParser::UnaryPrefixOperationContext::Dec() {
  return getToken(SolidityParser::Dec, 0);
}

tree::TerminalNode* SolidityParser::UnaryPrefixOperationContext::Not() {
  return getToken(SolidityParser::Not, 0);
}

tree::TerminalNode* SolidityParser::UnaryPrefixOperationContext::BitNot() {
  return getToken(SolidityParser::BitNot, 0);
}

tree::TerminalNode* SolidityParser::UnaryPrefixOperationContext::Delete() {
  return getToken(SolidityParser::Delete, 0);
}

tree::TerminalNode* SolidityParser::UnaryPrefixOperationContext::Sub() {
  return getToken(SolidityParser::Sub, 0);
}

SolidityParser::UnaryPrefixOperationContext::UnaryPrefixOperationContext(ExpressionContext *ctx) { copyFrom(ctx); }


antlrcpp::Any SolidityParser::UnaryPrefixOperationContext::accept(tree::ParseTreeVisitor *visitor) {
  if (auto parserVisitor = dynamic_cast<SolidityVisitor*>(visitor))
    return parserVisitor->visitUnaryPrefixOperation(this);
  else
    return visitor->visitChildren(this);
}
//----------------- PrimaryExpressionContext ------------------------------------------------------------------

SolidityParser::IdentifierContext* SolidityParser::PrimaryExpressionContext::identifier() {
  return getRuleContext<SolidityParser::IdentifierContext>(0);
}

SolidityParser::LiteralContext* SolidityParser::PrimaryExpressionContext::literal() {
  return getRuleContext<SolidityParser::LiteralContext>(0);
}

SolidityParser::ElementaryTypeNameContext* SolidityParser::PrimaryExpressionContext::elementaryTypeName() {
  return getRuleContext<SolidityParser::ElementaryTypeNameContext>(0);
}

SolidityParser::UserDefinedTypeNameContext* SolidityParser::PrimaryExpressionContext::userDefinedTypeName() {
  return getRuleContext<SolidityParser::UserDefinedTypeNameContext>(0);
}

SolidityParser::PrimaryExpressionContext::PrimaryExpressionContext(ExpressionContext *ctx) { copyFrom(ctx); }


antlrcpp::Any SolidityParser::PrimaryExpressionContext::accept(tree::ParseTreeVisitor *visitor) {
  if (auto parserVisitor = dynamic_cast<SolidityVisitor*>(visitor))
    return parserVisitor->visitPrimaryExpression(this);
  else
    return visitor->visitChildren(this);
}
//----------------- OrderComparisonContext ------------------------------------------------------------------

std::vector<SolidityParser::ExpressionContext *> SolidityParser::OrderComparisonContext::expression() {
  return getRuleContexts<SolidityParser::ExpressionContext>();
}

SolidityParser::ExpressionContext* SolidityParser::OrderComparisonContext::expression(size_t i) {
  return getRuleContext<SolidityParser::ExpressionContext>(i);
}

tree::TerminalNode* SolidityParser::OrderComparisonContext::LessThan() {
  return getToken(SolidityParser::LessThan, 0);
}

tree::TerminalNode* SolidityParser::OrderComparisonContext::GreaterThan() {
  return getToken(SolidityParser::GreaterThan, 0);
}

tree::TerminalNode* SolidityParser::OrderComparisonContext::LessThanOrEqual() {
  return getToken(SolidityParser::LessThanOrEqual, 0);
}

tree::TerminalNode* SolidityParser::OrderComparisonContext::GreaterThanOrEqual() {
  return getToken(SolidityParser::GreaterThanOrEqual, 0);
}

SolidityParser::OrderComparisonContext::OrderComparisonContext(ExpressionContext *ctx) { copyFrom(ctx); }


antlrcpp::Any SolidityParser::OrderComparisonContext::accept(tree::ParseTreeVisitor *visitor) {
  if (auto parserVisitor = dynamic_cast<SolidityVisitor*>(visitor))
    return parserVisitor->visitOrderComparison(this);
  else
    return visitor->visitChildren(this);
}
//----------------- ConditionalContext ------------------------------------------------------------------

std::vector<SolidityParser::ExpressionContext *> SolidityParser::ConditionalContext::expression() {
  return getRuleContexts<SolidityParser::ExpressionContext>();
}

SolidityParser::ExpressionContext* SolidityParser::ConditionalContext::expression(size_t i) {
  return getRuleContext<SolidityParser::ExpressionContext>(i);
}

tree::TerminalNode* SolidityParser::ConditionalContext::Conditional() {
  return getToken(SolidityParser::Conditional, 0);
}

tree::TerminalNode* SolidityParser::ConditionalContext::Colon() {
  return getToken(SolidityParser::Colon, 0);
}

SolidityParser::ConditionalContext::ConditionalContext(ExpressionContext *ctx) { copyFrom(ctx); }


antlrcpp::Any SolidityParser::ConditionalContext::accept(tree::ParseTreeVisitor *visitor) {
  if (auto parserVisitor = dynamic_cast<SolidityVisitor*>(visitor))
    return parserVisitor->visitConditional(this);
  else
    return visitor->visitChildren(this);
}
//----------------- PayableConversionContext ------------------------------------------------------------------

tree::TerminalNode* SolidityParser::PayableConversionContext::Payable() {
  return getToken(SolidityParser::Payable, 0);
}

SolidityParser::CallArgumentListContext* SolidityParser::PayableConversionContext::callArgumentList() {
  return getRuleContext<SolidityParser::CallArgumentListContext>(0);
}

SolidityParser::PayableConversionContext::PayableConversionContext(ExpressionContext *ctx) { copyFrom(ctx); }


antlrcpp::Any SolidityParser::PayableConversionContext::accept(tree::ParseTreeVisitor *visitor) {
  if (auto parserVisitor = dynamic_cast<SolidityVisitor*>(visitor))
    return parserVisitor->visitPayableConversion(this);
  else
    return visitor->visitChildren(this);
}
//----------------- AssignmentContext ------------------------------------------------------------------

std::vector<SolidityParser::ExpressionContext *> SolidityParser::AssignmentContext::expression() {
  return getRuleContexts<SolidityParser::ExpressionContext>();
}

SolidityParser::ExpressionContext* SolidityParser::AssignmentContext::expression(size_t i) {
  return getRuleContext<SolidityParser::ExpressionContext>(i);
}

SolidityParser::AssignOpContext* SolidityParser::AssignmentContext::assignOp() {
  return getRuleContext<SolidityParser::AssignOpContext>(0);
}

SolidityParser::AssignmentContext::AssignmentContext(ExpressionContext *ctx) { copyFrom(ctx); }


antlrcpp::Any SolidityParser::AssignmentContext::accept(tree::ParseTreeVisitor *visitor) {
  if (auto parserVisitor = dynamic_cast<SolidityVisitor*>(visitor))
    return parserVisitor->visitAssignment(this);
  else
    return visitor->visitChildren(this);
}
//----------------- UnarySuffixOperationContext ------------------------------------------------------------------

SolidityParser::ExpressionContext* SolidityParser::UnarySuffixOperationContext::expression() {
  return getRuleContext<SolidityParser::ExpressionContext>(0);
}

tree::TerminalNode* SolidityParser::UnarySuffixOperationContext::Inc() {
  return getToken(SolidityParser::Inc, 0);
}

tree::TerminalNode* SolidityParser::UnarySuffixOperationContext::Dec() {
  return getToken(SolidityParser::Dec, 0);
}

SolidityParser::UnarySuffixOperationContext::UnarySuffixOperationContext(ExpressionContext *ctx) { copyFrom(ctx); }


antlrcpp::Any SolidityParser::UnarySuffixOperationContext::accept(tree::ParseTreeVisitor *visitor) {
  if (auto parserVisitor = dynamic_cast<SolidityVisitor*>(visitor))
    return parserVisitor->visitUnarySuffixOperation(this);
  else
    return visitor->visitChildren(this);
}
//----------------- ShiftOperationContext ------------------------------------------------------------------

std::vector<SolidityParser::ExpressionContext *> SolidityParser::ShiftOperationContext::expression() {
  return getRuleContexts<SolidityParser::ExpressionContext>();
}

SolidityParser::ExpressionContext* SolidityParser::ShiftOperationContext::expression(size_t i) {
  return getRuleContext<SolidityParser::ExpressionContext>(i);
}

tree::TerminalNode* SolidityParser::ShiftOperationContext::Shl() {
  return getToken(SolidityParser::Shl, 0);
}

tree::TerminalNode* SolidityParser::ShiftOperationContext::Sar() {
  return getToken(SolidityParser::Sar, 0);
}

tree::TerminalNode* SolidityParser::ShiftOperationContext::Shr() {
  return getToken(SolidityParser::Shr, 0);
}

SolidityParser::ShiftOperationContext::ShiftOperationContext(ExpressionContext *ctx) { copyFrom(ctx); }


antlrcpp::Any SolidityParser::ShiftOperationContext::accept(tree::ParseTreeVisitor *visitor) {
  if (auto parserVisitor = dynamic_cast<SolidityVisitor*>(visitor))
    return parserVisitor->visitShiftOperation(this);
  else
    return visitor->visitChildren(this);
}
//----------------- BitAndOperationContext ------------------------------------------------------------------

std::vector<SolidityParser::ExpressionContext *> SolidityParser::BitAndOperationContext::expression() {
  return getRuleContexts<SolidityParser::ExpressionContext>();
}

SolidityParser::ExpressionContext* SolidityParser::BitAndOperationContext::expression(size_t i) {
  return getRuleContext<SolidityParser::ExpressionContext>(i);
}

tree::TerminalNode* SolidityParser::BitAndOperationContext::BitAnd() {
  return getToken(SolidityParser::BitAnd, 0);
}

SolidityParser::BitAndOperationContext::BitAndOperationContext(ExpressionContext *ctx) { copyFrom(ctx); }


antlrcpp::Any SolidityParser::BitAndOperationContext::accept(tree::ParseTreeVisitor *visitor) {
  if (auto parserVisitor = dynamic_cast<SolidityVisitor*>(visitor))
    return parserVisitor->visitBitAndOperation(this);
  else
    return visitor->visitChildren(this);
}
//----------------- FunctionCallContext ------------------------------------------------------------------

SolidityParser::ExpressionContext* SolidityParser::FunctionCallContext::expression() {
  return getRuleContext<SolidityParser::ExpressionContext>(0);
}

SolidityParser::CallArgumentListContext* SolidityParser::FunctionCallContext::callArgumentList() {
  return getRuleContext<SolidityParser::CallArgumentListContext>(0);
}

SolidityParser::FunctionCallContext::FunctionCallContext(ExpressionContext *ctx) { copyFrom(ctx); }


antlrcpp::Any SolidityParser::FunctionCallContext::accept(tree::ParseTreeVisitor *visitor) {
  if (auto parserVisitor = dynamic_cast<SolidityVisitor*>(visitor))
    return parserVisitor->visitFunctionCall(this);
  else
    return visitor->visitChildren(this);
}
//----------------- IndexRangeAccessContext ------------------------------------------------------------------

std::vector<SolidityParser::ExpressionContext *> SolidityParser::IndexRangeAccessContext::expression() {
  return getRuleContexts<SolidityParser::ExpressionContext>();
}

SolidityParser::ExpressionContext* SolidityParser::IndexRangeAccessContext::expression(size_t i) {
  return getRuleContext<SolidityParser::ExpressionContext>(i);
}

tree::TerminalNode* SolidityParser::IndexRangeAccessContext::LBrack() {
  return getToken(SolidityParser::LBrack, 0);
}

tree::TerminalNode* SolidityParser::IndexRangeAccessContext::Colon() {
  return getToken(SolidityParser::Colon, 0);
}

tree::TerminalNode* SolidityParser::IndexRangeAccessContext::RBrack() {
  return getToken(SolidityParser::RBrack, 0);
}

SolidityParser::IndexRangeAccessContext::IndexRangeAccessContext(ExpressionContext *ctx) { copyFrom(ctx); }


antlrcpp::Any SolidityParser::IndexRangeAccessContext::accept(tree::ParseTreeVisitor *visitor) {
  if (auto parserVisitor = dynamic_cast<SolidityVisitor*>(visitor))
    return parserVisitor->visitIndexRangeAccess(this);
  else
    return visitor->visitChildren(this);
}
//----------------- NewExpressionContext ------------------------------------------------------------------

tree::TerminalNode* SolidityParser::NewExpressionContext::New() {
  return getToken(SolidityParser::New, 0);
}

SolidityParser::TypeNameContext* SolidityParser::NewExpressionContext::typeName() {
  return getRuleContext<SolidityParser::TypeNameContext>(0);
}

SolidityParser::NewExpressionContext::NewExpressionContext(ExpressionContext *ctx) { copyFrom(ctx); }


antlrcpp::Any SolidityParser::NewExpressionContext::accept(tree::ParseTreeVisitor *visitor) {
  if (auto parserVisitor = dynamic_cast<SolidityVisitor*>(visitor))
    return parserVisitor->visitNewExpression(this);
  else
    return visitor->visitChildren(this);
}
//----------------- IndexAccessContext ------------------------------------------------------------------

std::vector<SolidityParser::ExpressionContext *> SolidityParser::IndexAccessContext::expression() {
  return getRuleContexts<SolidityParser::ExpressionContext>();
}

SolidityParser::ExpressionContext* SolidityParser::IndexAccessContext::expression(size_t i) {
  return getRuleContext<SolidityParser::ExpressionContext>(i);
}

tree::TerminalNode* SolidityParser::IndexAccessContext::LBrack() {
  return getToken(SolidityParser::LBrack, 0);
}

tree::TerminalNode* SolidityParser::IndexAccessContext::RBrack() {
  return getToken(SolidityParser::RBrack, 0);
}

SolidityParser::IndexAccessContext::IndexAccessContext(ExpressionContext *ctx) { copyFrom(ctx); }


antlrcpp::Any SolidityParser::IndexAccessContext::accept(tree::ParseTreeVisitor *visitor) {
  if (auto parserVisitor = dynamic_cast<SolidityVisitor*>(visitor))
    return parserVisitor->visitIndexAccess(this);
  else
    return visitor->visitChildren(this);
}
//----------------- AddSubOperationContext ------------------------------------------------------------------

std::vector<SolidityParser::ExpressionContext *> SolidityParser::AddSubOperationContext::expression() {
  return getRuleContexts<SolidityParser::ExpressionContext>();
}

SolidityParser::ExpressionContext* SolidityParser::AddSubOperationContext::expression(size_t i) {
  return getRuleContext<SolidityParser::ExpressionContext>(i);
}

tree::TerminalNode* SolidityParser::AddSubOperationContext::Add() {
  return getToken(SolidityParser::Add, 0);
}

tree::TerminalNode* SolidityParser::AddSubOperationContext::Sub() {
  return getToken(SolidityParser::Sub, 0);
}

SolidityParser::AddSubOperationContext::AddSubOperationContext(ExpressionContext *ctx) { copyFrom(ctx); }


antlrcpp::Any SolidityParser::AddSubOperationContext::accept(tree::ParseTreeVisitor *visitor) {
  if (auto parserVisitor = dynamic_cast<SolidityVisitor*>(visitor))
    return parserVisitor->visitAddSubOperation(this);
  else
    return visitor->visitChildren(this);
}
//----------------- BitOrOperationContext ------------------------------------------------------------------

std::vector<SolidityParser::ExpressionContext *> SolidityParser::BitOrOperationContext::expression() {
  return getRuleContexts<SolidityParser::ExpressionContext>();
}

SolidityParser::ExpressionContext* SolidityParser::BitOrOperationContext::expression(size_t i) {
  return getRuleContext<SolidityParser::ExpressionContext>(i);
}

tree::TerminalNode* SolidityParser::BitOrOperationContext::BitOr() {
  return getToken(SolidityParser::BitOr, 0);
}

SolidityParser::BitOrOperationContext::BitOrOperationContext(ExpressionContext *ctx) { copyFrom(ctx); }


antlrcpp::Any SolidityParser::BitOrOperationContext::accept(tree::ParseTreeVisitor *visitor) {
  if (auto parserVisitor = dynamic_cast<SolidityVisitor*>(visitor))
    return parserVisitor->visitBitOrOperation(this);
  else
    return visitor->visitChildren(this);
}
//----------------- ExpOperationContext ------------------------------------------------------------------

std::vector<SolidityParser::ExpressionContext *> SolidityParser::ExpOperationContext::expression() {
  return getRuleContexts<SolidityParser::ExpressionContext>();
}

SolidityParser::ExpressionContext* SolidityParser::ExpOperationContext::expression(size_t i) {
  return getRuleContext<SolidityParser::ExpressionContext>(i);
}

tree::TerminalNode* SolidityParser::ExpOperationContext::Exp() {
  return getToken(SolidityParser::Exp, 0);
}

SolidityParser::ExpOperationContext::ExpOperationContext(ExpressionContext *ctx) { copyFrom(ctx); }


antlrcpp::Any SolidityParser::ExpOperationContext::accept(tree::ParseTreeVisitor *visitor) {
  if (auto parserVisitor = dynamic_cast<SolidityVisitor*>(visitor))
    return parserVisitor->visitExpOperation(this);
  else
    return visitor->visitChildren(this);
}
//----------------- AndOperationContext ------------------------------------------------------------------

std::vector<SolidityParser::ExpressionContext *> SolidityParser::AndOperationContext::expression() {
  return getRuleContexts<SolidityParser::ExpressionContext>();
}

SolidityParser::ExpressionContext* SolidityParser::AndOperationContext::expression(size_t i) {
  return getRuleContext<SolidityParser::ExpressionContext>(i);
}

tree::TerminalNode* SolidityParser::AndOperationContext::And() {
  return getToken(SolidityParser::And, 0);
}

SolidityParser::AndOperationContext::AndOperationContext(ExpressionContext *ctx) { copyFrom(ctx); }


antlrcpp::Any SolidityParser::AndOperationContext::accept(tree::ParseTreeVisitor *visitor) {
  if (auto parserVisitor = dynamic_cast<SolidityVisitor*>(visitor))
    return parserVisitor->visitAndOperation(this);
  else
    return visitor->visitChildren(this);
}
//----------------- InlineArrayContext ------------------------------------------------------------------

SolidityParser::InlineArrayExpressionContext* SolidityParser::InlineArrayContext::inlineArrayExpression() {
  return getRuleContext<SolidityParser::InlineArrayExpressionContext>(0);
}

SolidityParser::InlineArrayContext::InlineArrayContext(ExpressionContext *ctx) { copyFrom(ctx); }


antlrcpp::Any SolidityParser::InlineArrayContext::accept(tree::ParseTreeVisitor *visitor) {
  if (auto parserVisitor = dynamic_cast<SolidityVisitor*>(visitor))
    return parserVisitor->visitInlineArray(this);
  else
    return visitor->visitChildren(this);
}
//----------------- OrOperationContext ------------------------------------------------------------------

std::vector<SolidityParser::ExpressionContext *> SolidityParser::OrOperationContext::expression() {
  return getRuleContexts<SolidityParser::ExpressionContext>();
}

SolidityParser::ExpressionContext* SolidityParser::OrOperationContext::expression(size_t i) {
  return getRuleContext<SolidityParser::ExpressionContext>(i);
}

tree::TerminalNode* SolidityParser::OrOperationContext::Or() {
  return getToken(SolidityParser::Or, 0);
}

SolidityParser::OrOperationContext::OrOperationContext(ExpressionContext *ctx) { copyFrom(ctx); }


antlrcpp::Any SolidityParser::OrOperationContext::accept(tree::ParseTreeVisitor *visitor) {
  if (auto parserVisitor = dynamic_cast<SolidityVisitor*>(visitor))
    return parserVisitor->visitOrOperation(this);
  else
    return visitor->visitChildren(this);
}
//----------------- MemberAccessContext ------------------------------------------------------------------

SolidityParser::ExpressionContext* SolidityParser::MemberAccessContext::expression() {
  return getRuleContext<SolidityParser::ExpressionContext>(0);
}

tree::TerminalNode* SolidityParser::MemberAccessContext::Period() {
  return getToken(SolidityParser::Period, 0);
}

SolidityParser::IdentifierContext* SolidityParser::MemberAccessContext::identifier() {
  return getRuleContext<SolidityParser::IdentifierContext>(0);
}

tree::TerminalNode* SolidityParser::MemberAccessContext::Address() {
  return getToken(SolidityParser::Address, 0);
}

SolidityParser::MemberAccessContext::MemberAccessContext(ExpressionContext *ctx) { copyFrom(ctx); }


antlrcpp::Any SolidityParser::MemberAccessContext::accept(tree::ParseTreeVisitor *visitor) {
  if (auto parserVisitor = dynamic_cast<SolidityVisitor*>(visitor))
    return parserVisitor->visitMemberAccess(this);
  else
    return visitor->visitChildren(this);
}
//----------------- MulDivModOperationContext ------------------------------------------------------------------

std::vector<SolidityParser::ExpressionContext *> SolidityParser::MulDivModOperationContext::expression() {
  return getRuleContexts<SolidityParser::ExpressionContext>();
}

SolidityParser::ExpressionContext* SolidityParser::MulDivModOperationContext::expression(size_t i) {
  return getRuleContext<SolidityParser::ExpressionContext>(i);
}

tree::TerminalNode* SolidityParser::MulDivModOperationContext::Mul() {
  return getToken(SolidityParser::Mul, 0);
}

tree::TerminalNode* SolidityParser::MulDivModOperationContext::Div() {
  return getToken(SolidityParser::Div, 0);
}

tree::TerminalNode* SolidityParser::MulDivModOperationContext::Mod() {
  return getToken(SolidityParser::Mod, 0);
}

SolidityParser::MulDivModOperationContext::MulDivModOperationContext(ExpressionContext *ctx) { copyFrom(ctx); }


antlrcpp::Any SolidityParser::MulDivModOperationContext::accept(tree::ParseTreeVisitor *visitor) {
  if (auto parserVisitor = dynamic_cast<SolidityVisitor*>(visitor))
    return parserVisitor->visitMulDivModOperation(this);
  else
    return visitor->visitChildren(this);
}
//----------------- FunctionCallOptionsContext ------------------------------------------------------------------

SolidityParser::ExpressionContext* SolidityParser::FunctionCallOptionsContext::expression() {
  return getRuleContext<SolidityParser::ExpressionContext>(0);
}

tree::TerminalNode* SolidityParser::FunctionCallOptionsContext::LBrace() {
  return getToken(SolidityParser::LBrace, 0);
}

tree::TerminalNode* SolidityParser::FunctionCallOptionsContext::RBrace() {
  return getToken(SolidityParser::RBrace, 0);
}

std::vector<SolidityParser::NamedArgumentContext *> SolidityParser::FunctionCallOptionsContext::namedArgument() {
  return getRuleContexts<SolidityParser::NamedArgumentContext>();
}

SolidityParser::NamedArgumentContext* SolidityParser::FunctionCallOptionsContext::namedArgument(size_t i) {
  return getRuleContext<SolidityParser::NamedArgumentContext>(i);
}

std::vector<tree::TerminalNode *> SolidityParser::FunctionCallOptionsContext::Comma() {
  return getTokens(SolidityParser::Comma);
}

tree::TerminalNode* SolidityParser::FunctionCallOptionsContext::Comma(size_t i) {
  return getToken(SolidityParser::Comma, i);
}

SolidityParser::FunctionCallOptionsContext::FunctionCallOptionsContext(ExpressionContext *ctx) { copyFrom(ctx); }


antlrcpp::Any SolidityParser::FunctionCallOptionsContext::accept(tree::ParseTreeVisitor *visitor) {
  if (auto parserVisitor = dynamic_cast<SolidityVisitor*>(visitor))
    return parserVisitor->visitFunctionCallOptions(this);
  else
    return visitor->visitChildren(this);
}
//----------------- BitXorOperationContext ------------------------------------------------------------------

std::vector<SolidityParser::ExpressionContext *> SolidityParser::BitXorOperationContext::expression() {
  return getRuleContexts<SolidityParser::ExpressionContext>();
}

SolidityParser::ExpressionContext* SolidityParser::BitXorOperationContext::expression(size_t i) {
  return getRuleContext<SolidityParser::ExpressionContext>(i);
}

tree::TerminalNode* SolidityParser::BitXorOperationContext::BitXor() {
  return getToken(SolidityParser::BitXor, 0);
}

SolidityParser::BitXorOperationContext::BitXorOperationContext(ExpressionContext *ctx) { copyFrom(ctx); }


antlrcpp::Any SolidityParser::BitXorOperationContext::accept(tree::ParseTreeVisitor *visitor) {
  if (auto parserVisitor = dynamic_cast<SolidityVisitor*>(visitor))
    return parserVisitor->visitBitXorOperation(this);
  else
    return visitor->visitChildren(this);
}
//----------------- TupleContext ------------------------------------------------------------------

SolidityParser::TupleExpressionContext* SolidityParser::TupleContext::tupleExpression() {
  return getRuleContext<SolidityParser::TupleExpressionContext>(0);
}

SolidityParser::TupleContext::TupleContext(ExpressionContext *ctx) { copyFrom(ctx); }


antlrcpp::Any SolidityParser::TupleContext::accept(tree::ParseTreeVisitor *visitor) {
  if (auto parserVisitor = dynamic_cast<SolidityVisitor*>(visitor))
    return parserVisitor->visitTuple(this);
  else
    return visitor->visitChildren(this);
}
//----------------- EqualityComparisonContext ------------------------------------------------------------------

std::vector<SolidityParser::ExpressionContext *> SolidityParser::EqualityComparisonContext::expression() {
  return getRuleContexts<SolidityParser::ExpressionContext>();
}

SolidityParser::ExpressionContext* SolidityParser::EqualityComparisonContext::expression(size_t i) {
  return getRuleContext<SolidityParser::ExpressionContext>(i);
}

tree::TerminalNode* SolidityParser::EqualityComparisonContext::Equal() {
  return getToken(SolidityParser::Equal, 0);
}

tree::TerminalNode* SolidityParser::EqualityComparisonContext::NotEqual() {
  return getToken(SolidityParser::NotEqual, 0);
}

SolidityParser::EqualityComparisonContext::EqualityComparisonContext(ExpressionContext *ctx) { copyFrom(ctx); }


antlrcpp::Any SolidityParser::EqualityComparisonContext::accept(tree::ParseTreeVisitor *visitor) {
  if (auto parserVisitor = dynamic_cast<SolidityVisitor*>(visitor))
    return parserVisitor->visitEqualityComparison(this);
  else
    return visitor->visitChildren(this);
}
//----------------- MetaTypeContext ------------------------------------------------------------------

tree::TerminalNode* SolidityParser::MetaTypeContext::Type() {
  return getToken(SolidityParser::Type, 0);
}

tree::TerminalNode* SolidityParser::MetaTypeContext::LParen() {
  return getToken(SolidityParser::LParen, 0);
}

SolidityParser::TypeNameContext* SolidityParser::MetaTypeContext::typeName() {
  return getRuleContext<SolidityParser::TypeNameContext>(0);
}

tree::TerminalNode* SolidityParser::MetaTypeContext::RParen() {
  return getToken(SolidityParser::RParen, 0);
}

SolidityParser::MetaTypeContext::MetaTypeContext(ExpressionContext *ctx) { copyFrom(ctx); }


antlrcpp::Any SolidityParser::MetaTypeContext::accept(tree::ParseTreeVisitor *visitor) {
  if (auto parserVisitor = dynamic_cast<SolidityVisitor*>(visitor))
    return parserVisitor->visitMetaType(this);
  else
    return visitor->visitChildren(this);
}

SolidityParser::ExpressionContext* SolidityParser::expression() {
   return expression(0);
}

SolidityParser::ExpressionContext* SolidityParser::expression(int precedence) {
  ParserRuleContext *parentContext = _ctx;
  size_t parentState = getState();
  SolidityParser::ExpressionContext *_localctx = _tracker.createInstance<ExpressionContext>(_ctx, parentState);
  SolidityParser::ExpressionContext *previousContext = _localctx;
  (void)previousContext; // Silence compiler, in case the context is not used by generated code.
  size_t startState = 74;
  enterRecursionRule(_localctx, 74, SolidityParser::RuleExpression, precedence);

    size_t _la = 0;

  auto onExit = finally([=] {
    unrollRecursionContexts(parentContext);
  });
  try {
    size_t alt;
    enterOuterAlt(_localctx, 1);
    setState(666);
    _errHandler->sync(this);
    switch (getInterpreter<atn::ParserATNSimulator>()->adaptivePredict(_input, 66, _ctx)) {
    case 1: {
      _localctx = _tracker.createInstance<PayableConversionContext>(_localctx);
      _ctx = _localctx;
      previousContext = _localctx;

      setState(647);
      match(SolidityParser::Payable);
      setState(648);
      callArgumentList();
      break;
    }

    case 2: {
      _localctx = _tracker.createInstance<MetaTypeContext>(_localctx);
      _ctx = _localctx;
      previousContext = _localctx;
      setState(649);
      match(SolidityParser::Type);
      setState(650);
      match(SolidityParser::LParen);
      setState(651);
      typeName(0);
      setState(652);
      match(SolidityParser::RParen);
      break;
    }

    case 3: {
      _localctx = _tracker.createInstance<UnaryPrefixOperationContext>(_localctx);
      _ctx = _localctx;
      previousContext = _localctx;
      setState(654);
      _la = _input->LA(1);
      if (!(_la == SolidityParser::Delete || ((((_la - 99) & ~ 0x3fULL) == 0) &&
        ((1ULL << (_la - 99)) & ((1ULL << (SolidityParser::Sub - 99))
        | (1ULL << (SolidityParser::Not - 99))
        | (1ULL << (SolidityParser::BitNot - 99))
        | (1ULL << (SolidityParser::Inc - 99))
        | (1ULL << (SolidityParser::Dec - 99)))) != 0))) {
      _errHandler->recoverInline(this);
      }
      else {
        _errHandler->reportMatch(this);
        consume();
      }
      setState(655);
      expression(19);
      break;
    }

    case 4: {
      _localctx = _tracker.createInstance<NewExpressionContext>(_localctx);
      _ctx = _localctx;
      previousContext = _localctx;
      setState(656);
      match(SolidityParser::New);
      setState(657);
      typeName(0);
      break;
    }

    case 5: {
      _localctx = _tracker.createInstance<TupleContext>(_localctx);
      _ctx = _localctx;
      previousContext = _localctx;
      setState(658);
      tupleExpression();
      break;
    }

    case 6: {
      _localctx = _tracker.createInstance<InlineArrayContext>(_localctx);
      _ctx = _localctx;
      previousContext = _localctx;
      setState(659);
      inlineArrayExpression();
      break;
    }

    case 7: {
      _localctx = _tracker.createInstance<PrimaryExpressionContext>(_localctx);
      _ctx = _localctx;
      previousContext = _localctx;
      setState(664);
      _errHandler->sync(this);
      switch (getInterpreter<atn::ParserATNSimulator>()->adaptivePredict(_input, 65, _ctx)) {
      case 1: {
        setState(660);
        identifier();
        break;
      }

      case 2: {
        setState(661);
        literal();
        break;
      }

      case 3: {
        setState(662);
        elementaryTypeName(false);
        break;
      }

      case 4: {
        setState(663);
        userDefinedTypeName();
        break;
      }

      }
      break;
    }

    }
    _ctx->stop = _input->LT(-1);
    setState(752);
    _errHandler->sync(this);
    alt = getInterpreter<atn::ParserATNSimulator>()->adaptivePredict(_input, 74, _ctx);
    while (alt != 2 && alt != atn::ATN::INVALID_ALT_NUMBER) {
      if (alt == 1) {
        if (!_parseListeners.empty())
          triggerExitRuleEvent();
        previousContext = _localctx;
        setState(750);
        _errHandler->sync(this);
        switch (getInterpreter<atn::ParserATNSimulator>()->adaptivePredict(_input, 73, _ctx)) {
        case 1: {
          auto newContext = _tracker.createInstance<ExpOperationContext>(_tracker.createInstance<ExpressionContext>(parentContext, parentState));
          _localctx = newContext;
          pushNewRecursionContext(newContext, startState, RuleExpression);
          setState(668);

          if (!(precpred(_ctx, 17))) throw FailedPredicateException(this, "precpred(_ctx, 17)");
          setState(669);
          match(SolidityParser::Exp);
          setState(670);
          expression(17);
          break;
        }

        case 2: {
          auto newContext = _tracker.createInstance<MulDivModOperationContext>(_tracker.createInstance<ExpressionContext>(parentContext, parentState));
          _localctx = newContext;
          pushNewRecursionContext(newContext, startState, RuleExpression);
          setState(671);

          if (!(precpred(_ctx, 16))) throw FailedPredicateException(this, "precpred(_ctx, 16)");
          setState(672);
          _la = _input->LA(1);
          if (!(((((_la - 100) & ~ 0x3fULL) == 0) &&
            ((1ULL << (_la - 100)) & ((1ULL << (SolidityParser::Mul - 100))
            | (1ULL << (SolidityParser::Div - 100))
            | (1ULL << (SolidityParser::Mod - 100)))) != 0))) {
          _errHandler->recoverInline(this);
          }
          else {
            _errHandler->reportMatch(this);
            consume();
          }
          setState(673);
          expression(17);
          break;
        }

        case 3: {
          auto newContext = _tracker.createInstance<AddSubOperationContext>(_tracker.createInstance<ExpressionContext>(parentContext, parentState));
          _localctx = newContext;
          pushNewRecursionContext(newContext, startState, RuleExpression);
          setState(674);

          if (!(precpred(_ctx, 15))) throw FailedPredicateException(this, "precpred(_ctx, 15)");
          setState(675);
          _la = _input->LA(1);
          if (!(_la == SolidityParser::Add

          || _la == SolidityParser::Sub)) {
          _errHandler->recoverInline(this);
          }
          else {
            _errHandler->reportMatch(this);
            consume();
          }
          setState(676);
          expression(16);
          break;
        }

        case 4: {
          auto newContext = _tracker.createInstance<ShiftOperationContext>(_tracker.createInstance<ExpressionContext>(parentContext, parentState));
          _localctx = newContext;
          pushNewRecursionContext(newContext, startState, RuleExpression);
          setState(677);

          if (!(precpred(_ctx, 14))) throw FailedPredicateException(this, "precpred(_ctx, 14)");
          setState(678);
          _la = _input->LA(1);
          if (!(((((_la - 95) & ~ 0x3fULL) == 0) &&
            ((1ULL << (_la - 95)) & ((1ULL << (SolidityParser::Shl - 95))
            | (1ULL << (SolidityParser::Sar - 95))
            | (1ULL << (SolidityParser::Shr - 95)))) != 0))) {
          _errHandler->recoverInline(this);
          }
          else {
            _errHandler->reportMatch(this);
            consume();
          }
          setState(679);
          expression(15);
          break;
        }

        case 5: {
          auto newContext = _tracker.createInstance<BitAndOperationContext>(_tracker.createInstance<ExpressionContext>(parentContext, parentState));
          _localctx = newContext;
          pushNewRecursionContext(newContext, startState, RuleExpression);
          setState(680);

          if (!(precpred(_ctx, 13))) throw FailedPredicateException(this, "precpred(_ctx, 13)");
          setState(681);
          match(SolidityParser::BitAnd);
          setState(682);
          expression(14);
          break;
        }

        case 6: {
          auto newContext = _tracker.createInstance<BitXorOperationContext>(_tracker.createInstance<ExpressionContext>(parentContext, parentState));
          _localctx = newContext;
          pushNewRecursionContext(newContext, startState, RuleExpression);
          setState(683);

          if (!(precpred(_ctx, 12))) throw FailedPredicateException(this, "precpred(_ctx, 12)");
          setState(684);
          match(SolidityParser::BitXor);
          setState(685);
          expression(13);
          break;
        }

        case 7: {
          auto newContext = _tracker.createInstance<BitOrOperationContext>(_tracker.createInstance<ExpressionContext>(parentContext, parentState));
          _localctx = newContext;
          pushNewRecursionContext(newContext, startState, RuleExpression);
          setState(686);

          if (!(precpred(_ctx, 11))) throw FailedPredicateException(this, "precpred(_ctx, 11)");
          setState(687);
          match(SolidityParser::BitOr);
          setState(688);
          expression(12);
          break;
        }

        case 8: {
          auto newContext = _tracker.createInstance<OrderComparisonContext>(_tracker.createInstance<ExpressionContext>(parentContext, parentState));
          _localctx = newContext;
          pushNewRecursionContext(newContext, startState, RuleExpression);
          setState(689);

          if (!(precpred(_ctx, 10))) throw FailedPredicateException(this, "precpred(_ctx, 10)");
          setState(690);
          _la = _input->LA(1);
          if (!(((((_la - 106) & ~ 0x3fULL) == 0) &&
            ((1ULL << (_la - 106)) & ((1ULL << (SolidityParser::LessThan - 106))
            | (1ULL << (SolidityParser::GreaterThan - 106))
            | (1ULL << (SolidityParser::LessThanOrEqual - 106))
            | (1ULL << (SolidityParser::GreaterThanOrEqual - 106)))) != 0))) {
          _errHandler->recoverInline(this);
          }
          else {
            _errHandler->reportMatch(this);
            consume();
          }
          setState(691);
          expression(11);
          break;
        }

        case 9: {
          auto newContext = _tracker.createInstance<EqualityComparisonContext>(_tracker.createInstance<ExpressionContext>(parentContext, parentState));
          _localctx = newContext;
          pushNewRecursionContext(newContext, startState, RuleExpression);
          setState(692);

          if (!(precpred(_ctx, 9))) throw FailedPredicateException(this, "precpred(_ctx, 9)");
          setState(693);
          _la = _input->LA(1);
          if (!(_la == SolidityParser::Equal

          || _la == SolidityParser::NotEqual)) {
          _errHandler->recoverInline(this);
          }
          else {
            _errHandler->reportMatch(this);
            consume();
          }
          setState(694);
          expression(10);
          break;
        }

        case 10: {
          auto newContext = _tracker.createInstance<AndOperationContext>(_tracker.createInstance<ExpressionContext>(parentContext, parentState));
          _localctx = newContext;
          pushNewRecursionContext(newContext, startState, RuleExpression);
          setState(695);

          if (!(precpred(_ctx, 8))) throw FailedPredicateException(this, "precpred(_ctx, 8)");
          setState(696);
          match(SolidityParser::And);
          setState(697);
          expression(9);
          break;
        }

        case 11: {
          auto newContext = _tracker.createInstance<OrOperationContext>(_tracker.createInstance<ExpressionContext>(parentContext, parentState));
          _localctx = newContext;
          pushNewRecursionContext(newContext, startState, RuleExpression);
          setState(698);

          if (!(precpred(_ctx, 7))) throw FailedPredicateException(this, "precpred(_ctx, 7)");
          setState(699);
          match(SolidityParser::Or);
          setState(700);
          expression(8);
          break;
        }

        case 12: {
          auto newContext = _tracker.createInstance<ConditionalContext>(_tracker.createInstance<ExpressionContext>(parentContext, parentState));
          _localctx = newContext;
          pushNewRecursionContext(newContext, startState, RuleExpression);
          setState(701);

          if (!(precpred(_ctx, 6))) throw FailedPredicateException(this, "precpred(_ctx, 6)");
          setState(702);
          match(SolidityParser::Conditional);
          setState(703);
          expression(0);
          setState(704);
          match(SolidityParser::Colon);
          setState(705);
          expression(6);
          break;
        }

        case 13: {
          auto newContext = _tracker.createInstance<AssignmentContext>(_tracker.createInstance<ExpressionContext>(parentContext, parentState));
          _localctx = newContext;
          pushNewRecursionContext(newContext, startState, RuleExpression);
          setState(707);

          if (!(precpred(_ctx, 5))) throw FailedPredicateException(this, "precpred(_ctx, 5)");
          setState(708);
          assignOp();
          setState(709);
          expression(5);
          break;
        }

        case 14: {
          auto newContext = _tracker.createInstance<IndexAccessContext>(_tracker.createInstance<ExpressionContext>(parentContext, parentState));
          _localctx = newContext;
          pushNewRecursionContext(newContext, startState, RuleExpression);
          setState(711);

          if (!(precpred(_ctx, 26))) throw FailedPredicateException(this, "precpred(_ctx, 26)");
          setState(712);
          match(SolidityParser::LBrack);
          setState(714);
          _errHandler->sync(this);

          switch (getInterpreter<atn::ParserATNSimulator>()->adaptivePredict(_input, 67, _ctx)) {
          case 1: {
            setState(713);
            dynamic_cast<IndexAccessContext *>(_localctx)->index = expression(0);
            break;
          }

          }
          setState(716);
          match(SolidityParser::RBrack);
          break;
        }

        case 15: {
          auto newContext = _tracker.createInstance<IndexRangeAccessContext>(_tracker.createInstance<ExpressionContext>(parentContext, parentState));
          _localctx = newContext;
          pushNewRecursionContext(newContext, startState, RuleExpression);
          setState(717);

          if (!(precpred(_ctx, 25))) throw FailedPredicateException(this, "precpred(_ctx, 25)");
          setState(718);
          match(SolidityParser::LBrack);
          setState(720);
          _errHandler->sync(this);

          switch (getInterpreter<atn::ParserATNSimulator>()->adaptivePredict(_input, 68, _ctx)) {
          case 1: {
            setState(719);
            dynamic_cast<IndexRangeAccessContext *>(_localctx)->start = expression(0);
            break;
          }

          }
          setState(722);
          match(SolidityParser::Colon);
          setState(724);
          _errHandler->sync(this);

          switch (getInterpreter<atn::ParserATNSimulator>()->adaptivePredict(_input, 69, _ctx)) {
          case 1: {
            setState(723);
            dynamic_cast<IndexRangeAccessContext *>(_localctx)->end = expression(0);
            break;
          }

          }
          setState(726);
          match(SolidityParser::RBrack);
          break;
        }

        case 16: {
          auto newContext = _tracker.createInstance<MemberAccessContext>(_tracker.createInstance<ExpressionContext>(parentContext, parentState));
          _localctx = newContext;
          pushNewRecursionContext(newContext, startState, RuleExpression);
          setState(727);

          if (!(precpred(_ctx, 24))) throw FailedPredicateException(this, "precpred(_ctx, 24)");
          setState(728);
          match(SolidityParser::Period);
          setState(731);
          _errHandler->sync(this);
          switch (_input->LA(1)) {
            case SolidityParser::From:
            case SolidityParser::Identifier: {
              setState(729);
              identifier();
              break;
            }

            case SolidityParser::Address: {
              setState(730);
              match(SolidityParser::Address);
              break;
            }

          default:
            throw NoViableAltException(this);
          }
          break;
        }

        case 17: {
          auto newContext = _tracker.createInstance<FunctionCallOptionsContext>(_tracker.createInstance<ExpressionContext>(parentContext, parentState));
          _localctx = newContext;
          pushNewRecursionContext(newContext, startState, RuleExpression);
          setState(733);

          if (!(precpred(_ctx, 23))) throw FailedPredicateException(this, "precpred(_ctx, 23)");
          setState(734);
          match(SolidityParser::LBrace);
          setState(743);
          _errHandler->sync(this);

          _la = _input->LA(1);
          if (_la == SolidityParser::From || _la == SolidityParser::Identifier) {
            setState(735);
            namedArgument();
            setState(740);
            _errHandler->sync(this);
            _la = _input->LA(1);
            while (_la == SolidityParser::Comma) {
              setState(736);
              match(SolidityParser::Comma);
              setState(737);
              namedArgument();
              setState(742);
              _errHandler->sync(this);
              _la = _input->LA(1);
            }
          }
          setState(745);
          match(SolidityParser::RBrace);
          break;
        }

        case 18: {
          auto newContext = _tracker.createInstance<FunctionCallContext>(_tracker.createInstance<ExpressionContext>(parentContext, parentState));
          _localctx = newContext;
          pushNewRecursionContext(newContext, startState, RuleExpression);
          setState(746);

          if (!(precpred(_ctx, 22))) throw FailedPredicateException(this, "precpred(_ctx, 22)");
          setState(747);
          callArgumentList();
          break;
        }

        case 19: {
          auto newContext = _tracker.createInstance<UnarySuffixOperationContext>(_tracker.createInstance<ExpressionContext>(parentContext, parentState));
          _localctx = newContext;
          pushNewRecursionContext(newContext, startState, RuleExpression);
          setState(748);

          if (!(precpred(_ctx, 18))) throw FailedPredicateException(this, "precpred(_ctx, 18)");
          setState(749);
          _la = _input->LA(1);
          if (!(_la == SolidityParser::Inc

          || _la == SolidityParser::Dec)) {
          _errHandler->recoverInline(this);
          }
          else {
            _errHandler->reportMatch(this);
            consume();
          }
          break;
        }

        } 
      }
      setState(754);
      _errHandler->sync(this);
      alt = getInterpreter<atn::ParserATNSimulator>()->adaptivePredict(_input, 74, _ctx);
    }
  }
  catch (RecognitionException &e) {
    _errHandler->reportError(this, e);
    _localctx->exception = std::current_exception();
    _errHandler->recover(this, _localctx->exception);
  }
  return _localctx;
}

//----------------- AssignOpContext ------------------------------------------------------------------

SolidityParser::AssignOpContext::AssignOpContext(ParserRuleContext *parent, size_t invokingState)
  : ParserRuleContext(parent, invokingState) {
}

tree::TerminalNode* SolidityParser::AssignOpContext::Assign() {
  return getToken(SolidityParser::Assign, 0);
}

tree::TerminalNode* SolidityParser::AssignOpContext::AssignBitOr() {
  return getToken(SolidityParser::AssignBitOr, 0);
}

tree::TerminalNode* SolidityParser::AssignOpContext::AssignBitXor() {
  return getToken(SolidityParser::AssignBitXor, 0);
}

tree::TerminalNode* SolidityParser::AssignOpContext::AssignBitAnd() {
  return getToken(SolidityParser::AssignBitAnd, 0);
}

tree::TerminalNode* SolidityParser::AssignOpContext::AssignShl() {
  return getToken(SolidityParser::AssignShl, 0);
}

tree::TerminalNode* SolidityParser::AssignOpContext::AssignSar() {
  return getToken(SolidityParser::AssignSar, 0);
}

tree::TerminalNode* SolidityParser::AssignOpContext::AssignShr() {
  return getToken(SolidityParser::AssignShr, 0);
}

tree::TerminalNode* SolidityParser::AssignOpContext::AssignAdd() {
  return getToken(SolidityParser::AssignAdd, 0);
}

tree::TerminalNode* SolidityParser::AssignOpContext::AssignSub() {
  return getToken(SolidityParser::AssignSub, 0);
}

tree::TerminalNode* SolidityParser::AssignOpContext::AssignMul() {
  return getToken(SolidityParser::AssignMul, 0);
}

tree::TerminalNode* SolidityParser::AssignOpContext::AssignDiv() {
  return getToken(SolidityParser::AssignDiv, 0);
}

tree::TerminalNode* SolidityParser::AssignOpContext::AssignMod() {
  return getToken(SolidityParser::AssignMod, 0);
}


size_t SolidityParser::AssignOpContext::getRuleIndex() const {
  return SolidityParser::RuleAssignOp;
}


antlrcpp::Any SolidityParser::AssignOpContext::accept(tree::ParseTreeVisitor *visitor) {
  if (auto parserVisitor = dynamic_cast<SolidityVisitor*>(visitor))
    return parserVisitor->visitAssignOp(this);
  else
    return visitor->visitChildren(this);
}

SolidityParser::AssignOpContext* SolidityParser::assignOp() {
  AssignOpContext *_localctx = _tracker.createInstance<AssignOpContext>(_ctx, getState());
  enterRule(_localctx, 76, SolidityParser::RuleAssignOp);
  size_t _la = 0;

  auto onExit = finally([=] {
    exitRule();
  });
  try {
    enterOuterAlt(_localctx, 1);
    setState(755);
    _la = _input->LA(1);
    if (!(((((_la - 77) & ~ 0x3fULL) == 0) &&
      ((1ULL << (_la - 77)) & ((1ULL << (SolidityParser::Assign - 77))
      | (1ULL << (SolidityParser::AssignBitOr - 77))
      | (1ULL << (SolidityParser::AssignBitXor - 77))
      | (1ULL << (SolidityParser::AssignBitAnd - 77))
      | (1ULL << (SolidityParser::AssignShl - 77))
      | (1ULL << (SolidityParser::AssignSar - 77))
      | (1ULL << (SolidityParser::AssignShr - 77))
      | (1ULL << (SolidityParser::AssignAdd - 77))
      | (1ULL << (SolidityParser::AssignSub - 77))
      | (1ULL << (SolidityParser::AssignMul - 77))
      | (1ULL << (SolidityParser::AssignDiv - 77))
      | (1ULL << (SolidityParser::AssignMod - 77)))) != 0))) {
    _errHandler->recoverInline(this);
    }
    else {
      _errHandler->reportMatch(this);
      consume();
    }
   
  }
  catch (RecognitionException &e) {
    _errHandler->reportError(this, e);
    _localctx->exception = std::current_exception();
    _errHandler->recover(this, _localctx->exception);
  }

  return _localctx;
}

//----------------- TupleExpressionContext ------------------------------------------------------------------

SolidityParser::TupleExpressionContext::TupleExpressionContext(ParserRuleContext *parent, size_t invokingState)
  : ParserRuleContext(parent, invokingState) {
}

tree::TerminalNode* SolidityParser::TupleExpressionContext::LParen() {
  return getToken(SolidityParser::LParen, 0);
}

tree::TerminalNode* SolidityParser::TupleExpressionContext::RParen() {
  return getToken(SolidityParser::RParen, 0);
}

std::vector<SolidityParser::ExpressionContext *> SolidityParser::TupleExpressionContext::expression() {
  return getRuleContexts<SolidityParser::ExpressionContext>();
}

SolidityParser::ExpressionContext* SolidityParser::TupleExpressionContext::expression(size_t i) {
  return getRuleContext<SolidityParser::ExpressionContext>(i);
}

std::vector<tree::TerminalNode *> SolidityParser::TupleExpressionContext::Comma() {
  return getTokens(SolidityParser::Comma);
}

tree::TerminalNode* SolidityParser::TupleExpressionContext::Comma(size_t i) {
  return getToken(SolidityParser::Comma, i);
}


size_t SolidityParser::TupleExpressionContext::getRuleIndex() const {
  return SolidityParser::RuleTupleExpression;
}


antlrcpp::Any SolidityParser::TupleExpressionContext::accept(tree::ParseTreeVisitor *visitor) {
  if (auto parserVisitor = dynamic_cast<SolidityVisitor*>(visitor))
    return parserVisitor->visitTupleExpression(this);
  else
    return visitor->visitChildren(this);
}

SolidityParser::TupleExpressionContext* SolidityParser::tupleExpression() {
  TupleExpressionContext *_localctx = _tracker.createInstance<TupleExpressionContext>(_ctx, getState());
  enterRule(_localctx, 78, SolidityParser::RuleTupleExpression);
  size_t _la = 0;

  auto onExit = finally([=] {
    exitRule();
  });
  try {
    enterOuterAlt(_localctx, 1);
    setState(757);
    match(SolidityParser::LParen);

    setState(759);
    _errHandler->sync(this);

    switch (getInterpreter<atn::ParserATNSimulator>()->adaptivePredict(_input, 75, _ctx)) {
    case 1: {
      setState(758);
      expression(0);
      break;
    }

    }
    setState(767);
    _errHandler->sync(this);
    _la = _input->LA(1);
    while (_la == SolidityParser::Comma) {
      setState(761);
      match(SolidityParser::Comma);
      setState(763);
      _errHandler->sync(this);

      switch (getInterpreter<atn::ParserATNSimulator>()->adaptivePredict(_input, 76, _ctx)) {
      case 1: {
        setState(762);
        expression(0);
        break;
      }

      }
      setState(769);
      _errHandler->sync(this);
      _la = _input->LA(1);
    }
    setState(770);
    match(SolidityParser::RParen);
   
  }
  catch (RecognitionException &e) {
    _errHandler->reportError(this, e);
    _localctx->exception = std::current_exception();
    _errHandler->recover(this, _localctx->exception);
  }

  return _localctx;
}

//----------------- InlineArrayExpressionContext ------------------------------------------------------------------

SolidityParser::InlineArrayExpressionContext::InlineArrayExpressionContext(ParserRuleContext *parent, size_t invokingState)
  : ParserRuleContext(parent, invokingState) {
}

tree::TerminalNode* SolidityParser::InlineArrayExpressionContext::LBrack() {
  return getToken(SolidityParser::LBrack, 0);
}

tree::TerminalNode* SolidityParser::InlineArrayExpressionContext::RBrack() {
  return getToken(SolidityParser::RBrack, 0);
}

std::vector<SolidityParser::ExpressionContext *> SolidityParser::InlineArrayExpressionContext::expression() {
  return getRuleContexts<SolidityParser::ExpressionContext>();
}

SolidityParser::ExpressionContext* SolidityParser::InlineArrayExpressionContext::expression(size_t i) {
  return getRuleContext<SolidityParser::ExpressionContext>(i);
}

std::vector<tree::TerminalNode *> SolidityParser::InlineArrayExpressionContext::Comma() {
  return getTokens(SolidityParser::Comma);
}

tree::TerminalNode* SolidityParser::InlineArrayExpressionContext::Comma(size_t i) {
  return getToken(SolidityParser::Comma, i);
}


size_t SolidityParser::InlineArrayExpressionContext::getRuleIndex() const {
  return SolidityParser::RuleInlineArrayExpression;
}


antlrcpp::Any SolidityParser::InlineArrayExpressionContext::accept(tree::ParseTreeVisitor *visitor) {
  if (auto parserVisitor = dynamic_cast<SolidityVisitor*>(visitor))
    return parserVisitor->visitInlineArrayExpression(this);
  else
    return visitor->visitChildren(this);
}

SolidityParser::InlineArrayExpressionContext* SolidityParser::inlineArrayExpression() {
  InlineArrayExpressionContext *_localctx = _tracker.createInstance<InlineArrayExpressionContext>(_ctx, getState());
  enterRule(_localctx, 80, SolidityParser::RuleInlineArrayExpression);
  size_t _la = 0;

  auto onExit = finally([=] {
    exitRule();
  });
  try {
    enterOuterAlt(_localctx, 1);
    setState(772);
    match(SolidityParser::LBrack);

    setState(773);
    expression(0);
    setState(778);
    _errHandler->sync(this);
    _la = _input->LA(1);
    while (_la == SolidityParser::Comma) {
      setState(774);
      match(SolidityParser::Comma);
      setState(775);
      expression(0);
      setState(780);
      _errHandler->sync(this);
      _la = _input->LA(1);
    }
    setState(781);
    match(SolidityParser::RBrack);
   
  }
  catch (RecognitionException &e) {
    _errHandler->reportError(this, e);
    _localctx->exception = std::current_exception();
    _errHandler->recover(this, _localctx->exception);
  }

  return _localctx;
}

//----------------- IdentifierContext ------------------------------------------------------------------

SolidityParser::IdentifierContext::IdentifierContext(ParserRuleContext *parent, size_t invokingState)
  : ParserRuleContext(parent, invokingState) {
}

tree::TerminalNode* SolidityParser::IdentifierContext::Identifier() {
  return getToken(SolidityParser::Identifier, 0);
}

tree::TerminalNode* SolidityParser::IdentifierContext::From() {
  return getToken(SolidityParser::From, 0);
}


size_t SolidityParser::IdentifierContext::getRuleIndex() const {
  return SolidityParser::RuleIdentifier;
}


antlrcpp::Any SolidityParser::IdentifierContext::accept(tree::ParseTreeVisitor *visitor) {
  if (auto parserVisitor = dynamic_cast<SolidityVisitor*>(visitor))
    return parserVisitor->visitIdentifier(this);
  else
    return visitor->visitChildren(this);
}

SolidityParser::IdentifierContext* SolidityParser::identifier() {
  IdentifierContext *_localctx = _tracker.createInstance<IdentifierContext>(_ctx, getState());
  enterRule(_localctx, 82, SolidityParser::RuleIdentifier);
  size_t _la = 0;

  auto onExit = finally([=] {
    exitRule();
  });
  try {
    enterOuterAlt(_localctx, 1);
    setState(783);
    _la = _input->LA(1);
    if (!(_la == SolidityParser::From || _la == SolidityParser::Identifier)) {
    _errHandler->recoverInline(this);
    }
    else {
      _errHandler->reportMatch(this);
      consume();
    }
   
  }
  catch (RecognitionException &e) {
    _errHandler->reportError(this, e);
    _localctx->exception = std::current_exception();
    _errHandler->recover(this, _localctx->exception);
  }

  return _localctx;
}

//----------------- LiteralContext ------------------------------------------------------------------

SolidityParser::LiteralContext::LiteralContext(ParserRuleContext *parent, size_t invokingState)
  : ParserRuleContext(parent, invokingState) {
}

SolidityParser::StringLiteralContext* SolidityParser::LiteralContext::stringLiteral() {
  return getRuleContext<SolidityParser::StringLiteralContext>(0);
}

SolidityParser::NumberLiteralContext* SolidityParser::LiteralContext::numberLiteral() {
  return getRuleContext<SolidityParser::NumberLiteralContext>(0);
}

SolidityParser::boolLiteralContext* SolidityParser::LiteralContext::boolLiteral() {
  return getRuleContext<SolidityParser::boolLiteralContext>(0);
}

SolidityParser::HexStringLiteralContext* SolidityParser::LiteralContext::hexStringLiteral() {
  return getRuleContext<SolidityParser::HexStringLiteralContext>(0);
}

SolidityParser::UnicodeStringLiteralContext* SolidityParser::LiteralContext::unicodeStringLiteral() {
  return getRuleContext<SolidityParser::UnicodeStringLiteralContext>(0);
}


size_t SolidityParser::LiteralContext::getRuleIndex() const {
  return SolidityParser::RuleLiteral;
}


antlrcpp::Any SolidityParser::LiteralContext::accept(tree::ParseTreeVisitor *visitor) {
  if (auto parserVisitor = dynamic_cast<SolidityVisitor*>(visitor))
    return parserVisitor->visitLiteral(this);
  else
    return visitor->visitChildren(this);
}

SolidityParser::LiteralContext* SolidityParser::literal() {
  LiteralContext *_localctx = _tracker.createInstance<LiteralContext>(_ctx, getState());
  enterRule(_localctx, 84, SolidityParser::RuleLiteral);

  auto onExit = finally([=] {
    exitRule();
  });
  try {
    setState(790);
    _errHandler->sync(this);
    switch (_input->LA(1)) {
      case SolidityParser::StringLiteral: {
        enterOuterAlt(_localctx, 1);
        setState(785);
        stringLiteral();
        break;
      }

      case SolidityParser::HexNumber:
      case SolidityParser::DecimalNumber: {
        enterOuterAlt(_localctx, 2);
        setState(786);
        numberLiteral();
        break;
      }

      case SolidityParser::False:
      case SolidityParser::True: {
        enterOuterAlt(_localctx, 3);
        setState(787);
        boolLiteral();
        break;
      }

      case SolidityParser::HexString: {
        enterOuterAlt(_localctx, 4);
        setState(788);
        hexStringLiteral();
        break;
      }

      case SolidityParser::UnicodeStringLiteral: {
        enterOuterAlt(_localctx, 5);
        setState(789);
        unicodeStringLiteral();
        break;
      }

    default:
      throw NoViableAltException(this);
    }
   
  }
  catch (RecognitionException &e) {
    _errHandler->reportError(this, e);
    _localctx->exception = std::current_exception();
    _errHandler->recover(this, _localctx->exception);
  }

  return _localctx;
}

//----------------- boolLiteralContext ------------------------------------------------------------------

SolidityParser::boolLiteralContext::boolLiteralContext(ParserRuleContext *parent, size_t invokingState)
  : ParserRuleContext(parent, invokingState) {
}

tree::TerminalNode* SolidityParser::boolLiteralContext::True() {
  return getToken(SolidityParser::True, 0);
}

tree::TerminalNode* SolidityParser::boolLiteralContext::False() {
  return getToken(SolidityParser::False, 0);
}


size_t SolidityParser::boolLiteralContext::getRuleIndex() const {
  return SolidityParser::RuleboolLiteral;
}


antlrcpp::Any SolidityParser::boolLiteralContext::accept(tree::ParseTreeVisitor *visitor) {
  if (auto parserVisitor = dynamic_cast<SolidityVisitor*>(visitor))
    return parserVisitor->visitBooleanLiteral(this);
  else
    return visitor->visitChildren(this);
}

SolidityParser::boolLiteralContext* SolidityParser::boolLiteral() {
  boolLiteralContext *_localctx = _tracker.createInstance<boolLiteralContext>(_ctx, getState());
  enterRule(_localctx, 86, SolidityParser::RuleboolLiteral);
  size_t _la = 0;

  auto onExit = finally([=] {
    exitRule();
  });
  try {
    enterOuterAlt(_localctx, 1);
    setState(792);
    _la = _input->LA(1);
    if (!(_la == SolidityParser::False

    || _la == SolidityParser::True)) {
    _errHandler->recoverInline(this);
    }
    else {
      _errHandler->reportMatch(this);
      consume();
    }
   
  }
  catch (RecognitionException &e) {
    _errHandler->reportError(this, e);
    _localctx->exception = std::current_exception();
    _errHandler->recover(this, _localctx->exception);
  }

  return _localctx;
}

//----------------- StringLiteralContext ------------------------------------------------------------------

SolidityParser::StringLiteralContext::StringLiteralContext(ParserRuleContext *parent, size_t invokingState)
  : ParserRuleContext(parent, invokingState) {
}

std::vector<tree::TerminalNode *> SolidityParser::StringLiteralContext::StringLiteral() {
  return getTokens(SolidityParser::StringLiteral);
}

tree::TerminalNode* SolidityParser::StringLiteralContext::StringLiteral(size_t i) {
  return getToken(SolidityParser::StringLiteral, i);
}


size_t SolidityParser::StringLiteralContext::getRuleIndex() const {
  return SolidityParser::RuleStringLiteral;
}


antlrcpp::Any SolidityParser::StringLiteralContext::accept(tree::ParseTreeVisitor *visitor) {
  if (auto parserVisitor = dynamic_cast<SolidityVisitor*>(visitor))
    return parserVisitor->visitStringLiteral(this);
  else
    return visitor->visitChildren(this);
}

SolidityParser::StringLiteralContext* SolidityParser::stringLiteral() {
  StringLiteralContext *_localctx = _tracker.createInstance<StringLiteralContext>(_ctx, getState());
  enterRule(_localctx, 88, SolidityParser::RuleStringLiteral);

  auto onExit = finally([=] {
    exitRule();
  });
  try {
    size_t alt;
    enterOuterAlt(_localctx, 1);
    setState(795); 
    _errHandler->sync(this);
    alt = 1;
    do {
      switch (alt) {
        case 1: {
              setState(794);
              match(SolidityParser::StringLiteral);
              break;
            }

      default:
        throw NoViableAltException(this);
      }
      setState(797); 
      _errHandler->sync(this);
      alt = getInterpreter<atn::ParserATNSimulator>()->adaptivePredict(_input, 80, _ctx);
    } while (alt != 2 && alt != atn::ATN::INVALID_ALT_NUMBER);
   
  }
  catch (RecognitionException &e) {
    _errHandler->reportError(this, e);
    _localctx->exception = std::current_exception();
    _errHandler->recover(this, _localctx->exception);
  }

  return _localctx;
}

//----------------- HexStringLiteralContext ------------------------------------------------------------------

SolidityParser::HexStringLiteralContext::HexStringLiteralContext(ParserRuleContext *parent, size_t invokingState)
  : ParserRuleContext(parent, invokingState) {
}

std::vector<tree::TerminalNode *> SolidityParser::HexStringLiteralContext::HexString() {
  return getTokens(SolidityParser::HexString);
}

tree::TerminalNode* SolidityParser::HexStringLiteralContext::HexString(size_t i) {
  return getToken(SolidityParser::HexString, i);
}


size_t SolidityParser::HexStringLiteralContext::getRuleIndex() const {
  return SolidityParser::RuleHexStringLiteral;
}


antlrcpp::Any SolidityParser::HexStringLiteralContext::accept(tree::ParseTreeVisitor *visitor) {
  if (auto parserVisitor = dynamic_cast<SolidityVisitor*>(visitor))
    return parserVisitor->visitHexStringLiteral(this);
  else
    return visitor->visitChildren(this);
}

SolidityParser::HexStringLiteralContext* SolidityParser::hexStringLiteral() {
  HexStringLiteralContext *_localctx = _tracker.createInstance<HexStringLiteralContext>(_ctx, getState());
  enterRule(_localctx, 90, SolidityParser::RuleHexStringLiteral);

  auto onExit = finally([=] {
    exitRule();
  });
  try {
    size_t alt;
    enterOuterAlt(_localctx, 1);
    setState(800); 
    _errHandler->sync(this);
    alt = 1;
    do {
      switch (alt) {
        case 1: {
              setState(799);
              match(SolidityParser::HexString);
              break;
            }

      default:
        throw NoViableAltException(this);
      }
      setState(802); 
      _errHandler->sync(this);
      alt = getInterpreter<atn::ParserATNSimulator>()->adaptivePredict(_input, 81, _ctx);
    } while (alt != 2 && alt != atn::ATN::INVALID_ALT_NUMBER);
   
  }
  catch (RecognitionException &e) {
    _errHandler->reportError(this, e);
    _localctx->exception = std::current_exception();
    _errHandler->recover(this, _localctx->exception);
  }

  return _localctx;
}

//----------------- UnicodeStringLiteralContext ------------------------------------------------------------------

SolidityParser::UnicodeStringLiteralContext::UnicodeStringLiteralContext(ParserRuleContext *parent, size_t invokingState)
  : ParserRuleContext(parent, invokingState) {
}

std::vector<tree::TerminalNode *> SolidityParser::UnicodeStringLiteralContext::UnicodeStringLiteral() {
  return getTokens(SolidityParser::UnicodeStringLiteral);
}

tree::TerminalNode* SolidityParser::UnicodeStringLiteralContext::UnicodeStringLiteral(size_t i) {
  return getToken(SolidityParser::UnicodeStringLiteral, i);
}


size_t SolidityParser::UnicodeStringLiteralContext::getRuleIndex() const {
  return SolidityParser::RuleUnicodeStringLiteral;
}


antlrcpp::Any SolidityParser::UnicodeStringLiteralContext::accept(tree::ParseTreeVisitor *visitor) {
  if (auto parserVisitor = dynamic_cast<SolidityVisitor*>(visitor))
    return parserVisitor->visitUnicodeStringLiteral(this);
  else
    return visitor->visitChildren(this);
}

SolidityParser::UnicodeStringLiteralContext* SolidityParser::unicodeStringLiteral() {
  UnicodeStringLiteralContext *_localctx = _tracker.createInstance<UnicodeStringLiteralContext>(_ctx, getState());
  enterRule(_localctx, 92, SolidityParser::RuleUnicodeStringLiteral);

  auto onExit = finally([=] {
    exitRule();
  });
  try {
    size_t alt;
    enterOuterAlt(_localctx, 1);
    setState(805); 
    _errHandler->sync(this);
    alt = 1;
    do {
      switch (alt) {
        case 1: {
              setState(804);
              match(SolidityParser::UnicodeStringLiteral);
              break;
            }

      default:
        throw NoViableAltException(this);
      }
      setState(807); 
      _errHandler->sync(this);
      alt = getInterpreter<atn::ParserATNSimulator>()->adaptivePredict(_input, 82, _ctx);
    } while (alt != 2 && alt != atn::ATN::INVALID_ALT_NUMBER);
   
  }
  catch (RecognitionException &e) {
    _errHandler->reportError(this, e);
    _localctx->exception = std::current_exception();
    _errHandler->recover(this, _localctx->exception);
  }

  return _localctx;
}

//----------------- NumberLiteralContext ------------------------------------------------------------------

SolidityParser::NumberLiteralContext::NumberLiteralContext(ParserRuleContext *parent, size_t invokingState)
  : ParserRuleContext(parent, invokingState) {
}

tree::TerminalNode* SolidityParser::NumberLiteralContext::DecimalNumber() {
  return getToken(SolidityParser::DecimalNumber, 0);
}

tree::TerminalNode* SolidityParser::NumberLiteralContext::HexNumber() {
  return getToken(SolidityParser::HexNumber, 0);
}

tree::TerminalNode* SolidityParser::NumberLiteralContext::NumberUnit() {
  return getToken(SolidityParser::NumberUnit, 0);
}


size_t SolidityParser::NumberLiteralContext::getRuleIndex() const {
  return SolidityParser::RuleNumberLiteral;
}


antlrcpp::Any SolidityParser::NumberLiteralContext::accept(tree::ParseTreeVisitor *visitor) {
  if (auto parserVisitor = dynamic_cast<SolidityVisitor*>(visitor))
    return parserVisitor->visitNumberLiteral(this);
  else
    return visitor->visitChildren(this);
}

SolidityParser::NumberLiteralContext* SolidityParser::numberLiteral() {
  NumberLiteralContext *_localctx = _tracker.createInstance<NumberLiteralContext>(_ctx, getState());
  enterRule(_localctx, 94, SolidityParser::RuleNumberLiteral);
  size_t _la = 0;

  auto onExit = finally([=] {
    exitRule();
  });
  try {
    enterOuterAlt(_localctx, 1);
    setState(809);
    _la = _input->LA(1);
    if (!(_la == SolidityParser::HexNumber

    || _la == SolidityParser::DecimalNumber)) {
    _errHandler->recoverInline(this);
    }
    else {
      _errHandler->reportMatch(this);
      consume();
    }
    setState(811);
    _errHandler->sync(this);

    switch (getInterpreter<atn::ParserATNSimulator>()->adaptivePredict(_input, 83, _ctx)) {
    case 1: {
      setState(810);
      match(SolidityParser::NumberUnit);
      break;
    }

    }
   
  }
  catch (RecognitionException &e) {
    _errHandler->reportError(this, e);
    _localctx->exception = std::current_exception();
    _errHandler->recover(this, _localctx->exception);
  }

  return _localctx;
}

//----------------- BlockContext ------------------------------------------------------------------

SolidityParser::BlockContext::BlockContext(ParserRuleContext *parent, size_t invokingState)
  : ParserRuleContext(parent, invokingState) {
}

tree::TerminalNode* SolidityParser::BlockContext::LBrace() {
  return getToken(SolidityParser::LBrace, 0);
}

tree::TerminalNode* SolidityParser::BlockContext::RBrace() {
  return getToken(SolidityParser::RBrace, 0);
}

std::vector<SolidityParser::StatementContext *> SolidityParser::BlockContext::statement() {
  return getRuleContexts<SolidityParser::StatementContext>();
}

SolidityParser::StatementContext* SolidityParser::BlockContext::statement(size_t i) {
  return getRuleContext<SolidityParser::StatementContext>(i);
}


size_t SolidityParser::BlockContext::getRuleIndex() const {
  return SolidityParser::RuleBlock;
}


antlrcpp::Any SolidityParser::BlockContext::accept(tree::ParseTreeVisitor *visitor) {
  if (auto parserVisitor = dynamic_cast<SolidityVisitor*>(visitor))
    return parserVisitor->visitBlock(this);
  else
    return visitor->visitChildren(this);
}

SolidityParser::BlockContext* SolidityParser::block() {
  BlockContext *_localctx = _tracker.createInstance<BlockContext>(_ctx, getState());
  enterRule(_localctx, 96, SolidityParser::RuleBlock);

  auto onExit = finally([=] {
    exitRule();
  });
  try {
    size_t alt;
    enterOuterAlt(_localctx, 1);
    setState(813);
    match(SolidityParser::LBrace);
    setState(817);
    _errHandler->sync(this);
    alt = getInterpreter<atn::ParserATNSimulator>()->adaptivePredict(_input, 84, _ctx);
    while (alt != 2 && alt != atn::ATN::INVALID_ALT_NUMBER) {
      if (alt == 1) {
        setState(814);
        statement(); 
      }
      setState(819);
      _errHandler->sync(this);
      alt = getInterpreter<atn::ParserATNSimulator>()->adaptivePredict(_input, 84, _ctx);
    }
    setState(820);
    match(SolidityParser::RBrace);
   
  }
  catch (RecognitionException &e) {
    _errHandler->reportError(this, e);
    _localctx->exception = std::current_exception();
    _errHandler->recover(this, _localctx->exception);
  }

  return _localctx;
}

//----------------- StatementContext ------------------------------------------------------------------

SolidityParser::StatementContext::StatementContext(ParserRuleContext *parent, size_t invokingState)
  : ParserRuleContext(parent, invokingState) {
}

SolidityParser::BlockContext* SolidityParser::StatementContext::block() {
  return getRuleContext<SolidityParser::BlockContext>(0);
}

SolidityParser::SimpleStatementContext* SolidityParser::StatementContext::simpleStatement() {
  return getRuleContext<SolidityParser::SimpleStatementContext>(0);
}

SolidityParser::IfStatementContext* SolidityParser::StatementContext::ifStatement() {
  return getRuleContext<SolidityParser::IfStatementContext>(0);
}

SolidityParser::ForStatementContext* SolidityParser::StatementContext::forStatement() {
  return getRuleContext<SolidityParser::ForStatementContext>(0);
}

SolidityParser::WhileStatementContext* SolidityParser::StatementContext::whileStatement() {
  return getRuleContext<SolidityParser::WhileStatementContext>(0);
}

SolidityParser::DoWhileStatementContext* SolidityParser::StatementContext::doWhileStatement() {
  return getRuleContext<SolidityParser::DoWhileStatementContext>(0);
}

SolidityParser::ContinueStatementContext* SolidityParser::StatementContext::continueStatement() {
  return getRuleContext<SolidityParser::ContinueStatementContext>(0);
}

SolidityParser::BreakStatementContext* SolidityParser::StatementContext::breakStatement() {
  return getRuleContext<SolidityParser::BreakStatementContext>(0);
}

SolidityParser::TryStatementContext* SolidityParser::StatementContext::tryStatement() {
  return getRuleContext<SolidityParser::TryStatementContext>(0);
}

SolidityParser::ReturnStatementContext* SolidityParser::StatementContext::returnStatement() {
  return getRuleContext<SolidityParser::ReturnStatementContext>(0);
}

SolidityParser::EmitStatementContext* SolidityParser::StatementContext::emitStatement() {
  return getRuleContext<SolidityParser::EmitStatementContext>(0);
}

SolidityParser::AssemblyStatementContext* SolidityParser::StatementContext::assemblyStatement() {
  return getRuleContext<SolidityParser::AssemblyStatementContext>(0);
}


size_t SolidityParser::StatementContext::getRuleIndex() const {
  return SolidityParser::RuleStatement;
}


antlrcpp::Any SolidityParser::StatementContext::accept(tree::ParseTreeVisitor *visitor) {
  if (auto parserVisitor = dynamic_cast<SolidityVisitor*>(visitor))
    return parserVisitor->visitStatement(this);
  else
    return visitor->visitChildren(this);
}

SolidityParser::StatementContext* SolidityParser::statement() {
  StatementContext *_localctx = _tracker.createInstance<StatementContext>(_ctx, getState());
  enterRule(_localctx, 98, SolidityParser::RuleStatement);

  auto onExit = finally([=] {
    exitRule();
  });
  try {
    setState(834);
    _errHandler->sync(this);
    switch (getInterpreter<atn::ParserATNSimulator>()->adaptivePredict(_input, 85, _ctx)) {
    case 1: {
      enterOuterAlt(_localctx, 1);
      setState(822);
      block();
      break;
    }

    case 2: {
      enterOuterAlt(_localctx, 2);
      setState(823);
      simpleStatement();
      break;
    }

    case 3: {
      enterOuterAlt(_localctx, 3);
      setState(824);
      ifStatement();
      break;
    }

    case 4: {
      enterOuterAlt(_localctx, 4);
      setState(825);
      forStatement();
      break;
    }

    case 5: {
      enterOuterAlt(_localctx, 5);
      setState(826);
      whileStatement();
      break;
    }

    case 6: {
      enterOuterAlt(_localctx, 6);
      setState(827);
      doWhileStatement();
      break;
    }

    case 7: {
      enterOuterAlt(_localctx, 7);
      setState(828);
      continueStatement();
      break;
    }

    case 8: {
      enterOuterAlt(_localctx, 8);
      setState(829);
      breakStatement();
      break;
    }

    case 9: {
      enterOuterAlt(_localctx, 9);
      setState(830);
      tryStatement();
      break;
    }

    case 10: {
      enterOuterAlt(_localctx, 10);
      setState(831);
      returnStatement();
      break;
    }

    case 11: {
      enterOuterAlt(_localctx, 11);
      setState(832);
      emitStatement();
      break;
    }

    case 12: {
      enterOuterAlt(_localctx, 12);
      setState(833);
      assemblyStatement();
      break;
    }

    }
   
  }
  catch (RecognitionException &e) {
    _errHandler->reportError(this, e);
    _localctx->exception = std::current_exception();
    _errHandler->recover(this, _localctx->exception);
  }

  return _localctx;
}

//----------------- SimpleStatementContext ------------------------------------------------------------------

SolidityParser::SimpleStatementContext::SimpleStatementContext(ParserRuleContext *parent, size_t invokingState)
  : ParserRuleContext(parent, invokingState) {
}

SolidityParser::VariableDeclarationStatementContext* SolidityParser::SimpleStatementContext::variableDeclarationStatement() {
  return getRuleContext<SolidityParser::VariableDeclarationStatementContext>(0);
}

SolidityParser::ExpressionStatementContext* SolidityParser::SimpleStatementContext::expressionStatement() {
  return getRuleContext<SolidityParser::ExpressionStatementContext>(0);
}


size_t SolidityParser::SimpleStatementContext::getRuleIndex() const {
  return SolidityParser::RuleSimpleStatement;
}


antlrcpp::Any SolidityParser::SimpleStatementContext::accept(tree::ParseTreeVisitor *visitor) {
  if (auto parserVisitor = dynamic_cast<SolidityVisitor*>(visitor))
    return parserVisitor->visitSimpleStatement(this);
  else
    return visitor->visitChildren(this);
}

SolidityParser::SimpleStatementContext* SolidityParser::simpleStatement() {
  SimpleStatementContext *_localctx = _tracker.createInstance<SimpleStatementContext>(_ctx, getState());
  enterRule(_localctx, 100, SolidityParser::RuleSimpleStatement);

  auto onExit = finally([=] {
    exitRule();
  });
  try {
    setState(838);
    _errHandler->sync(this);
    switch (getInterpreter<atn::ParserATNSimulator>()->adaptivePredict(_input, 86, _ctx)) {
    case 1: {
      enterOuterAlt(_localctx, 1);
      setState(836);
      variableDeclarationStatement();
      break;
    }

    case 2: {
      enterOuterAlt(_localctx, 2);
      setState(837);
      expressionStatement();
      break;
    }

    }
   
  }
  catch (RecognitionException &e) {
    _errHandler->reportError(this, e);
    _localctx->exception = std::current_exception();
    _errHandler->recover(this, _localctx->exception);
  }

  return _localctx;
}

//----------------- IfStatementContext ------------------------------------------------------------------

SolidityParser::IfStatementContext::IfStatementContext(ParserRuleContext *parent, size_t invokingState)
  : ParserRuleContext(parent, invokingState) {
}

tree::TerminalNode* SolidityParser::IfStatementContext::If() {
  return getToken(SolidityParser::If, 0);
}

tree::TerminalNode* SolidityParser::IfStatementContext::LParen() {
  return getToken(SolidityParser::LParen, 0);
}

SolidityParser::ExpressionContext* SolidityParser::IfStatementContext::expression() {
  return getRuleContext<SolidityParser::ExpressionContext>(0);
}

tree::TerminalNode* SolidityParser::IfStatementContext::RParen() {
  return getToken(SolidityParser::RParen, 0);
}

std::vector<SolidityParser::StatementContext *> SolidityParser::IfStatementContext::statement() {
  return getRuleContexts<SolidityParser::StatementContext>();
}

SolidityParser::StatementContext* SolidityParser::IfStatementContext::statement(size_t i) {
  return getRuleContext<SolidityParser::StatementContext>(i);
}

tree::TerminalNode* SolidityParser::IfStatementContext::Else() {
  return getToken(SolidityParser::Else, 0);
}


size_t SolidityParser::IfStatementContext::getRuleIndex() const {
  return SolidityParser::RuleIfStatement;
}


antlrcpp::Any SolidityParser::IfStatementContext::accept(tree::ParseTreeVisitor *visitor) {
  if (auto parserVisitor = dynamic_cast<SolidityVisitor*>(visitor))
    return parserVisitor->visitIfStatement(this);
  else
    return visitor->visitChildren(this);
}

SolidityParser::IfStatementContext* SolidityParser::ifStatement() {
  IfStatementContext *_localctx = _tracker.createInstance<IfStatementContext>(_ctx, getState());
  enterRule(_localctx, 102, SolidityParser::RuleIfStatement);

  auto onExit = finally([=] {
    exitRule();
  });
  try {
    enterOuterAlt(_localctx, 1);
    setState(840);
    match(SolidityParser::If);
    setState(841);
    match(SolidityParser::LParen);
    setState(842);
    expression(0);
    setState(843);
    match(SolidityParser::RParen);
    setState(844);
    statement();
    setState(847);
    _errHandler->sync(this);

    switch (getInterpreter<atn::ParserATNSimulator>()->adaptivePredict(_input, 87, _ctx)) {
    case 1: {
      setState(845);
      match(SolidityParser::Else);
      setState(846);
      statement();
      break;
    }

    }
   
  }
  catch (RecognitionException &e) {
    _errHandler->reportError(this, e);
    _localctx->exception = std::current_exception();
    _errHandler->recover(this, _localctx->exception);
  }

  return _localctx;
}

//----------------- ForStatementContext ------------------------------------------------------------------

SolidityParser::ForStatementContext::ForStatementContext(ParserRuleContext *parent, size_t invokingState)
  : ParserRuleContext(parent, invokingState) {
}

tree::TerminalNode* SolidityParser::ForStatementContext::For() {
  return getToken(SolidityParser::For, 0);
}

tree::TerminalNode* SolidityParser::ForStatementContext::LParen() {
  return getToken(SolidityParser::LParen, 0);
}

tree::TerminalNode* SolidityParser::ForStatementContext::RParen() {
  return getToken(SolidityParser::RParen, 0);
}

SolidityParser::StatementContext* SolidityParser::ForStatementContext::statement() {
  return getRuleContext<SolidityParser::StatementContext>(0);
}

SolidityParser::SimpleStatementContext* SolidityParser::ForStatementContext::simpleStatement() {
  return getRuleContext<SolidityParser::SimpleStatementContext>(0);
}

std::vector<tree::TerminalNode *> SolidityParser::ForStatementContext::Semicolon() {
  return getTokens(SolidityParser::Semicolon);
}

tree::TerminalNode* SolidityParser::ForStatementContext::Semicolon(size_t i) {
  return getToken(SolidityParser::Semicolon, i);
}

SolidityParser::ExpressionStatementContext* SolidityParser::ForStatementContext::expressionStatement() {
  return getRuleContext<SolidityParser::ExpressionStatementContext>(0);
}

SolidityParser::ExpressionContext* SolidityParser::ForStatementContext::expression() {
  return getRuleContext<SolidityParser::ExpressionContext>(0);
}


size_t SolidityParser::ForStatementContext::getRuleIndex() const {
  return SolidityParser::RuleForStatement;
}


antlrcpp::Any SolidityParser::ForStatementContext::accept(tree::ParseTreeVisitor *visitor) {
  if (auto parserVisitor = dynamic_cast<SolidityVisitor*>(visitor))
    return parserVisitor->visitForStatement(this);
  else
    return visitor->visitChildren(this);
}

SolidityParser::ForStatementContext* SolidityParser::forStatement() {
  ForStatementContext *_localctx = _tracker.createInstance<ForStatementContext>(_ctx, getState());
  enterRule(_localctx, 104, SolidityParser::RuleForStatement);

  auto onExit = finally([=] {
    exitRule();
  });
  try {
    enterOuterAlt(_localctx, 1);
    setState(849);
    match(SolidityParser::For);
    setState(850);
    match(SolidityParser::LParen);
    setState(853);
    _errHandler->sync(this);
    switch (getInterpreter<atn::ParserATNSimulator>()->adaptivePredict(_input, 88, _ctx)) {
    case 1: {
      setState(851);
      simpleStatement();
      break;
    }

    case 2: {
      setState(852);
      match(SolidityParser::Semicolon);
      break;
    }

    }
    setState(857);
    _errHandler->sync(this);
    switch (getInterpreter<atn::ParserATNSimulator>()->adaptivePredict(_input, 89, _ctx)) {
    case 1: {
      setState(855);
      expressionStatement();
      break;
    }

    case 2: {
      setState(856);
      match(SolidityParser::Semicolon);
      break;
    }

    }
    setState(860);
    _errHandler->sync(this);

    switch (getInterpreter<atn::ParserATNSimulator>()->adaptivePredict(_input, 90, _ctx)) {
    case 1: {
      setState(859);
      expression(0);
      break;
    }

    }
    setState(862);
    match(SolidityParser::RParen);
    setState(863);
    statement();
   
  }
  catch (RecognitionException &e) {
    _errHandler->reportError(this, e);
    _localctx->exception = std::current_exception();
    _errHandler->recover(this, _localctx->exception);
  }

  return _localctx;
}

//----------------- WhileStatementContext ------------------------------------------------------------------

SolidityParser::WhileStatementContext::WhileStatementContext(ParserRuleContext *parent, size_t invokingState)
  : ParserRuleContext(parent, invokingState) {
}

tree::TerminalNode* SolidityParser::WhileStatementContext::While() {
  return getToken(SolidityParser::While, 0);
}

tree::TerminalNode* SolidityParser::WhileStatementContext::LParen() {
  return getToken(SolidityParser::LParen, 0);
}

SolidityParser::ExpressionContext* SolidityParser::WhileStatementContext::expression() {
  return getRuleContext<SolidityParser::ExpressionContext>(0);
}

tree::TerminalNode* SolidityParser::WhileStatementContext::RParen() {
  return getToken(SolidityParser::RParen, 0);
}

SolidityParser::StatementContext* SolidityParser::WhileStatementContext::statement() {
  return getRuleContext<SolidityParser::StatementContext>(0);
}


size_t SolidityParser::WhileStatementContext::getRuleIndex() const {
  return SolidityParser::RuleWhileStatement;
}


antlrcpp::Any SolidityParser::WhileStatementContext::accept(tree::ParseTreeVisitor *visitor) {
  if (auto parserVisitor = dynamic_cast<SolidityVisitor*>(visitor))
    return parserVisitor->visitWhileStatement(this);
  else
    return visitor->visitChildren(this);
}

SolidityParser::WhileStatementContext* SolidityParser::whileStatement() {
  WhileStatementContext *_localctx = _tracker.createInstance<WhileStatementContext>(_ctx, getState());
  enterRule(_localctx, 106, SolidityParser::RuleWhileStatement);

  auto onExit = finally([=] {
    exitRule();
  });
  try {
    enterOuterAlt(_localctx, 1);
    setState(865);
    match(SolidityParser::While);
    setState(866);
    match(SolidityParser::LParen);
    setState(867);
    expression(0);
    setState(868);
    match(SolidityParser::RParen);
    setState(869);
    statement();
   
  }
  catch (RecognitionException &e) {
    _errHandler->reportError(this, e);
    _localctx->exception = std::current_exception();
    _errHandler->recover(this, _localctx->exception);
  }

  return _localctx;
}

//----------------- DoWhileStatementContext ------------------------------------------------------------------

SolidityParser::DoWhileStatementContext::DoWhileStatementContext(ParserRuleContext *parent, size_t invokingState)
  : ParserRuleContext(parent, invokingState) {
}

tree::TerminalNode* SolidityParser::DoWhileStatementContext::Do() {
  return getToken(SolidityParser::Do, 0);
}

SolidityParser::StatementContext* SolidityParser::DoWhileStatementContext::statement() {
  return getRuleContext<SolidityParser::StatementContext>(0);
}

tree::TerminalNode* SolidityParser::DoWhileStatementContext::While() {
  return getToken(SolidityParser::While, 0);
}

tree::TerminalNode* SolidityParser::DoWhileStatementContext::LParen() {
  return getToken(SolidityParser::LParen, 0);
}

SolidityParser::ExpressionContext* SolidityParser::DoWhileStatementContext::expression() {
  return getRuleContext<SolidityParser::ExpressionContext>(0);
}

tree::TerminalNode* SolidityParser::DoWhileStatementContext::RParen() {
  return getToken(SolidityParser::RParen, 0);
}

tree::TerminalNode* SolidityParser::DoWhileStatementContext::Semicolon() {
  return getToken(SolidityParser::Semicolon, 0);
}


size_t SolidityParser::DoWhileStatementContext::getRuleIndex() const {
  return SolidityParser::RuleDoWhileStatement;
}


antlrcpp::Any SolidityParser::DoWhileStatementContext::accept(tree::ParseTreeVisitor *visitor) {
  if (auto parserVisitor = dynamic_cast<SolidityVisitor*>(visitor))
    return parserVisitor->visitDoWhileStatement(this);
  else
    return visitor->visitChildren(this);
}

SolidityParser::DoWhileStatementContext* SolidityParser::doWhileStatement() {
  DoWhileStatementContext *_localctx = _tracker.createInstance<DoWhileStatementContext>(_ctx, getState());
  enterRule(_localctx, 108, SolidityParser::RuleDoWhileStatement);

  auto onExit = finally([=] {
    exitRule();
  });
  try {
    enterOuterAlt(_localctx, 1);
    setState(871);
    match(SolidityParser::Do);
    setState(872);
    statement();
    setState(873);
    match(SolidityParser::While);
    setState(874);
    match(SolidityParser::LParen);
    setState(875);
    expression(0);
    setState(876);
    match(SolidityParser::RParen);
    setState(877);
    match(SolidityParser::Semicolon);
   
  }
  catch (RecognitionException &e) {
    _errHandler->reportError(this, e);
    _localctx->exception = std::current_exception();
    _errHandler->recover(this, _localctx->exception);
  }

  return _localctx;
}

//----------------- ContinueStatementContext ------------------------------------------------------------------

SolidityParser::ContinueStatementContext::ContinueStatementContext(ParserRuleContext *parent, size_t invokingState)
  : ParserRuleContext(parent, invokingState) {
}

tree::TerminalNode* SolidityParser::ContinueStatementContext::Continue() {
  return getToken(SolidityParser::Continue, 0);
}

tree::TerminalNode* SolidityParser::ContinueStatementContext::Semicolon() {
  return getToken(SolidityParser::Semicolon, 0);
}


size_t SolidityParser::ContinueStatementContext::getRuleIndex() const {
  return SolidityParser::RuleContinueStatement;
}


antlrcpp::Any SolidityParser::ContinueStatementContext::accept(tree::ParseTreeVisitor *visitor) {
  if (auto parserVisitor = dynamic_cast<SolidityVisitor*>(visitor))
    return parserVisitor->visitContinueStatement(this);
  else
    return visitor->visitChildren(this);
}

SolidityParser::ContinueStatementContext* SolidityParser::continueStatement() {
  ContinueStatementContext *_localctx = _tracker.createInstance<ContinueStatementContext>(_ctx, getState());
  enterRule(_localctx, 110, SolidityParser::RuleContinueStatement);

  auto onExit = finally([=] {
    exitRule();
  });
  try {
    enterOuterAlt(_localctx, 1);
    setState(879);
    match(SolidityParser::Continue);
    setState(880);
    match(SolidityParser::Semicolon);
   
  }
  catch (RecognitionException &e) {
    _errHandler->reportError(this, e);
    _localctx->exception = std::current_exception();
    _errHandler->recover(this, _localctx->exception);
  }

  return _localctx;
}

//----------------- BreakStatementContext ------------------------------------------------------------------

SolidityParser::BreakStatementContext::BreakStatementContext(ParserRuleContext *parent, size_t invokingState)
  : ParserRuleContext(parent, invokingState) {
}

tree::TerminalNode* SolidityParser::BreakStatementContext::Break() {
  return getToken(SolidityParser::Break, 0);
}

tree::TerminalNode* SolidityParser::BreakStatementContext::Semicolon() {
  return getToken(SolidityParser::Semicolon, 0);
}


size_t SolidityParser::BreakStatementContext::getRuleIndex() const {
  return SolidityParser::RuleBreakStatement;
}


antlrcpp::Any SolidityParser::BreakStatementContext::accept(tree::ParseTreeVisitor *visitor) {
  if (auto parserVisitor = dynamic_cast<SolidityVisitor*>(visitor))
    return parserVisitor->visitBreakStatement(this);
  else
    return visitor->visitChildren(this);
}

SolidityParser::BreakStatementContext* SolidityParser::breakStatement() {
  BreakStatementContext *_localctx = _tracker.createInstance<BreakStatementContext>(_ctx, getState());
  enterRule(_localctx, 112, SolidityParser::RuleBreakStatement);

  auto onExit = finally([=] {
    exitRule();
  });
  try {
    enterOuterAlt(_localctx, 1);
    setState(882);
    match(SolidityParser::Break);
    setState(883);
    match(SolidityParser::Semicolon);
   
  }
  catch (RecognitionException &e) {
    _errHandler->reportError(this, e);
    _localctx->exception = std::current_exception();
    _errHandler->recover(this, _localctx->exception);
  }

  return _localctx;
}

//----------------- TryStatementContext ------------------------------------------------------------------

SolidityParser::TryStatementContext::TryStatementContext(ParserRuleContext *parent, size_t invokingState)
  : ParserRuleContext(parent, invokingState) {
}

tree::TerminalNode* SolidityParser::TryStatementContext::Try() {
  return getToken(SolidityParser::Try, 0);
}

SolidityParser::ExpressionContext* SolidityParser::TryStatementContext::expression() {
  return getRuleContext<SolidityParser::ExpressionContext>(0);
}

SolidityParser::BlockContext* SolidityParser::TryStatementContext::block() {
  return getRuleContext<SolidityParser::BlockContext>(0);
}

tree::TerminalNode* SolidityParser::TryStatementContext::Returns() {
  return getToken(SolidityParser::Returns, 0);
}

tree::TerminalNode* SolidityParser::TryStatementContext::LParen() {
  return getToken(SolidityParser::LParen, 0);
}

tree::TerminalNode* SolidityParser::TryStatementContext::RParen() {
  return getToken(SolidityParser::RParen, 0);
}

std::vector<SolidityParser::CatchClauseContext *> SolidityParser::TryStatementContext::catchClause() {
  return getRuleContexts<SolidityParser::CatchClauseContext>();
}

SolidityParser::CatchClauseContext* SolidityParser::TryStatementContext::catchClause(size_t i) {
  return getRuleContext<SolidityParser::CatchClauseContext>(i);
}

SolidityParser::ParameterListContext* SolidityParser::TryStatementContext::parameterList() {
  return getRuleContext<SolidityParser::ParameterListContext>(0);
}


size_t SolidityParser::TryStatementContext::getRuleIndex() const {
  return SolidityParser::RuleTryStatement;
}


antlrcpp::Any SolidityParser::TryStatementContext::accept(tree::ParseTreeVisitor *visitor) {
  if (auto parserVisitor = dynamic_cast<SolidityVisitor*>(visitor))
    return parserVisitor->visitTryStatement(this);
  else
    return visitor->visitChildren(this);
}

SolidityParser::TryStatementContext* SolidityParser::tryStatement() {
  TryStatementContext *_localctx = _tracker.createInstance<TryStatementContext>(_ctx, getState());
  enterRule(_localctx, 114, SolidityParser::RuleTryStatement);
  size_t _la = 0;

  auto onExit = finally([=] {
    exitRule();
  });
  try {
    size_t alt;
    enterOuterAlt(_localctx, 1);
    setState(885);
    match(SolidityParser::Try);
    setState(886);
    expression(0);
    setState(892);
    _errHandler->sync(this);

    _la = _input->LA(1);
    if (_la == SolidityParser::Returns) {
      setState(887);
      match(SolidityParser::Returns);
      setState(888);
      match(SolidityParser::LParen);
      setState(889);
      dynamic_cast<TryStatementContext *>(_localctx)->returnParameters = parameterList();
      setState(890);
      match(SolidityParser::RParen);
    }
    setState(894);
    block();
    setState(896); 
    _errHandler->sync(this);
    alt = 1;
    do {
      switch (alt) {
        case 1: {
              setState(895);
              catchClause();
              break;
            }

      default:
        throw NoViableAltException(this);
      }
      setState(898); 
      _errHandler->sync(this);
      alt = getInterpreter<atn::ParserATNSimulator>()->adaptivePredict(_input, 92, _ctx);
    } while (alt != 2 && alt != atn::ATN::INVALID_ALT_NUMBER);
   
  }
  catch (RecognitionException &e) {
    _errHandler->reportError(this, e);
    _localctx->exception = std::current_exception();
    _errHandler->recover(this, _localctx->exception);
  }

  return _localctx;
}

//----------------- CatchClauseContext ------------------------------------------------------------------

SolidityParser::CatchClauseContext::CatchClauseContext(ParserRuleContext *parent, size_t invokingState)
  : ParserRuleContext(parent, invokingState) {
}

tree::TerminalNode* SolidityParser::CatchClauseContext::Catch() {
  return getToken(SolidityParser::Catch, 0);
}

SolidityParser::BlockContext* SolidityParser::CatchClauseContext::block() {
  return getRuleContext<SolidityParser::BlockContext>(0);
}

tree::TerminalNode* SolidityParser::CatchClauseContext::LParen() {
  return getToken(SolidityParser::LParen, 0);
}

tree::TerminalNode* SolidityParser::CatchClauseContext::RParen() {
  return getToken(SolidityParser::RParen, 0);
}

SolidityParser::IdentifierContext* SolidityParser::CatchClauseContext::identifier() {
  return getRuleContext<SolidityParser::IdentifierContext>(0);
}

SolidityParser::ParameterListContext* SolidityParser::CatchClauseContext::parameterList() {
  return getRuleContext<SolidityParser::ParameterListContext>(0);
}


size_t SolidityParser::CatchClauseContext::getRuleIndex() const {
  return SolidityParser::RuleCatchClause;
}


antlrcpp::Any SolidityParser::CatchClauseContext::accept(tree::ParseTreeVisitor *visitor) {
  if (auto parserVisitor = dynamic_cast<SolidityVisitor*>(visitor))
    return parserVisitor->visitCatchClause(this);
  else
    return visitor->visitChildren(this);
}

SolidityParser::CatchClauseContext* SolidityParser::catchClause() {
  CatchClauseContext *_localctx = _tracker.createInstance<CatchClauseContext>(_ctx, getState());
  enterRule(_localctx, 116, SolidityParser::RuleCatchClause);
  size_t _la = 0;

  auto onExit = finally([=] {
    exitRule();
  });
  try {
    enterOuterAlt(_localctx, 1);
    setState(900);
    match(SolidityParser::Catch);
    setState(908);
    _errHandler->sync(this);

    _la = _input->LA(1);
    if (_la == SolidityParser::From || _la == SolidityParser::LParen

    || _la == SolidityParser::Identifier) {
      setState(902);
      _errHandler->sync(this);

      _la = _input->LA(1);
      if (_la == SolidityParser::From || _la == SolidityParser::Identifier) {
        setState(901);
        identifier();
      }
      setState(904);
      match(SolidityParser::LParen);

      setState(905);
      dynamic_cast<CatchClauseContext *>(_localctx)->arguments = parameterList();
      setState(906);
      match(SolidityParser::RParen);
    }
    setState(910);
    block();
   
  }
  catch (RecognitionException &e) {
    _errHandler->reportError(this, e);
    _localctx->exception = std::current_exception();
    _errHandler->recover(this, _localctx->exception);
  }

  return _localctx;
}

//----------------- ReturnStatementContext ------------------------------------------------------------------

SolidityParser::ReturnStatementContext::ReturnStatementContext(ParserRuleContext *parent, size_t invokingState)
  : ParserRuleContext(parent, invokingState) {
}

tree::TerminalNode* SolidityParser::ReturnStatementContext::Return() {
  return getToken(SolidityParser::Return, 0);
}

tree::TerminalNode* SolidityParser::ReturnStatementContext::Semicolon() {
  return getToken(SolidityParser::Semicolon, 0);
}

SolidityParser::ExpressionContext* SolidityParser::ReturnStatementContext::expression() {
  return getRuleContext<SolidityParser::ExpressionContext>(0);
}


size_t SolidityParser::ReturnStatementContext::getRuleIndex() const {
  return SolidityParser::RuleReturnStatement;
}


antlrcpp::Any SolidityParser::ReturnStatementContext::accept(tree::ParseTreeVisitor *visitor) {
  if (auto parserVisitor = dynamic_cast<SolidityVisitor*>(visitor))
    return parserVisitor->visitReturnStatement(this);
  else
    return visitor->visitChildren(this);
}

SolidityParser::ReturnStatementContext* SolidityParser::returnStatement() {
  ReturnStatementContext *_localctx = _tracker.createInstance<ReturnStatementContext>(_ctx, getState());
  enterRule(_localctx, 118, SolidityParser::RuleReturnStatement);

  auto onExit = finally([=] {
    exitRule();
  });
  try {
    enterOuterAlt(_localctx, 1);
    setState(912);
    match(SolidityParser::Return);
    setState(914);
    _errHandler->sync(this);

    switch (getInterpreter<atn::ParserATNSimulator>()->adaptivePredict(_input, 95, _ctx)) {
    case 1: {
      setState(913);
      expression(0);
      break;
    }

    }
    setState(916);
    match(SolidityParser::Semicolon);
   
  }
  catch (RecognitionException &e) {
    _errHandler->reportError(this, e);
    _localctx->exception = std::current_exception();
    _errHandler->recover(this, _localctx->exception);
  }

  return _localctx;
}

//----------------- EmitStatementContext ------------------------------------------------------------------

SolidityParser::EmitStatementContext::EmitStatementContext(ParserRuleContext *parent, size_t invokingState)
  : ParserRuleContext(parent, invokingState) {
}

tree::TerminalNode* SolidityParser::EmitStatementContext::Emit() {
  return getToken(SolidityParser::Emit, 0);
}

SolidityParser::ExpressionContext* SolidityParser::EmitStatementContext::expression() {
  return getRuleContext<SolidityParser::ExpressionContext>(0);
}

SolidityParser::CallArgumentListContext* SolidityParser::EmitStatementContext::callArgumentList() {
  return getRuleContext<SolidityParser::CallArgumentListContext>(0);
}

tree::TerminalNode* SolidityParser::EmitStatementContext::Semicolon() {
  return getToken(SolidityParser::Semicolon, 0);
}


size_t SolidityParser::EmitStatementContext::getRuleIndex() const {
  return SolidityParser::RuleEmitStatement;
}


antlrcpp::Any SolidityParser::EmitStatementContext::accept(tree::ParseTreeVisitor *visitor) {
  if (auto parserVisitor = dynamic_cast<SolidityVisitor*>(visitor))
    return parserVisitor->visitEmitStatement(this);
  else
    return visitor->visitChildren(this);
}

SolidityParser::EmitStatementContext* SolidityParser::emitStatement() {
  EmitStatementContext *_localctx = _tracker.createInstance<EmitStatementContext>(_ctx, getState());
  enterRule(_localctx, 120, SolidityParser::RuleEmitStatement);

  auto onExit = finally([=] {
    exitRule();
  });
  try {
    enterOuterAlt(_localctx, 1);
    setState(918);
    match(SolidityParser::Emit);
    setState(919);
    expression(0);
    setState(920);
    callArgumentList();
    setState(921);
    match(SolidityParser::Semicolon);
   
  }
  catch (RecognitionException &e) {
    _errHandler->reportError(this, e);
    _localctx->exception = std::current_exception();
    _errHandler->recover(this, _localctx->exception);
  }

  return _localctx;
}

//----------------- AssemblyStatementContext ------------------------------------------------------------------

SolidityParser::AssemblyStatementContext::AssemblyStatementContext(ParserRuleContext *parent, size_t invokingState)
  : ParserRuleContext(parent, invokingState) {
}

tree::TerminalNode* SolidityParser::AssemblyStatementContext::Assembly() {
  return getToken(SolidityParser::Assembly, 0);
}

tree::TerminalNode* SolidityParser::AssemblyStatementContext::AssemblyLBrace() {
  return getToken(SolidityParser::AssemblyLBrace, 0);
}

tree::TerminalNode* SolidityParser::AssemblyStatementContext::YulRBrace() {
  return getToken(SolidityParser::YulRBrace, 0);
}

tree::TerminalNode* SolidityParser::AssemblyStatementContext::AssemblyDialect() {
  return getToken(SolidityParser::AssemblyDialect, 0);
}

std::vector<SolidityParser::YulStatementContext *> SolidityParser::AssemblyStatementContext::yulStatement() {
  return getRuleContexts<SolidityParser::YulStatementContext>();
}

SolidityParser::YulStatementContext* SolidityParser::AssemblyStatementContext::yulStatement(size_t i) {
  return getRuleContext<SolidityParser::YulStatementContext>(i);
}


size_t SolidityParser::AssemblyStatementContext::getRuleIndex() const {
  return SolidityParser::RuleAssemblyStatement;
}


antlrcpp::Any SolidityParser::AssemblyStatementContext::accept(tree::ParseTreeVisitor *visitor) {
  if (auto parserVisitor = dynamic_cast<SolidityVisitor*>(visitor))
    return parserVisitor->visitAssemblyStatement(this);
  else
    return visitor->visitChildren(this);
}

SolidityParser::AssemblyStatementContext* SolidityParser::assemblyStatement() {
  AssemblyStatementContext *_localctx = _tracker.createInstance<AssemblyStatementContext>(_ctx, getState());
  enterRule(_localctx, 122, SolidityParser::RuleAssemblyStatement);
  size_t _la = 0;

  auto onExit = finally([=] {
    exitRule();
  });
  try {
    enterOuterAlt(_localctx, 1);
    setState(923);
    match(SolidityParser::Assembly);
    setState(925);
    _errHandler->sync(this);

    _la = _input->LA(1);
    if (_la == SolidityParser::AssemblyDialect) {
      setState(924);
      match(SolidityParser::AssemblyDialect);
    }
    setState(927);
    match(SolidityParser::AssemblyLBrace);
    setState(931);
    _errHandler->sync(this);
    _la = _input->LA(1);
    while (((((_la - 129) & ~ 0x3fULL) == 0) &&
      ((1ULL << (_la - 129)) & ((1ULL << (SolidityParser::YulBreak - 129))
      | (1ULL << (SolidityParser::YulContinue - 129))
      | (1ULL << (SolidityParser::YulFor - 129))
      | (1ULL << (SolidityParser::YulFunction - 129))
      | (1ULL << (SolidityParser::YulIf - 129))
      | (1ULL << (SolidityParser::YulLeave - 129))
      | (1ULL << (SolidityParser::YulLet - 129))
      | (1ULL << (SolidityParser::YulSwitch - 129))
      | (1ULL << (SolidityParser::YulEVMBuiltin - 129))
      | (1ULL << (SolidityParser::YulLBrace - 129))
      | (1ULL << (SolidityParser::YulIdentifier - 129)))) != 0)) {
      setState(928);
      yulStatement();
      setState(933);
      _errHandler->sync(this);
      _la = _input->LA(1);
    }
    setState(934);
    match(SolidityParser::YulRBrace);
   
  }
  catch (RecognitionException &e) {
    _errHandler->reportError(this, e);
    _localctx->exception = std::current_exception();
    _errHandler->recover(this, _localctx->exception);
  }

  return _localctx;
}

//----------------- VariableDeclarationListContext ------------------------------------------------------------------

SolidityParser::VariableDeclarationListContext::VariableDeclarationListContext(ParserRuleContext *parent, size_t invokingState)
  : ParserRuleContext(parent, invokingState) {
}

std::vector<SolidityParser::VariableDeclarationContext *> SolidityParser::VariableDeclarationListContext::variableDeclaration() {
  return getRuleContexts<SolidityParser::VariableDeclarationContext>();
}

SolidityParser::VariableDeclarationContext* SolidityParser::VariableDeclarationListContext::variableDeclaration(size_t i) {
  return getRuleContext<SolidityParser::VariableDeclarationContext>(i);
}

std::vector<tree::TerminalNode *> SolidityParser::VariableDeclarationListContext::Comma() {
  return getTokens(SolidityParser::Comma);
}

tree::TerminalNode* SolidityParser::VariableDeclarationListContext::Comma(size_t i) {
  return getToken(SolidityParser::Comma, i);
}


size_t SolidityParser::VariableDeclarationListContext::getRuleIndex() const {
  return SolidityParser::RuleVariableDeclarationList;
}


antlrcpp::Any SolidityParser::VariableDeclarationListContext::accept(tree::ParseTreeVisitor *visitor) {
  if (auto parserVisitor = dynamic_cast<SolidityVisitor*>(visitor))
    return parserVisitor->visitVariableDeclarationList(this);
  else
    return visitor->visitChildren(this);
}

SolidityParser::VariableDeclarationListContext* SolidityParser::variableDeclarationList() {
  VariableDeclarationListContext *_localctx = _tracker.createInstance<VariableDeclarationListContext>(_ctx, getState());
  enterRule(_localctx, 124, SolidityParser::RuleVariableDeclarationList);
  size_t _la = 0;

  auto onExit = finally([=] {
    exitRule();
  });
  try {
    enterOuterAlt(_localctx, 1);
    setState(936);
    dynamic_cast<VariableDeclarationListContext *>(_localctx)->variableDeclarationContext = variableDeclaration();
    dynamic_cast<VariableDeclarationListContext *>(_localctx)->variableDeclarations.push_back(dynamic_cast<VariableDeclarationListContext *>(_localctx)->variableDeclarationContext);
    setState(941);
    _errHandler->sync(this);
    _la = _input->LA(1);
    while (_la == SolidityParser::Comma) {
      setState(937);
      match(SolidityParser::Comma);
      setState(938);
      dynamic_cast<VariableDeclarationListContext *>(_localctx)->variableDeclarationContext = variableDeclaration();
      dynamic_cast<VariableDeclarationListContext *>(_localctx)->variableDeclarations.push_back(dynamic_cast<VariableDeclarationListContext *>(_localctx)->variableDeclarationContext);
      setState(943);
      _errHandler->sync(this);
      _la = _input->LA(1);
    }
   
  }
  catch (RecognitionException &e) {
    _errHandler->reportError(this, e);
    _localctx->exception = std::current_exception();
    _errHandler->recover(this, _localctx->exception);
  }

  return _localctx;
}

//----------------- VariableDeclarationTupleContext ------------------------------------------------------------------

SolidityParser::VariableDeclarationTupleContext::VariableDeclarationTupleContext(ParserRuleContext *parent, size_t invokingState)
  : ParserRuleContext(parent, invokingState) {
}

tree::TerminalNode* SolidityParser::VariableDeclarationTupleContext::LParen() {
  return getToken(SolidityParser::LParen, 0);
}

tree::TerminalNode* SolidityParser::VariableDeclarationTupleContext::RParen() {
  return getToken(SolidityParser::RParen, 0);
}

std::vector<SolidityParser::VariableDeclarationContext *> SolidityParser::VariableDeclarationTupleContext::variableDeclaration() {
  return getRuleContexts<SolidityParser::VariableDeclarationContext>();
}

SolidityParser::VariableDeclarationContext* SolidityParser::VariableDeclarationTupleContext::variableDeclaration(size_t i) {
  return getRuleContext<SolidityParser::VariableDeclarationContext>(i);
}

std::vector<tree::TerminalNode *> SolidityParser::VariableDeclarationTupleContext::Comma() {
  return getTokens(SolidityParser::Comma);
}

tree::TerminalNode* SolidityParser::VariableDeclarationTupleContext::Comma(size_t i) {
  return getToken(SolidityParser::Comma, i);
}


size_t SolidityParser::VariableDeclarationTupleContext::getRuleIndex() const {
  return SolidityParser::RuleVariableDeclarationTuple;
}


antlrcpp::Any SolidityParser::VariableDeclarationTupleContext::accept(tree::ParseTreeVisitor *visitor) {
  if (auto parserVisitor = dynamic_cast<SolidityVisitor*>(visitor))
    return parserVisitor->visitVariableDeclarationTuple(this);
  else
    return visitor->visitChildren(this);
}

SolidityParser::VariableDeclarationTupleContext* SolidityParser::variableDeclarationTuple() {
  VariableDeclarationTupleContext *_localctx = _tracker.createInstance<VariableDeclarationTupleContext>(_ctx, getState());
  enterRule(_localctx, 126, SolidityParser::RuleVariableDeclarationTuple);
  size_t _la = 0;

  auto onExit = finally([=] {
    exitRule();
  });
  try {
    size_t alt;
    enterOuterAlt(_localctx, 1);
    setState(944);
    match(SolidityParser::LParen);

    setState(948);
    _errHandler->sync(this);
    alt = getInterpreter<atn::ParserATNSimulator>()->adaptivePredict(_input, 99, _ctx);
    while (alt != 2 && alt != atn::ATN::INVALID_ALT_NUMBER) {
      if (alt == 1) {
        setState(945);
        match(SolidityParser::Comma); 
      }
      setState(950);
      _errHandler->sync(this);
      alt = getInterpreter<atn::ParserATNSimulator>()->adaptivePredict(_input, 99, _ctx);
    }
    setState(951);
    dynamic_cast<VariableDeclarationTupleContext *>(_localctx)->variableDeclarationContext = variableDeclaration();
    dynamic_cast<VariableDeclarationTupleContext *>(_localctx)->variableDeclarations.push_back(dynamic_cast<VariableDeclarationTupleContext *>(_localctx)->variableDeclarationContext);
    setState(959);
    _errHandler->sync(this);
    _la = _input->LA(1);
    while (_la == SolidityParser::Comma) {
      setState(953);
      match(SolidityParser::Comma);
      setState(955);
      _errHandler->sync(this);

      switch (getInterpreter<atn::ParserATNSimulator>()->adaptivePredict(_input, 100, _ctx)) {
      case 1: {
        setState(954);
        dynamic_cast<VariableDeclarationTupleContext *>(_localctx)->variableDeclarationContext = variableDeclaration();
        dynamic_cast<VariableDeclarationTupleContext *>(_localctx)->variableDeclarations.push_back(dynamic_cast<VariableDeclarationTupleContext *>(_localctx)->variableDeclarationContext);
        break;
      }

      }
      setState(961);
      _errHandler->sync(this);
      _la = _input->LA(1);
    }
    setState(962);
    match(SolidityParser::RParen);
   
  }
  catch (RecognitionException &e) {
    _errHandler->reportError(this, e);
    _localctx->exception = std::current_exception();
    _errHandler->recover(this, _localctx->exception);
  }

  return _localctx;
}

//----------------- VariableDeclarationStatementContext ------------------------------------------------------------------

SolidityParser::VariableDeclarationStatementContext::VariableDeclarationStatementContext(ParserRuleContext *parent, size_t invokingState)
  : ParserRuleContext(parent, invokingState) {
}

tree::TerminalNode* SolidityParser::VariableDeclarationStatementContext::Semicolon() {
  return getToken(SolidityParser::Semicolon, 0);
}

SolidityParser::VariableDeclarationContext* SolidityParser::VariableDeclarationStatementContext::variableDeclaration() {
  return getRuleContext<SolidityParser::VariableDeclarationContext>(0);
}

SolidityParser::VariableDeclarationTupleContext* SolidityParser::VariableDeclarationStatementContext::variableDeclarationTuple() {
  return getRuleContext<SolidityParser::VariableDeclarationTupleContext>(0);
}

tree::TerminalNode* SolidityParser::VariableDeclarationStatementContext::Assign() {
  return getToken(SolidityParser::Assign, 0);
}

SolidityParser::ExpressionContext* SolidityParser::VariableDeclarationStatementContext::expression() {
  return getRuleContext<SolidityParser::ExpressionContext>(0);
}


size_t SolidityParser::VariableDeclarationStatementContext::getRuleIndex() const {
  return SolidityParser::RuleVariableDeclarationStatement;
}


antlrcpp::Any SolidityParser::VariableDeclarationStatementContext::accept(tree::ParseTreeVisitor *visitor) {
  if (auto parserVisitor = dynamic_cast<SolidityVisitor*>(visitor))
    return parserVisitor->visitVariableDeclarationStatement(this);
  else
    return visitor->visitChildren(this);
}

SolidityParser::VariableDeclarationStatementContext* SolidityParser::variableDeclarationStatement() {
  VariableDeclarationStatementContext *_localctx = _tracker.createInstance<VariableDeclarationStatementContext>(_ctx, getState());
  enterRule(_localctx, 128, SolidityParser::RuleVariableDeclarationStatement);
  size_t _la = 0;

  auto onExit = finally([=] {
    exitRule();
  });
  try {
    enterOuterAlt(_localctx, 1);
    setState(973);
    _errHandler->sync(this);
    switch (getInterpreter<atn::ParserATNSimulator>()->adaptivePredict(_input, 103, _ctx)) {
    case 1: {
      setState(964);
      variableDeclaration();
      setState(967);
      _errHandler->sync(this);

      _la = _input->LA(1);
      if (_la == SolidityParser::Assign) {
        setState(965);
        match(SolidityParser::Assign);
        setState(966);
        expression(0);
      }
      break;
    }

    case 2: {
      setState(969);
      variableDeclarationTuple();
      setState(970);
      match(SolidityParser::Assign);
      setState(971);
      expression(0);
      break;
    }

    }
    setState(975);
    match(SolidityParser::Semicolon);
   
  }
  catch (RecognitionException &e) {
    _errHandler->reportError(this, e);
    _localctx->exception = std::current_exception();
    _errHandler->recover(this, _localctx->exception);
  }

  return _localctx;
}

//----------------- ExpressionStatementContext ------------------------------------------------------------------

SolidityParser::ExpressionStatementContext::ExpressionStatementContext(ParserRuleContext *parent, size_t invokingState)
  : ParserRuleContext(parent, invokingState) {
}

SolidityParser::ExpressionContext* SolidityParser::ExpressionStatementContext::expression() {
  return getRuleContext<SolidityParser::ExpressionContext>(0);
}

tree::TerminalNode* SolidityParser::ExpressionStatementContext::Semicolon() {
  return getToken(SolidityParser::Semicolon, 0);
}


size_t SolidityParser::ExpressionStatementContext::getRuleIndex() const {
  return SolidityParser::RuleExpressionStatement;
}


antlrcpp::Any SolidityParser::ExpressionStatementContext::accept(tree::ParseTreeVisitor *visitor) {
  if (auto parserVisitor = dynamic_cast<SolidityVisitor*>(visitor))
    return parserVisitor->visitExpressionStatement(this);
  else
    return visitor->visitChildren(this);
}

SolidityParser::ExpressionStatementContext* SolidityParser::expressionStatement() {
  ExpressionStatementContext *_localctx = _tracker.createInstance<ExpressionStatementContext>(_ctx, getState());
  enterRule(_localctx, 130, SolidityParser::RuleExpressionStatement);

  auto onExit = finally([=] {
    exitRule();
  });
  try {
    enterOuterAlt(_localctx, 1);
    setState(977);
    expression(0);
    setState(978);
    match(SolidityParser::Semicolon);
   
  }
  catch (RecognitionException &e) {
    _errHandler->reportError(this, e);
    _localctx->exception = std::current_exception();
    _errHandler->recover(this, _localctx->exception);
  }

  return _localctx;
}

//----------------- MappingTypeContext ------------------------------------------------------------------

SolidityParser::MappingTypeContext::MappingTypeContext(ParserRuleContext *parent, size_t invokingState)
  : ParserRuleContext(parent, invokingState) {
}

tree::TerminalNode* SolidityParser::MappingTypeContext::Mapping() {
  return getToken(SolidityParser::Mapping, 0);
}

tree::TerminalNode* SolidityParser::MappingTypeContext::LParen() {
  return getToken(SolidityParser::LParen, 0);
}

tree::TerminalNode* SolidityParser::MappingTypeContext::Arrow() {
  return getToken(SolidityParser::Arrow, 0);
}

tree::TerminalNode* SolidityParser::MappingTypeContext::RParen() {
  return getToken(SolidityParser::RParen, 0);
}

SolidityParser::MappingKeyTypeContext* SolidityParser::MappingTypeContext::mappingKeyType() {
  return getRuleContext<SolidityParser::MappingKeyTypeContext>(0);
}

SolidityParser::TypeNameContext* SolidityParser::MappingTypeContext::typeName() {
  return getRuleContext<SolidityParser::TypeNameContext>(0);
}


size_t SolidityParser::MappingTypeContext::getRuleIndex() const {
  return SolidityParser::RuleMappingType;
}


antlrcpp::Any SolidityParser::MappingTypeContext::accept(tree::ParseTreeVisitor *visitor) {
  if (auto parserVisitor = dynamic_cast<SolidityVisitor*>(visitor))
    return parserVisitor->visitMappingType(this);
  else
    return visitor->visitChildren(this);
}

SolidityParser::MappingTypeContext* SolidityParser::mappingType() {
  MappingTypeContext *_localctx = _tracker.createInstance<MappingTypeContext>(_ctx, getState());
  enterRule(_localctx, 132, SolidityParser::RuleMappingType);

  auto onExit = finally([=] {
    exitRule();
  });
  try {
    enterOuterAlt(_localctx, 1);
    setState(980);
    match(SolidityParser::Mapping);
    setState(981);
    match(SolidityParser::LParen);
    setState(982);
    dynamic_cast<MappingTypeContext *>(_localctx)->key = mappingKeyType();
    setState(983);
    match(SolidityParser::Arrow);
    setState(984);
    dynamic_cast<MappingTypeContext *>(_localctx)->value = typeName(0);
    setState(985);
    match(SolidityParser::RParen);
   
  }
  catch (RecognitionException &e) {
    _errHandler->reportError(this, e);
    _localctx->exception = std::current_exception();
    _errHandler->recover(this, _localctx->exception);
  }

  return _localctx;
}

//----------------- MappingKeyTypeContext ------------------------------------------------------------------

SolidityParser::MappingKeyTypeContext::MappingKeyTypeContext(ParserRuleContext *parent, size_t invokingState)
  : ParserRuleContext(parent, invokingState) {
}

SolidityParser::ElementaryTypeNameContext* SolidityParser::MappingKeyTypeContext::elementaryTypeName() {
  return getRuleContext<SolidityParser::ElementaryTypeNameContext>(0);
}

SolidityParser::UserDefinedTypeNameContext* SolidityParser::MappingKeyTypeContext::userDefinedTypeName() {
  return getRuleContext<SolidityParser::UserDefinedTypeNameContext>(0);
}


size_t SolidityParser::MappingKeyTypeContext::getRuleIndex() const {
  return SolidityParser::RuleMappingKeyType;
}


antlrcpp::Any SolidityParser::MappingKeyTypeContext::accept(tree::ParseTreeVisitor *visitor) {
  if (auto parserVisitor = dynamic_cast<SolidityVisitor*>(visitor))
    return parserVisitor->visitMappingKeyType(this);
  else
    return visitor->visitChildren(this);
}

SolidityParser::MappingKeyTypeContext* SolidityParser::mappingKeyType() {
  MappingKeyTypeContext *_localctx = _tracker.createInstance<MappingKeyTypeContext>(_ctx, getState());
  enterRule(_localctx, 134, SolidityParser::RuleMappingKeyType);

  auto onExit = finally([=] {
    exitRule();
  });
  try {
    setState(989);
    _errHandler->sync(this);
    switch (getInterpreter<atn::ParserATNSimulator>()->adaptivePredict(_input, 104, _ctx)) {
    case 1: {
      enterOuterAlt(_localctx, 1);
      setState(987);
      elementaryTypeName(false);
      break;
    }

    case 2: {
      enterOuterAlt(_localctx, 2);
      setState(988);
      userDefinedTypeName();
      break;
    }

    }
   
  }
  catch (RecognitionException &e) {
    _errHandler->reportError(this, e);
    _localctx->exception = std::current_exception();
    _errHandler->recover(this, _localctx->exception);
  }

  return _localctx;
}

//----------------- YulStatementContext ------------------------------------------------------------------

SolidityParser::YulStatementContext::YulStatementContext(ParserRuleContext *parent, size_t invokingState)
  : ParserRuleContext(parent, invokingState) {
}

SolidityParser::YulBlockContext* SolidityParser::YulStatementContext::yulBlock() {
  return getRuleContext<SolidityParser::YulBlockContext>(0);
}

SolidityParser::YulVariableDeclarationContext* SolidityParser::YulStatementContext::yulVariableDeclaration() {
  return getRuleContext<SolidityParser::YulVariableDeclarationContext>(0);
}

SolidityParser::YulAssignmentContext* SolidityParser::YulStatementContext::yulAssignment() {
  return getRuleContext<SolidityParser::YulAssignmentContext>(0);
}

SolidityParser::YulFunctionCallContext* SolidityParser::YulStatementContext::yulFunctionCall() {
  return getRuleContext<SolidityParser::YulFunctionCallContext>(0);
}

SolidityParser::YulIfStatementContext* SolidityParser::YulStatementContext::yulIfStatement() {
  return getRuleContext<SolidityParser::YulIfStatementContext>(0);
}

SolidityParser::YulForStatementContext* SolidityParser::YulStatementContext::yulForStatement() {
  return getRuleContext<SolidityParser::YulForStatementContext>(0);
}

SolidityParser::YulSwitchStatementContext* SolidityParser::YulStatementContext::yulSwitchStatement() {
  return getRuleContext<SolidityParser::YulSwitchStatementContext>(0);
}

tree::TerminalNode* SolidityParser::YulStatementContext::YulLeave() {
  return getToken(SolidityParser::YulLeave, 0);
}

tree::TerminalNode* SolidityParser::YulStatementContext::YulBreak() {
  return getToken(SolidityParser::YulBreak, 0);
}

tree::TerminalNode* SolidityParser::YulStatementContext::YulContinue() {
  return getToken(SolidityParser::YulContinue, 0);
}

SolidityParser::YulFunctionDefinitionContext* SolidityParser::YulStatementContext::yulFunctionDefinition() {
  return getRuleContext<SolidityParser::YulFunctionDefinitionContext>(0);
}


size_t SolidityParser::YulStatementContext::getRuleIndex() const {
  return SolidityParser::RuleYulStatement;
}


antlrcpp::Any SolidityParser::YulStatementContext::accept(tree::ParseTreeVisitor *visitor) {
  if (auto parserVisitor = dynamic_cast<SolidityVisitor*>(visitor))
    return parserVisitor->visitYulStatement(this);
  else
    return visitor->visitChildren(this);
}

SolidityParser::YulStatementContext* SolidityParser::yulStatement() {
  YulStatementContext *_localctx = _tracker.createInstance<YulStatementContext>(_ctx, getState());
  enterRule(_localctx, 136, SolidityParser::RuleYulStatement);

  auto onExit = finally([=] {
    exitRule();
  });
  try {
    setState(1002);
    _errHandler->sync(this);
    switch (getInterpreter<atn::ParserATNSimulator>()->adaptivePredict(_input, 105, _ctx)) {
    case 1: {
      enterOuterAlt(_localctx, 1);
      setState(991);
      yulBlock();
      break;
    }

    case 2: {
      enterOuterAlt(_localctx, 2);
      setState(992);
      yulVariableDeclaration();
      break;
    }

    case 3: {
      enterOuterAlt(_localctx, 3);
      setState(993);
      yulAssignment();
      break;
    }

    case 4: {
      enterOuterAlt(_localctx, 4);
      setState(994);
      yulFunctionCall();
      break;
    }

    case 5: {
      enterOuterAlt(_localctx, 5);
      setState(995);
      yulIfStatement();
      break;
    }

    case 6: {
      enterOuterAlt(_localctx, 6);
      setState(996);
      yulForStatement();
      break;
    }

    case 7: {
      enterOuterAlt(_localctx, 7);
      setState(997);
      yulSwitchStatement();
      break;
    }

    case 8: {
      enterOuterAlt(_localctx, 8);
      setState(998);
      match(SolidityParser::YulLeave);
      break;
    }

    case 9: {
      enterOuterAlt(_localctx, 9);
      setState(999);
      match(SolidityParser::YulBreak);
      break;
    }

    case 10: {
      enterOuterAlt(_localctx, 10);
      setState(1000);
      match(SolidityParser::YulContinue);
      break;
    }

    case 11: {
      enterOuterAlt(_localctx, 11);
      setState(1001);
      yulFunctionDefinition();
      break;
    }

    }
   
  }
  catch (RecognitionException &e) {
    _errHandler->reportError(this, e);
    _localctx->exception = std::current_exception();
    _errHandler->recover(this, _localctx->exception);
  }

  return _localctx;
}

//----------------- YulBlockContext ------------------------------------------------------------------

SolidityParser::YulBlockContext::YulBlockContext(ParserRuleContext *parent, size_t invokingState)
  : ParserRuleContext(parent, invokingState) {
}

tree::TerminalNode* SolidityParser::YulBlockContext::YulLBrace() {
  return getToken(SolidityParser::YulLBrace, 0);
}

tree::TerminalNode* SolidityParser::YulBlockContext::YulRBrace() {
  return getToken(SolidityParser::YulRBrace, 0);
}

std::vector<SolidityParser::YulStatementContext *> SolidityParser::YulBlockContext::yulStatement() {
  return getRuleContexts<SolidityParser::YulStatementContext>();
}

SolidityParser::YulStatementContext* SolidityParser::YulBlockContext::yulStatement(size_t i) {
  return getRuleContext<SolidityParser::YulStatementContext>(i);
}


size_t SolidityParser::YulBlockContext::getRuleIndex() const {
  return SolidityParser::RuleYulBlock;
}


antlrcpp::Any SolidityParser::YulBlockContext::accept(tree::ParseTreeVisitor *visitor) {
  if (auto parserVisitor = dynamic_cast<SolidityVisitor*>(visitor))
    return parserVisitor->visitYulBlock(this);
  else
    return visitor->visitChildren(this);
}

SolidityParser::YulBlockContext* SolidityParser::yulBlock() {
  YulBlockContext *_localctx = _tracker.createInstance<YulBlockContext>(_ctx, getState());
  enterRule(_localctx, 138, SolidityParser::RuleYulBlock);
  size_t _la = 0;

  auto onExit = finally([=] {
    exitRule();
  });
  try {
    enterOuterAlt(_localctx, 1);
    setState(1004);
    match(SolidityParser::YulLBrace);
    setState(1008);
    _errHandler->sync(this);
    _la = _input->LA(1);
    while (((((_la - 129) & ~ 0x3fULL) == 0) &&
      ((1ULL << (_la - 129)) & ((1ULL << (SolidityParser::YulBreak - 129))
      | (1ULL << (SolidityParser::YulContinue - 129))
      | (1ULL << (SolidityParser::YulFor - 129))
      | (1ULL << (SolidityParser::YulFunction - 129))
      | (1ULL << (SolidityParser::YulIf - 129))
      | (1ULL << (SolidityParser::YulLeave - 129))
      | (1ULL << (SolidityParser::YulLet - 129))
      | (1ULL << (SolidityParser::YulSwitch - 129))
      | (1ULL << (SolidityParser::YulEVMBuiltin - 129))
      | (1ULL << (SolidityParser::YulLBrace - 129))
      | (1ULL << (SolidityParser::YulIdentifier - 129)))) != 0)) {
      setState(1005);
      yulStatement();
      setState(1010);
      _errHandler->sync(this);
      _la = _input->LA(1);
    }
    setState(1011);
    match(SolidityParser::YulRBrace);
   
  }
  catch (RecognitionException &e) {
    _errHandler->reportError(this, e);
    _localctx->exception = std::current_exception();
    _errHandler->recover(this, _localctx->exception);
  }

  return _localctx;
}

//----------------- YulVariableDeclarationContext ------------------------------------------------------------------

SolidityParser::YulVariableDeclarationContext::YulVariableDeclarationContext(ParserRuleContext *parent, size_t invokingState)
  : ParserRuleContext(parent, invokingState) {
}

tree::TerminalNode* SolidityParser::YulVariableDeclarationContext::YulLet() {
  return getToken(SolidityParser::YulLet, 0);
}

std::vector<tree::TerminalNode *> SolidityParser::YulVariableDeclarationContext::YulIdentifier() {
  return getTokens(SolidityParser::YulIdentifier);
}

tree::TerminalNode* SolidityParser::YulVariableDeclarationContext::YulIdentifier(size_t i) {
  return getToken(SolidityParser::YulIdentifier, i);
}

tree::TerminalNode* SolidityParser::YulVariableDeclarationContext::YulAssign() {
  return getToken(SolidityParser::YulAssign, 0);
}

SolidityParser::YulExpressionContext* SolidityParser::YulVariableDeclarationContext::yulExpression() {
  return getRuleContext<SolidityParser::YulExpressionContext>(0);
}

std::vector<tree::TerminalNode *> SolidityParser::YulVariableDeclarationContext::YulComma() {
  return getTokens(SolidityParser::YulComma);
}

tree::TerminalNode* SolidityParser::YulVariableDeclarationContext::YulComma(size_t i) {
  return getToken(SolidityParser::YulComma, i);
}

SolidityParser::YulFunctionCallContext* SolidityParser::YulVariableDeclarationContext::yulFunctionCall() {
  return getRuleContext<SolidityParser::YulFunctionCallContext>(0);
}


size_t SolidityParser::YulVariableDeclarationContext::getRuleIndex() const {
  return SolidityParser::RuleYulVariableDeclaration;
}


antlrcpp::Any SolidityParser::YulVariableDeclarationContext::accept(tree::ParseTreeVisitor *visitor) {
  if (auto parserVisitor = dynamic_cast<SolidityVisitor*>(visitor))
    return parserVisitor->visitYulVariableDeclaration(this);
  else
    return visitor->visitChildren(this);
}

SolidityParser::YulVariableDeclarationContext* SolidityParser::yulVariableDeclaration() {
  YulVariableDeclarationContext *_localctx = _tracker.createInstance<YulVariableDeclarationContext>(_ctx, getState());
  enterRule(_localctx, 140, SolidityParser::RuleYulVariableDeclaration);
  size_t _la = 0;

  auto onExit = finally([=] {
    exitRule();
  });
  try {
    setState(1032);
    _errHandler->sync(this);
    switch (getInterpreter<atn::ParserATNSimulator>()->adaptivePredict(_input, 110, _ctx)) {
    case 1: {
      enterOuterAlt(_localctx, 1);
      setState(1013);
      match(SolidityParser::YulLet);
      setState(1014);
      dynamic_cast<YulVariableDeclarationContext *>(_localctx)->yulidentifierToken = match(SolidityParser::YulIdentifier);
      dynamic_cast<YulVariableDeclarationContext *>(_localctx)->variables.push_back(dynamic_cast<YulVariableDeclarationContext *>(_localctx)->yulidentifierToken);
      setState(1017);
      _errHandler->sync(this);

      _la = _input->LA(1);
      if (_la == SolidityParser::YulAssign) {
        setState(1015);
        match(SolidityParser::YulAssign);
        setState(1016);
        yulExpression();
      }
      break;
    }

    case 2: {
      enterOuterAlt(_localctx, 2);
      setState(1019);
      match(SolidityParser::YulLet);
      setState(1020);
      dynamic_cast<YulVariableDeclarationContext *>(_localctx)->yulidentifierToken = match(SolidityParser::YulIdentifier);
      dynamic_cast<YulVariableDeclarationContext *>(_localctx)->variables.push_back(dynamic_cast<YulVariableDeclarationContext *>(_localctx)->yulidentifierToken);
      setState(1025);
      _errHandler->sync(this);
      _la = _input->LA(1);
      while (_la == SolidityParser::YulComma) {
        setState(1021);
        match(SolidityParser::YulComma);
        setState(1022);
        dynamic_cast<YulVariableDeclarationContext *>(_localctx)->yulidentifierToken = match(SolidityParser::YulIdentifier);
        dynamic_cast<YulVariableDeclarationContext *>(_localctx)->variables.push_back(dynamic_cast<YulVariableDeclarationContext *>(_localctx)->yulidentifierToken);
        setState(1027);
        _errHandler->sync(this);
        _la = _input->LA(1);
      }
      setState(1030);
      _errHandler->sync(this);

      _la = _input->LA(1);
      if (_la == SolidityParser::YulAssign) {
        setState(1028);
        match(SolidityParser::YulAssign);
        setState(1029);
        yulFunctionCall();
      }
      break;
    }

    }
   
  }
  catch (RecognitionException &e) {
    _errHandler->reportError(this, e);
    _localctx->exception = std::current_exception();
    _errHandler->recover(this, _localctx->exception);
  }

  return _localctx;
}

//----------------- YulAssignmentContext ------------------------------------------------------------------

SolidityParser::YulAssignmentContext::YulAssignmentContext(ParserRuleContext *parent, size_t invokingState)
  : ParserRuleContext(parent, invokingState) {
}

std::vector<SolidityParser::YulPathContext *> SolidityParser::YulAssignmentContext::yulPath() {
  return getRuleContexts<SolidityParser::YulPathContext>();
}

SolidityParser::YulPathContext* SolidityParser::YulAssignmentContext::yulPath(size_t i) {
  return getRuleContext<SolidityParser::YulPathContext>(i);
}

tree::TerminalNode* SolidityParser::YulAssignmentContext::YulAssign() {
  return getToken(SolidityParser::YulAssign, 0);
}

SolidityParser::YulExpressionContext* SolidityParser::YulAssignmentContext::yulExpression() {
  return getRuleContext<SolidityParser::YulExpressionContext>(0);
}

SolidityParser::YulFunctionCallContext* SolidityParser::YulAssignmentContext::yulFunctionCall() {
  return getRuleContext<SolidityParser::YulFunctionCallContext>(0);
}

std::vector<tree::TerminalNode *> SolidityParser::YulAssignmentContext::YulComma() {
  return getTokens(SolidityParser::YulComma);
}

tree::TerminalNode* SolidityParser::YulAssignmentContext::YulComma(size_t i) {
  return getToken(SolidityParser::YulComma, i);
}


size_t SolidityParser::YulAssignmentContext::getRuleIndex() const {
  return SolidityParser::RuleYulAssignment;
}


antlrcpp::Any SolidityParser::YulAssignmentContext::accept(tree::ParseTreeVisitor *visitor) {
  if (auto parserVisitor = dynamic_cast<SolidityVisitor*>(visitor))
    return parserVisitor->visitYulAssignment(this);
  else
    return visitor->visitChildren(this);
}

SolidityParser::YulAssignmentContext* SolidityParser::yulAssignment() {
  YulAssignmentContext *_localctx = _tracker.createInstance<YulAssignmentContext>(_ctx, getState());
  enterRule(_localctx, 142, SolidityParser::RuleYulAssignment);
  size_t _la = 0;

  auto onExit = finally([=] {
    exitRule();
  });
  try {
    setState(1048);
    _errHandler->sync(this);
    switch (getInterpreter<atn::ParserATNSimulator>()->adaptivePredict(_input, 112, _ctx)) {
    case 1: {
      enterOuterAlt(_localctx, 1);
      setState(1034);
      yulPath();
      setState(1035);
      match(SolidityParser::YulAssign);
      setState(1036);
      yulExpression();
      break;
    }

    case 2: {
      enterOuterAlt(_localctx, 2);
      setState(1038);
      yulPath();
      setState(1041); 
      _errHandler->sync(this);
      _la = _input->LA(1);
      do {
        setState(1039);
        match(SolidityParser::YulComma);
        setState(1040);
        yulPath();
        setState(1043); 
        _errHandler->sync(this);
        _la = _input->LA(1);
      } while (_la == SolidityParser::YulComma);
      setState(1045);
      match(SolidityParser::YulAssign);
      setState(1046);
      yulFunctionCall();
      break;
    }

    }
   
  }
  catch (RecognitionException &e) {
    _errHandler->reportError(this, e);
    _localctx->exception = std::current_exception();
    _errHandler->recover(this, _localctx->exception);
  }

  return _localctx;
}

//----------------- YulIfStatementContext ------------------------------------------------------------------

SolidityParser::YulIfStatementContext::YulIfStatementContext(ParserRuleContext *parent, size_t invokingState)
  : ParserRuleContext(parent, invokingState) {
}

tree::TerminalNode* SolidityParser::YulIfStatementContext::YulIf() {
  return getToken(SolidityParser::YulIf, 0);
}

SolidityParser::YulExpressionContext* SolidityParser::YulIfStatementContext::yulExpression() {
  return getRuleContext<SolidityParser::YulExpressionContext>(0);
}

SolidityParser::YulBlockContext* SolidityParser::YulIfStatementContext::yulBlock() {
  return getRuleContext<SolidityParser::YulBlockContext>(0);
}


size_t SolidityParser::YulIfStatementContext::getRuleIndex() const {
  return SolidityParser::RuleYulIfStatement;
}


antlrcpp::Any SolidityParser::YulIfStatementContext::accept(tree::ParseTreeVisitor *visitor) {
  if (auto parserVisitor = dynamic_cast<SolidityVisitor*>(visitor))
    return parserVisitor->visitYulIfStatement(this);
  else
    return visitor->visitChildren(this);
}

SolidityParser::YulIfStatementContext* SolidityParser::yulIfStatement() {
  YulIfStatementContext *_localctx = _tracker.createInstance<YulIfStatementContext>(_ctx, getState());
  enterRule(_localctx, 144, SolidityParser::RuleYulIfStatement);

  auto onExit = finally([=] {
    exitRule();
  });
  try {
    enterOuterAlt(_localctx, 1);
    setState(1050);
    match(SolidityParser::YulIf);
    setState(1051);
    dynamic_cast<YulIfStatementContext *>(_localctx)->cond = yulExpression();
    setState(1052);
    dynamic_cast<YulIfStatementContext *>(_localctx)->body = yulBlock();
   
  }
  catch (RecognitionException &e) {
    _errHandler->reportError(this, e);
    _localctx->exception = std::current_exception();
    _errHandler->recover(this, _localctx->exception);
  }

  return _localctx;
}

//----------------- YulForStatementContext ------------------------------------------------------------------

SolidityParser::YulForStatementContext::YulForStatementContext(ParserRuleContext *parent, size_t invokingState)
  : ParserRuleContext(parent, invokingState) {
}

tree::TerminalNode* SolidityParser::YulForStatementContext::YulFor() {
  return getToken(SolidityParser::YulFor, 0);
}

std::vector<SolidityParser::YulBlockContext *> SolidityParser::YulForStatementContext::yulBlock() {
  return getRuleContexts<SolidityParser::YulBlockContext>();
}

SolidityParser::YulBlockContext* SolidityParser::YulForStatementContext::yulBlock(size_t i) {
  return getRuleContext<SolidityParser::YulBlockContext>(i);
}

SolidityParser::YulExpressionContext* SolidityParser::YulForStatementContext::yulExpression() {
  return getRuleContext<SolidityParser::YulExpressionContext>(0);
}


size_t SolidityParser::YulForStatementContext::getRuleIndex() const {
  return SolidityParser::RuleYulForStatement;
}


antlrcpp::Any SolidityParser::YulForStatementContext::accept(tree::ParseTreeVisitor *visitor) {
  if (auto parserVisitor = dynamic_cast<SolidityVisitor*>(visitor))
    return parserVisitor->visitYulForStatement(this);
  else
    return visitor->visitChildren(this);
}

SolidityParser::YulForStatementContext* SolidityParser::yulForStatement() {
  YulForStatementContext *_localctx = _tracker.createInstance<YulForStatementContext>(_ctx, getState());
  enterRule(_localctx, 146, SolidityParser::RuleYulForStatement);

  auto onExit = finally([=] {
    exitRule();
  });
  try {
    enterOuterAlt(_localctx, 1);
    setState(1054);
    match(SolidityParser::YulFor);
    setState(1055);
    dynamic_cast<YulForStatementContext *>(_localctx)->init = yulBlock();
    setState(1056);
    dynamic_cast<YulForStatementContext *>(_localctx)->cond = yulExpression();
    setState(1057);
    dynamic_cast<YulForStatementContext *>(_localctx)->post = yulBlock();
    setState(1058);
    dynamic_cast<YulForStatementContext *>(_localctx)->body = yulBlock();
   
  }
  catch (RecognitionException &e) {
    _errHandler->reportError(this, e);
    _localctx->exception = std::current_exception();
    _errHandler->recover(this, _localctx->exception);
  }

  return _localctx;
}

//----------------- YulSwitchCaseContext ------------------------------------------------------------------

SolidityParser::YulSwitchCaseContext::YulSwitchCaseContext(ParserRuleContext *parent, size_t invokingState)
  : ParserRuleContext(parent, invokingState) {
}

tree::TerminalNode* SolidityParser::YulSwitchCaseContext::YulCase() {
  return getToken(SolidityParser::YulCase, 0);
}

SolidityParser::YulLiteralContext* SolidityParser::YulSwitchCaseContext::yulLiteral() {
  return getRuleContext<SolidityParser::YulLiteralContext>(0);
}

SolidityParser::YulBlockContext* SolidityParser::YulSwitchCaseContext::yulBlock() {
  return getRuleContext<SolidityParser::YulBlockContext>(0);
}


size_t SolidityParser::YulSwitchCaseContext::getRuleIndex() const {
  return SolidityParser::RuleYulSwitchCase;
}


antlrcpp::Any SolidityParser::YulSwitchCaseContext::accept(tree::ParseTreeVisitor *visitor) {
  if (auto parserVisitor = dynamic_cast<SolidityVisitor*>(visitor))
    return parserVisitor->visitYulSwitchCase(this);
  else
    return visitor->visitChildren(this);
}

SolidityParser::YulSwitchCaseContext* SolidityParser::yulSwitchCase() {
  YulSwitchCaseContext *_localctx = _tracker.createInstance<YulSwitchCaseContext>(_ctx, getState());
  enterRule(_localctx, 148, SolidityParser::RuleYulSwitchCase);

  auto onExit = finally([=] {
    exitRule();
  });
  try {
    enterOuterAlt(_localctx, 1);
    setState(1060);
    match(SolidityParser::YulCase);
    setState(1061);
    yulLiteral();
    setState(1062);
    yulBlock();
   
  }
  catch (RecognitionException &e) {
    _errHandler->reportError(this, e);
    _localctx->exception = std::current_exception();
    _errHandler->recover(this, _localctx->exception);
  }

  return _localctx;
}

//----------------- YulSwitchStatementContext ------------------------------------------------------------------

SolidityParser::YulSwitchStatementContext::YulSwitchStatementContext(ParserRuleContext *parent, size_t invokingState)
  : ParserRuleContext(parent, invokingState) {
}

tree::TerminalNode* SolidityParser::YulSwitchStatementContext::YulSwitch() {
  return getToken(SolidityParser::YulSwitch, 0);
}

SolidityParser::YulExpressionContext* SolidityParser::YulSwitchStatementContext::yulExpression() {
  return getRuleContext<SolidityParser::YulExpressionContext>(0);
}

tree::TerminalNode* SolidityParser::YulSwitchStatementContext::YulDefault() {
  return getToken(SolidityParser::YulDefault, 0);
}

SolidityParser::YulBlockContext* SolidityParser::YulSwitchStatementContext::yulBlock() {
  return getRuleContext<SolidityParser::YulBlockContext>(0);
}

std::vector<SolidityParser::YulSwitchCaseContext *> SolidityParser::YulSwitchStatementContext::yulSwitchCase() {
  return getRuleContexts<SolidityParser::YulSwitchCaseContext>();
}

SolidityParser::YulSwitchCaseContext* SolidityParser::YulSwitchStatementContext::yulSwitchCase(size_t i) {
  return getRuleContext<SolidityParser::YulSwitchCaseContext>(i);
}


size_t SolidityParser::YulSwitchStatementContext::getRuleIndex() const {
  return SolidityParser::RuleYulSwitchStatement;
}


antlrcpp::Any SolidityParser::YulSwitchStatementContext::accept(tree::ParseTreeVisitor *visitor) {
  if (auto parserVisitor = dynamic_cast<SolidityVisitor*>(visitor))
    return parserVisitor->visitYulSwitchStatement(this);
  else
    return visitor->visitChildren(this);
}

SolidityParser::YulSwitchStatementContext* SolidityParser::yulSwitchStatement() {
  YulSwitchStatementContext *_localctx = _tracker.createInstance<YulSwitchStatementContext>(_ctx, getState());
  enterRule(_localctx, 150, SolidityParser::RuleYulSwitchStatement);
  size_t _la = 0;

  auto onExit = finally([=] {
    exitRule();
  });
  try {
    enterOuterAlt(_localctx, 1);
    setState(1064);
    match(SolidityParser::YulSwitch);
    setState(1065);
    yulExpression();
    setState(1077);
    _errHandler->sync(this);
    switch (_input->LA(1)) {
      case SolidityParser::YulCase: {
        setState(1067); 
        _errHandler->sync(this);
        _la = _input->LA(1);
        do {
          setState(1066);
          yulSwitchCase();
          setState(1069); 
          _errHandler->sync(this);
          _la = _input->LA(1);
        } while (_la == SolidityParser::YulCase);
        setState(1073);
        _errHandler->sync(this);

        _la = _input->LA(1);
        if (_la == SolidityParser::YulDefault) {
          setState(1071);
          match(SolidityParser::YulDefault);
          setState(1072);
          yulBlock();
        }
        break;
      }

      case SolidityParser::YulDefault: {
        setState(1075);
        match(SolidityParser::YulDefault);
        setState(1076);
        yulBlock();
        break;
      }

    default:
      throw NoViableAltException(this);
    }
   
  }
  catch (RecognitionException &e) {
    _errHandler->reportError(this, e);
    _localctx->exception = std::current_exception();
    _errHandler->recover(this, _localctx->exception);
  }

  return _localctx;
}

//----------------- YulFunctionDefinitionContext ------------------------------------------------------------------

SolidityParser::YulFunctionDefinitionContext::YulFunctionDefinitionContext(ParserRuleContext *parent, size_t invokingState)
  : ParserRuleContext(parent, invokingState) {
}

tree::TerminalNode* SolidityParser::YulFunctionDefinitionContext::YulFunction() {
  return getToken(SolidityParser::YulFunction, 0);
}

std::vector<tree::TerminalNode *> SolidityParser::YulFunctionDefinitionContext::YulIdentifier() {
  return getTokens(SolidityParser::YulIdentifier);
}

tree::TerminalNode* SolidityParser::YulFunctionDefinitionContext::YulIdentifier(size_t i) {
  return getToken(SolidityParser::YulIdentifier, i);
}

tree::TerminalNode* SolidityParser::YulFunctionDefinitionContext::YulLParen() {
  return getToken(SolidityParser::YulLParen, 0);
}

tree::TerminalNode* SolidityParser::YulFunctionDefinitionContext::YulRParen() {
  return getToken(SolidityParser::YulRParen, 0);
}

SolidityParser::YulBlockContext* SolidityParser::YulFunctionDefinitionContext::yulBlock() {
  return getRuleContext<SolidityParser::YulBlockContext>(0);
}

tree::TerminalNode* SolidityParser::YulFunctionDefinitionContext::YulArrow() {
  return getToken(SolidityParser::YulArrow, 0);
}

std::vector<tree::TerminalNode *> SolidityParser::YulFunctionDefinitionContext::YulComma() {
  return getTokens(SolidityParser::YulComma);
}

tree::TerminalNode* SolidityParser::YulFunctionDefinitionContext::YulComma(size_t i) {
  return getToken(SolidityParser::YulComma, i);
}


size_t SolidityParser::YulFunctionDefinitionContext::getRuleIndex() const {
  return SolidityParser::RuleYulFunctionDefinition;
}


antlrcpp::Any SolidityParser::YulFunctionDefinitionContext::accept(tree::ParseTreeVisitor *visitor) {
  if (auto parserVisitor = dynamic_cast<SolidityVisitor*>(visitor))
    return parserVisitor->visitYulFunctionDefinition(this);
  else
    return visitor->visitChildren(this);
}

SolidityParser::YulFunctionDefinitionContext* SolidityParser::yulFunctionDefinition() {
  YulFunctionDefinitionContext *_localctx = _tracker.createInstance<YulFunctionDefinitionContext>(_ctx, getState());
  enterRule(_localctx, 152, SolidityParser::RuleYulFunctionDefinition);
  size_t _la = 0;

  auto onExit = finally([=] {
    exitRule();
  });
  try {
    enterOuterAlt(_localctx, 1);
    setState(1079);
    match(SolidityParser::YulFunction);
    setState(1080);
    match(SolidityParser::YulIdentifier);
    setState(1081);
    match(SolidityParser::YulLParen);
    setState(1090);
    _errHandler->sync(this);

    _la = _input->LA(1);
    if (_la == SolidityParser::YulIdentifier) {
      setState(1082);
      dynamic_cast<YulFunctionDefinitionContext *>(_localctx)->yulidentifierToken = match(SolidityParser::YulIdentifier);
      dynamic_cast<YulFunctionDefinitionContext *>(_localctx)->arguments.push_back(dynamic_cast<YulFunctionDefinitionContext *>(_localctx)->yulidentifierToken);
      setState(1087);
      _errHandler->sync(this);
      _la = _input->LA(1);
      while (_la == SolidityParser::YulComma) {
        setState(1083);
        match(SolidityParser::YulComma);
        setState(1084);
        dynamic_cast<YulFunctionDefinitionContext *>(_localctx)->yulidentifierToken = match(SolidityParser::YulIdentifier);
        dynamic_cast<YulFunctionDefinitionContext *>(_localctx)->arguments.push_back(dynamic_cast<YulFunctionDefinitionContext *>(_localctx)->yulidentifierToken);
        setState(1089);
        _errHandler->sync(this);
        _la = _input->LA(1);
      }
    }
    setState(1092);
    match(SolidityParser::YulRParen);
    setState(1102);
    _errHandler->sync(this);

    _la = _input->LA(1);
    if (_la == SolidityParser::YulArrow) {
      setState(1093);
      match(SolidityParser::YulArrow);
      setState(1094);
      dynamic_cast<YulFunctionDefinitionContext *>(_localctx)->yulidentifierToken = match(SolidityParser::YulIdentifier);
      dynamic_cast<YulFunctionDefinitionContext *>(_localctx)->returnParameters.push_back(dynamic_cast<YulFunctionDefinitionContext *>(_localctx)->yulidentifierToken);
      setState(1099);
      _errHandler->sync(this);
      _la = _input->LA(1);
      while (_la == SolidityParser::YulComma) {
        setState(1095);
        match(SolidityParser::YulComma);
        setState(1096);
        dynamic_cast<YulFunctionDefinitionContext *>(_localctx)->yulidentifierToken = match(SolidityParser::YulIdentifier);
        dynamic_cast<YulFunctionDefinitionContext *>(_localctx)->returnParameters.push_back(dynamic_cast<YulFunctionDefinitionContext *>(_localctx)->yulidentifierToken);
        setState(1101);
        _errHandler->sync(this);
        _la = _input->LA(1);
      }
    }
    setState(1104);
    dynamic_cast<YulFunctionDefinitionContext *>(_localctx)->body = yulBlock();
   
  }
  catch (RecognitionException &e) {
    _errHandler->reportError(this, e);
    _localctx->exception = std::current_exception();
    _errHandler->recover(this, _localctx->exception);
  }

  return _localctx;
}

//----------------- YulPathContext ------------------------------------------------------------------

SolidityParser::YulPathContext::YulPathContext(ParserRuleContext *parent, size_t invokingState)
  : ParserRuleContext(parent, invokingState) {
}

std::vector<tree::TerminalNode *> SolidityParser::YulPathContext::YulIdentifier() {
  return getTokens(SolidityParser::YulIdentifier);
}

tree::TerminalNode* SolidityParser::YulPathContext::YulIdentifier(size_t i) {
  return getToken(SolidityParser::YulIdentifier, i);
}

std::vector<tree::TerminalNode *> SolidityParser::YulPathContext::YulPeriod() {
  return getTokens(SolidityParser::YulPeriod);
}

tree::TerminalNode* SolidityParser::YulPathContext::YulPeriod(size_t i) {
  return getToken(SolidityParser::YulPeriod, i);
}


size_t SolidityParser::YulPathContext::getRuleIndex() const {
  return SolidityParser::RuleYulPath;
}


antlrcpp::Any SolidityParser::YulPathContext::accept(tree::ParseTreeVisitor *visitor) {
  if (auto parserVisitor = dynamic_cast<SolidityVisitor*>(visitor))
    return parserVisitor->visitYulPath(this);
  else
    return visitor->visitChildren(this);
}

SolidityParser::YulPathContext* SolidityParser::yulPath() {
  YulPathContext *_localctx = _tracker.createInstance<YulPathContext>(_ctx, getState());
  enterRule(_localctx, 154, SolidityParser::RuleYulPath);
  size_t _la = 0;

  auto onExit = finally([=] {
    exitRule();
  });
  try {
    enterOuterAlt(_localctx, 1);
    setState(1106);
    match(SolidityParser::YulIdentifier);
    setState(1111);
    _errHandler->sync(this);
    _la = _input->LA(1);
    while (_la == SolidityParser::YulPeriod) {
      setState(1107);
      match(SolidityParser::YulPeriod);
      setState(1108);
      match(SolidityParser::YulIdentifier);
      setState(1113);
      _errHandler->sync(this);
      _la = _input->LA(1);
    }
   
  }
  catch (RecognitionException &e) {
    _errHandler->reportError(this, e);
    _localctx->exception = std::current_exception();
    _errHandler->recover(this, _localctx->exception);
  }

  return _localctx;
}

//----------------- YulFunctionCallContext ------------------------------------------------------------------

SolidityParser::YulFunctionCallContext::YulFunctionCallContext(ParserRuleContext *parent, size_t invokingState)
  : ParserRuleContext(parent, invokingState) {
}

tree::TerminalNode* SolidityParser::YulFunctionCallContext::YulLParen() {
  return getToken(SolidityParser::YulLParen, 0);
}

tree::TerminalNode* SolidityParser::YulFunctionCallContext::YulRParen() {
  return getToken(SolidityParser::YulRParen, 0);
}

tree::TerminalNode* SolidityParser::YulFunctionCallContext::YulIdentifier() {
  return getToken(SolidityParser::YulIdentifier, 0);
}

tree::TerminalNode* SolidityParser::YulFunctionCallContext::YulEVMBuiltin() {
  return getToken(SolidityParser::YulEVMBuiltin, 0);
}

std::vector<SolidityParser::YulExpressionContext *> SolidityParser::YulFunctionCallContext::yulExpression() {
  return getRuleContexts<SolidityParser::YulExpressionContext>();
}

SolidityParser::YulExpressionContext* SolidityParser::YulFunctionCallContext::yulExpression(size_t i) {
  return getRuleContext<SolidityParser::YulExpressionContext>(i);
}

std::vector<tree::TerminalNode *> SolidityParser::YulFunctionCallContext::YulComma() {
  return getTokens(SolidityParser::YulComma);
}

tree::TerminalNode* SolidityParser::YulFunctionCallContext::YulComma(size_t i) {
  return getToken(SolidityParser::YulComma, i);
}


size_t SolidityParser::YulFunctionCallContext::getRuleIndex() const {
  return SolidityParser::RuleYulFunctionCall;
}


antlrcpp::Any SolidityParser::YulFunctionCallContext::accept(tree::ParseTreeVisitor *visitor) {
  if (auto parserVisitor = dynamic_cast<SolidityVisitor*>(visitor))
    return parserVisitor->visitYulFunctionCall(this);
  else
    return visitor->visitChildren(this);
}

SolidityParser::YulFunctionCallContext* SolidityParser::yulFunctionCall() {
  YulFunctionCallContext *_localctx = _tracker.createInstance<YulFunctionCallContext>(_ctx, getState());
  enterRule(_localctx, 156, SolidityParser::RuleYulFunctionCall);
  size_t _la = 0;

  auto onExit = finally([=] {
    exitRule();
  });
  try {
    enterOuterAlt(_localctx, 1);
    setState(1114);
    _la = _input->LA(1);
    if (!(_la == SolidityParser::YulEVMBuiltin

    || _la == SolidityParser::YulIdentifier)) {
    _errHandler->recoverInline(this);
    }
    else {
      _errHandler->reportMatch(this);
      consume();
    }
    setState(1115);
    match(SolidityParser::YulLParen);
    setState(1124);
    _errHandler->sync(this);

    _la = _input->LA(1);
    if (((((_la - 133) & ~ 0x3fULL) == 0) &&
      ((1ULL << (_la - 133)) & ((1ULL << (SolidityParser::YulFalse - 133))
      | (1ULL << (SolidityParser::YulTrue - 133))
      | (1ULL << (SolidityParser::YulEVMBuiltin - 133))
      | (1ULL << (SolidityParser::YulIdentifier - 133))
      | (1ULL << (SolidityParser::YulHexNumber - 133))
      | (1ULL << (SolidityParser::YulDecimalNumber - 133))
      | (1ULL << (SolidityParser::YulStringLiteral - 133)))) != 0)) {
      setState(1116);
      yulExpression();
      setState(1121);
      _errHandler->sync(this);
      _la = _input->LA(1);
      while (_la == SolidityParser::YulComma) {
        setState(1117);
        match(SolidityParser::YulComma);
        setState(1118);
        yulExpression();
        setState(1123);
        _errHandler->sync(this);
        _la = _input->LA(1);
      }
    }
    setState(1126);
    match(SolidityParser::YulRParen);
   
  }
  catch (RecognitionException &e) {
    _errHandler->reportError(this, e);
    _localctx->exception = std::current_exception();
    _errHandler->recover(this, _localctx->exception);
  }

  return _localctx;
}

//----------------- YulboolContext ------------------------------------------------------------------

SolidityParser::YulboolContext::YulboolContext(ParserRuleContext *parent, size_t invokingState)
  : ParserRuleContext(parent, invokingState) {
}

tree::TerminalNode* SolidityParser::YulboolContext::YulTrue() {
  return getToken(SolidityParser::YulTrue, 0);
}

tree::TerminalNode* SolidityParser::YulboolContext::YulFalse() {
  return getToken(SolidityParser::YulFalse, 0);
}


size_t SolidityParser::YulboolContext::getRuleIndex() const {
  return SolidityParser::RuleYulbool;
}


antlrcpp::Any SolidityParser::YulboolContext::accept(tree::ParseTreeVisitor *visitor) {
  if (auto parserVisitor = dynamic_cast<SolidityVisitor*>(visitor))
    return parserVisitor->visitYulBoolean(this);
  else
    return visitor->visitChildren(this);
}

SolidityParser::YulboolContext* SolidityParser::yulbool() {
  YulboolContext *_localctx = _tracker.createInstance<YulboolContext>(_ctx, getState());
  enterRule(_localctx, 158, SolidityParser::RuleYulbool);
  size_t _la = 0;

  auto onExit = finally([=] {
    exitRule();
  });
  try {
    enterOuterAlt(_localctx, 1);
    setState(1128);
    _la = _input->LA(1);
    if (!(_la == SolidityParser::YulFalse

    || _la == SolidityParser::YulTrue)) {
    _errHandler->recoverInline(this);
    }
    else {
      _errHandler->reportMatch(this);
      consume();
    }
   
  }
  catch (RecognitionException &e) {
    _errHandler->reportError(this, e);
    _localctx->exception = std::current_exception();
    _errHandler->recover(this, _localctx->exception);
  }

  return _localctx;
}

//----------------- YulLiteralContext ------------------------------------------------------------------

SolidityParser::YulLiteralContext::YulLiteralContext(ParserRuleContext *parent, size_t invokingState)
  : ParserRuleContext(parent, invokingState) {
}

tree::TerminalNode* SolidityParser::YulLiteralContext::YulDecimalNumber() {
  return getToken(SolidityParser::YulDecimalNumber, 0);
}

tree::TerminalNode* SolidityParser::YulLiteralContext::YulStringLiteral() {
  return getToken(SolidityParser::YulStringLiteral, 0);
}

tree::TerminalNode* SolidityParser::YulLiteralContext::YulHexNumber() {
  return getToken(SolidityParser::YulHexNumber, 0);
}

SolidityParser::YulboolContext* SolidityParser::YulLiteralContext::yulbool() {
  return getRuleContext<SolidityParser::YulboolContext>(0);
}


size_t SolidityParser::YulLiteralContext::getRuleIndex() const {
  return SolidityParser::RuleYulLiteral;
}


antlrcpp::Any SolidityParser::YulLiteralContext::accept(tree::ParseTreeVisitor *visitor) {
  if (auto parserVisitor = dynamic_cast<SolidityVisitor*>(visitor))
    return parserVisitor->visitYulLiteral(this);
  else
    return visitor->visitChildren(this);
}

SolidityParser::YulLiteralContext* SolidityParser::yulLiteral() {
  YulLiteralContext *_localctx = _tracker.createInstance<YulLiteralContext>(_ctx, getState());
  enterRule(_localctx, 160, SolidityParser::RuleYulLiteral);

  auto onExit = finally([=] {
    exitRule();
  });
  try {
    setState(1134);
    _errHandler->sync(this);
    switch (_input->LA(1)) {
      case SolidityParser::YulDecimalNumber: {
        enterOuterAlt(_localctx, 1);
        setState(1130);
        match(SolidityParser::YulDecimalNumber);
        break;
      }

      case SolidityParser::YulStringLiteral: {
        enterOuterAlt(_localctx, 2);
        setState(1131);
        match(SolidityParser::YulStringLiteral);
        break;
      }

      case SolidityParser::YulHexNumber: {
        enterOuterAlt(_localctx, 3);
        setState(1132);
        match(SolidityParser::YulHexNumber);
        break;
      }

      case SolidityParser::YulFalse:
      case SolidityParser::YulTrue: {
        enterOuterAlt(_localctx, 4);
        setState(1133);
        yulbool();
        break;
      }

    default:
      throw NoViableAltException(this);
    }
   
  }
  catch (RecognitionException &e) {
    _errHandler->reportError(this, e);
    _localctx->exception = std::current_exception();
    _errHandler->recover(this, _localctx->exception);
  }

  return _localctx;
}

//----------------- YulExpressionContext ------------------------------------------------------------------

SolidityParser::YulExpressionContext::YulExpressionContext(ParserRuleContext *parent, size_t invokingState)
  : ParserRuleContext(parent, invokingState) {
}

SolidityParser::YulPathContext* SolidityParser::YulExpressionContext::yulPath() {
  return getRuleContext<SolidityParser::YulPathContext>(0);
}

SolidityParser::YulFunctionCallContext* SolidityParser::YulExpressionContext::yulFunctionCall() {
  return getRuleContext<SolidityParser::YulFunctionCallContext>(0);
}

SolidityParser::YulLiteralContext* SolidityParser::YulExpressionContext::yulLiteral() {
  return getRuleContext<SolidityParser::YulLiteralContext>(0);
}


size_t SolidityParser::YulExpressionContext::getRuleIndex() const {
  return SolidityParser::RuleYulExpression;
}


antlrcpp::Any SolidityParser::YulExpressionContext::accept(tree::ParseTreeVisitor *visitor) {
  if (auto parserVisitor = dynamic_cast<SolidityVisitor*>(visitor))
    return parserVisitor->visitYulExpression(this);
  else
    return visitor->visitChildren(this);
}

SolidityParser::YulExpressionContext* SolidityParser::yulExpression() {
  YulExpressionContext *_localctx = _tracker.createInstance<YulExpressionContext>(_ctx, getState());
  enterRule(_localctx, 162, SolidityParser::RuleYulExpression);

  auto onExit = finally([=] {
    exitRule();
  });
  try {
    setState(1139);
    _errHandler->sync(this);
    switch (getInterpreter<atn::ParserATNSimulator>()->adaptivePredict(_input, 124, _ctx)) {
    case 1: {
      enterOuterAlt(_localctx, 1);
      setState(1136);
      yulPath();
      break;
    }

    case 2: {
      enterOuterAlt(_localctx, 2);
      setState(1137);
      yulFunctionCall();
      break;
    }

    case 3: {
      enterOuterAlt(_localctx, 3);
      setState(1138);
      yulLiteral();
      break;
    }

    }
   
  }
  catch (RecognitionException &e) {
    _errHandler->reportError(this, e);
    _localctx->exception = std::current_exception();
    _errHandler->recover(this, _localctx->exception);
  }

  return _localctx;
}

bool SolidityParser::sempred(RuleContext *context, size_t ruleIndex, size_t predicateIndex) {
  switch (ruleIndex) {
    case 19: return constructorDefinitionSempred(dynamic_cast<ConstructorDefinitionContext *>(context), predicateIndex);
    case 22: return functionDefinitionSempred(dynamic_cast<FunctionDefinitionContext *>(context), predicateIndex);
    case 23: return modifierDefinitionSempred(dynamic_cast<ModifierDefinitionContext *>(context), predicateIndex);
    case 24: return fallbackReceiveFunctionDefinitionSempred(dynamic_cast<FallbackReceiveFunctionDefinitionContext *>(context), predicateIndex);
    case 28: return stateVariableDeclarationSempred(dynamic_cast<StateVariableDeclarationContext *>(context), predicateIndex);
    case 32: return typeNameSempred(dynamic_cast<TypeNameContext *>(context), predicateIndex);
    case 33: return elementaryTypeNameSempred(dynamic_cast<ElementaryTypeNameContext *>(context), predicateIndex);
    case 34: return functionTypeNameSempred(dynamic_cast<FunctionTypeNameContext *>(context), predicateIndex);
    case 37: return expressionSempred(dynamic_cast<ExpressionContext *>(context), predicateIndex);

  default:
    break;
  }
  return true;
}

bool SolidityParser::constructorDefinitionSempred(ConstructorDefinitionContext *_localctx, size_t predicateIndex) {
  switch (predicateIndex) {
    case 0: return !_localctx->payableSet;
    case 1: return !_localctx->visibilitySet;
    case 2: return !_localctx->visibilitySet;

  default:
    break;
  }
  return true;
}

bool SolidityParser::functionDefinitionSempred(FunctionDefinitionContext *_localctx, size_t predicateIndex) {
  switch (predicateIndex) {
    case 3: return !_localctx->visibilitySet;
    case 4: return !_localctx->mutabilitySet;
    case 5: return !_localctx->virtualSet;
    case 6: return !_localctx->overrideSpecifierSet;

  default:
    break;
  }
  return true;
}

bool SolidityParser::modifierDefinitionSempred(ModifierDefinitionContext *_localctx, size_t predicateIndex) {
  switch (predicateIndex) {
    case 7: return !_localctx->virtualSet;
    case 8: return !_localctx->overrideSpecifierSet;

  default:
    break;
  }
  return true;
}

bool SolidityParser::fallbackReceiveFunctionDefinitionSempred(FallbackReceiveFunctionDefinitionContext *_localctx, size_t predicateIndex) {
  switch (predicateIndex) {
    case 9: return !_localctx->visibilitySet;
    case 10: return !_localctx->mutabilitySet;
    case 11: return !_localctx->virtualSet;
    case 12: return !_localctx->overrideSpecifierSet;

  default:
    break;
  }
  return true;
}

bool SolidityParser::stateVariableDeclarationSempred(StateVariableDeclarationContext *_localctx, size_t predicateIndex) {
  switch (predicateIndex) {
    case 13: return !_localctx->visibilitySet;
    case 14: return !_localctx->visibilitySet;
    case 15: return !_localctx->visibilitySet;
    case 16: return !_localctx->constantnessSet;
    case 17: return !_localctx->overrideSpecifierSet;
    case 18: return !_localctx->constantnessSet;

  default:
    break;
  }
  return true;
}

bool SolidityParser::typeNameSempred(TypeNameContext *, size_t predicateIndex) {
  switch (predicateIndex) {
    case 19: return precpred(_ctx, 1);

  default:
    break;
  }
  return true;
}

bool SolidityParser::elementaryTypeNameSempred(ElementaryTypeNameContext *_localctx, size_t predicateIndex) {
  switch (predicateIndex) {
    case 20: return _localctx->allowAddressPayable;

  default:
    break;
  }
  return true;
}

bool SolidityParser::functionTypeNameSempred(FunctionTypeNameContext *_localctx, size_t predicateIndex) {
  switch (predicateIndex) {
    case 21: return !_localctx->visibilitySet;
    case 22: return !_localctx->mutabilitySet;

  default:
    break;
  }
  return true;
}

bool SolidityParser::expressionSempred(ExpressionContext *, size_t predicateIndex) {
  switch (predicateIndex) {
    case 23: return precpred(_ctx, 17);
    case 24: return precpred(_ctx, 16);
    case 25: return precpred(_ctx, 15);
    case 26: return precpred(_ctx, 14);
    case 27: return precpred(_ctx, 13);
    case 28: return precpred(_ctx, 12);
    case 29: return precpred(_ctx, 11);
    case 30: return precpred(_ctx, 10);
    case 31: return precpred(_ctx, 9);
    case 32: return precpred(_ctx, 8);
    case 33: return precpred(_ctx, 7);
    case 34: return precpred(_ctx, 6);
    case 35: return precpred(_ctx, 5);
    case 36: return precpred(_ctx, 26);
    case 37: return precpred(_ctx, 25);
    case 38: return precpred(_ctx, 24);
    case 39: return precpred(_ctx, 23);
    case 40: return precpred(_ctx, 22);
    case 41: return precpred(_ctx, 18);

  default:
    break;
  }
  return true;
}

// Static vars and initialization.
std::vector<dfa::DFA> SolidityParser::_decisionToDFA;
atn::PredictionContextCache SolidityParser::_sharedContextCache;

// We own the ATN which in turn owns the ATN states.
atn::ATN SolidityParser::_atn;
std::vector<uint16_t> SolidityParser::_serializedATN;

std::vector<std::string> SolidityParser::_ruleNames = {
  "sourceUnit", "pragmaDirective", "importDirective", "importAliases", "path", 
  "symbolAliases", "contractDefinition", "interfaceDefinition", "libraryDefinition", 
  "inheritanceSpecifierList", "inheritanceSpecifier", "contractBodyElement", 
  "namedArgument", "callArgumentList", "userDefinedTypeName", "modifierInvocation", 
  "visibility", "parameterList", "parameterDeclaration", "constructorDefinition", 
  "stateMutability", "overrideSpecifier", "functionDefinition", "modifierDefinition", 
  "fallbackReceiveFunctionDefinition", "structDefinition", "structMember", 
  "enumDefinition", "stateVariableDeclaration", "eventParameter", "eventDefinition", 
  "usingDirective", "typeName", "elementaryTypeName", "functionTypeName", 
  "variableDeclaration", "dataLocation", "expression", "assignOp", "tupleExpression", 
  "inlineArrayExpression", "identifier", "literal", "boolLiteral", "stringLiteral", 
  "hexStringLiteral", "unicodeStringLiteral", "numberLiteral", "block", 
  "statement", "simpleStatement", "ifStatement", "forStatement", "whileStatement", 
  "doWhileStatement", "continueStatement", "breakStatement", "tryStatement", 
  "catchClause", "returnStatement", "emitStatement", "assemblyStatement", 
  "variableDeclarationList", "variableDeclarationTuple", "variableDeclarationStatement", 
  "expressionStatement", "mappingType", "mappingKeyType", "yulStatement", 
  "yulBlock", "yulVariableDeclaration", "yulAssignment", "yulIfStatement", 
  "yulForStatement", "yulSwitchCase", "yulSwitchStatement", "yulFunctionDefinition", 
  "yulPath", "yulFunctionCall", "yulbool", "yulLiteral", "yulExpression"
};

std::vector<std::string> SolidityParser::_literalNames = {
  "", "", "'pragma'", "'abstract'", "'anonymous'", "'address'", "'as'", 
  "'assembly'", "'bool'", "", "'bytes'", "'calldata'", "'catch'", "'constant'", 
  "'constructor'", "", "'contract'", "'delete'", "'do'", "'else'", "'emit'", 
  "'enum'", "'event'", "'external'", "'fallback'", "", "", "'from'", "", 
  "", "", "'hex'", "", "'immutable'", "'import'", "'indexed'", "'interface'", 
  "'internal'", "'is'", "'library'", "'mapping'", "'memory'", "'modifier'", 
  "'new'", "", "'override'", "'payable'", "'private'", "'public'", "'pure'", 
  "'receive'", "'return'", "'returns'", "", "'storage'", "'string'", "'struct'", 
  "", "'try'", "'type'", "", "", "'using'", "'view'", "'virtual'", "'while'", 
  "", "", "'['", "']'", "", "", "':'", "", "", "'?'", "'=>'", "'='", "'|='", 
  "'^='", "'&='", "'<<='", "'>>='", "'>>>='", "'+='", "'-='", "'*='", "'/='", 
  "'%='", "", "'||'", "'&&'", "'|'", "'^'", "'&'", "'<<'", "'>>'", "'>>>'", 
  "'+'", "'-'", "'*'", "'/'", "'%'", "'**'", "'=='", "'!='", "'<'", "'>'", 
  "'<='", "'>='", "'!'", "'~'", "'++'", "'--'", "", "", "", "", "", "", 
  "", "", "", "", "'\"evmasm\"'", "", "", "", "", "", "'case'", "", "'default'", 
  "", "", "", "", "'leave'", "'let'", "'switch'", "", "", "", "", "", "", 
  "':='"
};

std::vector<std::string> SolidityParser::_symbolicNames = {
  "", "ReservedKeywords", "Pragma", "Abstract", "Anonymous", "Address", 
  "As", "Assembly", "Bool", "Break", "Bytes", "Calldata", "Catch", "Constant", 
  "Constructor", "Continue", "Contract", "Delete", "Do", "Else", "Emit", 
  "Enum", "Event", "External", "Fallback", "False", "Fixed", "From", "FixedBytes", 
  "For", "Function", "Hex", "If", "Immutable", "Import", "Indexed", "Interface", 
  "Internal", "Is", "Library", "Mapping", "Memory", "Modifier", "New", "NumberUnit", 
  "Override", "Payable", "Private", "Public", "Pure", "Receive", "Return", 
  "Returns", "SignedIntegerType", "Storage", "String", "Struct", "True", 
  "Try", "Type", "Ufixed", "UnsignedIntegerType", "Using", "View", "Virtual", 
  "While", "LParen", "RParen", "LBrack", "RBrack", "LBrace", "RBrace", "Colon", 
  "Semicolon", "Period", "Conditional", "Arrow", "Assign", "AssignBitOr", 
  "AssignBitXor", "AssignBitAnd", "AssignShl", "AssignSar", "AssignShr", 
  "AssignAdd", "AssignSub", "AssignMul", "AssignDiv", "AssignMod", "Comma", 
  "Or", "And", "BitOr", "BitXor", "BitAnd", "Shl", "Sar", "Shr", "Add", 
  "Sub", "Mul", "Div", "Mod", "Exp", "Equal", "NotEqual", "LessThan", "GreaterThan", 
  "LessThanOrEqual", "GreaterThanOrEqual", "Not", "BitNot", "Inc", "Dec", 
  "StringLiteral", "NonEmptyStringLiteral", "UnicodeStringLiteral", "HexString", 
  "HexNumber", "DecimalNumber", "Identifier", "WS", "COMMENT", "LINE_COMMENT", 
  "AssemblyDialect", "AssemblyLBrace", "AssemblyBlockWS", "AssemblyBlockCOMMENT", 
  "AssemblyBlockLINE_COMMENT", "YulBreak", "YulCase", "YulContinue", "YulDefault", 
  "YulFalse", "YulFor", "YulFunction", "YulIf", "YulLeave", "YulLet", "YulSwitch", 
  "YulTrue", "YulEVMBuiltin", "YulLBrace", "YulRBrace", "YulLParen", "YulRParen", 
  "YulAssign", "YulPeriod", "YulComma", "YulArrow", "YulIdentifier", "YulHexNumber", 
  "YulDecimalNumber", "YulStringLiteral", "YulWS", "YulCOMMENT", "YulLINE_COMMENT", 
  "PragmaToken", "PragmaSemicolon", "PragmaWS", "PragmaCOMMENT", "PragmaLINE_COMMENT"
};

dfa::Vocabulary SolidityParser::_vocabulary(_literalNames, _symbolicNames);

std::vector<std::string> SolidityParser::_tokenNames;

SolidityParser::Initializer::Initializer() {
	for (size_t i = 0; i < _symbolicNames.size(); ++i) {
		std::string name = _vocabulary.getLiteralName(i);
		if (name.empty()) {
			name = _vocabulary.getSymbolicName(i);
		}

		if (name.empty()) {
			_tokenNames.push_back("<INVALID>");
		} else {
      _tokenNames.push_back(name);
    }
	}

  _serializedATN = {
    0x3, 0x608b, 0xa72a, 0x8133, 0xb9ed, 0x417c, 0x3be7, 0x7786, 0x5964, 
    0x3, 0xa3, 0x478, 0x4, 0x2, 0x9, 0x2, 0x4, 0x3, 0x9, 0x3, 0x4, 0x4, 
    0x9, 0x4, 0x4, 0x5, 0x9, 0x5, 0x4, 0x6, 0x9, 0x6, 0x4, 0x7, 0x9, 0x7, 
    0x4, 0x8, 0x9, 0x8, 0x4, 0x9, 0x9, 0x9, 0x4, 0xa, 0x9, 0xa, 0x4, 0xb, 
    0x9, 0xb, 0x4, 0xc, 0x9, 0xc, 0x4, 0xd, 0x9, 0xd, 0x4, 0xe, 0x9, 0xe, 
    0x4, 0xf, 0x9, 0xf, 0x4, 0x10, 0x9, 0x10, 0x4, 0x11, 0x9, 0x11, 0x4, 
    0x12, 0x9, 0x12, 0x4, 0x13, 0x9, 0x13, 0x4, 0x14, 0x9, 0x14, 0x4, 0x15, 
    0x9, 0x15, 0x4, 0x16, 0x9, 0x16, 0x4, 0x17, 0x9, 0x17, 0x4, 0x18, 0x9, 
    0x18, 0x4, 0x19, 0x9, 0x19, 0x4, 0x1a, 0x9, 0x1a, 0x4, 0x1b, 0x9, 0x1b, 
    0x4, 0x1c, 0x9, 0x1c, 0x4, 0x1d, 0x9, 0x1d, 0x4, 0x1e, 0x9, 0x1e, 0x4, 
    0x1f, 0x9, 0x1f, 0x4, 0x20, 0x9, 0x20, 0x4, 0x21, 0x9, 0x21, 0x4, 0x22, 
    0x9, 0x22, 0x4, 0x23, 0x9, 0x23, 0x4, 0x24, 0x9, 0x24, 0x4, 0x25, 0x9, 
    0x25, 0x4, 0x26, 0x9, 0x26, 0x4, 0x27, 0x9, 0x27, 0x4, 0x28, 0x9, 0x28, 
    0x4, 0x29, 0x9, 0x29, 0x4, 0x2a, 0x9, 0x2a, 0x4, 0x2b, 0x9, 0x2b, 0x4, 
    0x2c, 0x9, 0x2c, 0x4, 0x2d, 0x9, 0x2d, 0x4, 0x2e, 0x9, 0x2e, 0x4, 0x2f, 
    0x9, 0x2f, 0x4, 0x30, 0x9, 0x30, 0x4, 0x31, 0x9, 0x31, 0x4, 0x32, 0x9, 
    0x32, 0x4, 0x33, 0x9, 0x33, 0x4, 0x34, 0x9, 0x34, 0x4, 0x35, 0x9, 0x35, 
    0x4, 0x36, 0x9, 0x36, 0x4, 0x37, 0x9, 0x37, 0x4, 0x38, 0x9, 0x38, 0x4, 
    0x39, 0x9, 0x39, 0x4, 0x3a, 0x9, 0x3a, 0x4, 0x3b, 0x9, 0x3b, 0x4, 0x3c, 
    0x9, 0x3c, 0x4, 0x3d, 0x9, 0x3d, 0x4, 0x3e, 0x9, 0x3e, 0x4, 0x3f, 0x9, 
    0x3f, 0x4, 0x40, 0x9, 0x40, 0x4, 0x41, 0x9, 0x41, 0x4, 0x42, 0x9, 0x42, 
    0x4, 0x43, 0x9, 0x43, 0x4, 0x44, 0x9, 0x44, 0x4, 0x45, 0x9, 0x45, 0x4, 
    0x46, 0x9, 0x46, 0x4, 0x47, 0x9, 0x47, 0x4, 0x48, 0x9, 0x48, 0x4, 0x49, 
    0x9, 0x49, 0x4, 0x4a, 0x9, 0x4a, 0x4, 0x4b, 0x9, 0x4b, 0x4, 0x4c, 0x9, 
    0x4c, 0x4, 0x4d, 0x9, 0x4d, 0x4, 0x4e, 0x9, 0x4e, 0x4, 0x4f, 0x9, 0x4f, 
    0x4, 0x50, 0x9, 0x50, 0x4, 0x51, 0x9, 0x51, 0x4, 0x52, 0x9, 0x52, 0x4, 
    0x53, 0x9, 0x53, 0x3, 0x2, 0x3, 0x2, 0x3, 0x2, 0x3, 0x2, 0x3, 0x2, 0x3, 
    0x2, 0x3, 0x2, 0x3, 0x2, 0x7, 0x2, 0xaf, 0xa, 0x2, 0xc, 0x2, 0xe, 0x2, 
    0xb2, 0xb, 0x2, 0x3, 0x2, 0x3, 0x2, 0x3, 0x3, 0x3, 0x3, 0x6, 0x3, 0xb8, 
    0xa, 0x3, 0xd, 0x3, 0xe, 0x3, 0xb9, 0x3, 0x3, 0x3, 0x3, 0x3, 0x4, 0x3, 
    0x4, 0x3, 0x4, 0x3, 0x4, 0x5, 0x4, 0xc2, 0xa, 0x4, 0x3, 0x4, 0x3, 0x4, 
    0x3, 0x4, 0x3, 0x4, 0x3, 0x4, 0x3, 0x4, 0x3, 0x4, 0x3, 0x4, 0x3, 0x4, 
    0x3, 0x4, 0x5, 0x4, 0xce, 0xa, 0x4, 0x3, 0x4, 0x3, 0x4, 0x3, 0x5, 0x3, 
    0x5, 0x3, 0x5, 0x5, 0x5, 0xd5, 0xa, 0x5, 0x3, 0x6, 0x3, 0x6, 0x3, 0x7, 
    0x3, 0x7, 0x3, 0x7, 0x3, 0x7, 0x7, 0x7, 0xdd, 0xa, 0x7, 0xc, 0x7, 0xe, 
    0x7, 0xe0, 0xb, 0x7, 0x3, 0x7, 0x3, 0x7, 0x3, 0x8, 0x5, 0x8, 0xe5, 0xa, 
    0x8, 0x3, 0x8, 0x3, 0x8, 0x3, 0x8, 0x5, 0x8, 0xea, 0xa, 0x8, 0x3, 0x8, 
    0x3, 0x8, 0x7, 0x8, 0xee, 0xa, 0x8, 0xc, 0x8, 0xe, 0x8, 0xf1, 0xb, 0x8, 
    0x3, 0x8, 0x3, 0x8, 0x3, 0x9, 0x3, 0x9, 0x3, 0x9, 0x5, 0x9, 0xf8, 0xa, 
    0x9, 0x3, 0x9, 0x3, 0x9, 0x7, 0x9, 0xfc, 0xa, 0x9, 0xc, 0x9, 0xe, 0x9, 
    0xff, 0xb, 0x9, 0x3, 0x9, 0x3, 0x9, 0x3, 0xa, 0x3, 0xa, 0x3, 0xa, 0x3, 
    0xa, 0x7, 0xa, 0x107, 0xa, 0xa, 0xc, 0xa, 0xe, 0xa, 0x10a, 0xb, 0xa, 
    0x3, 0xa, 0x3, 0xa, 0x3, 0xb, 0x3, 0xb, 0x3, 0xb, 0x3, 0xb, 0x7, 0xb, 
    0x112, 0xa, 0xb, 0xc, 0xb, 0xe, 0xb, 0x115, 0xb, 0xb, 0x3, 0xc, 0x3, 
    0xc, 0x5, 0xc, 0x119, 0xa, 0xc, 0x3, 0xd, 0x3, 0xd, 0x3, 0xd, 0x3, 0xd, 
    0x3, 0xd, 0x3, 0xd, 0x3, 0xd, 0x3, 0xd, 0x3, 0xd, 0x5, 0xd, 0x124, 0xa, 
    0xd, 0x3, 0xe, 0x3, 0xe, 0x3, 0xe, 0x3, 0xe, 0x3, 0xf, 0x3, 0xf, 0x3, 
    0xf, 0x3, 0xf, 0x7, 0xf, 0x12e, 0xa, 0xf, 0xc, 0xf, 0xe, 0xf, 0x131, 
    0xb, 0xf, 0x5, 0xf, 0x133, 0xa, 0xf, 0x3, 0xf, 0x3, 0xf, 0x3, 0xf, 0x3, 
    0xf, 0x7, 0xf, 0x139, 0xa, 0xf, 0xc, 0xf, 0xe, 0xf, 0x13c, 0xb, 0xf, 
    0x5, 0xf, 0x13e, 0xa, 0xf, 0x3, 0xf, 0x5, 0xf, 0x141, 0xa, 0xf, 0x3, 
    0xf, 0x3, 0xf, 0x3, 0x10, 0x3, 0x10, 0x3, 0x10, 0x7, 0x10, 0x148, 0xa, 
    0x10, 0xc, 0x10, 0xe, 0x10, 0x14b, 0xb, 0x10, 0x3, 0x11, 0x3, 0x11, 
    0x5, 0x11, 0x14f, 0xa, 0x11, 0x3, 0x12, 0x3, 0x12, 0x3, 0x13, 0x3, 0x13, 
    0x3, 0x13, 0x7, 0x13, 0x156, 0xa, 0x13, 0xc, 0x13, 0xe, 0x13, 0x159, 
    0xb, 0x13, 0x3, 0x14, 0x3, 0x14, 0x5, 0x14, 0x15d, 0xa, 0x14, 0x3, 0x14, 
    0x5, 0x14, 0x160, 0xa, 0x14, 0x3, 0x15, 0x3, 0x15, 0x3, 0x15, 0x5, 0x15, 
    0x165, 0xa, 0x15, 0x3, 0x15, 0x3, 0x15, 0x3, 0x15, 0x3, 0x15, 0x3, 0x15, 
    0x3, 0x15, 0x3, 0x15, 0x3, 0x15, 0x3, 0x15, 0x3, 0x15, 0x3, 0x15, 0x7, 
    0x15, 0x172, 0xa, 0x15, 0xc, 0x15, 0xe, 0x15, 0x175, 0xb, 0x15, 0x3, 
    0x15, 0x3, 0x15, 0x3, 0x16, 0x3, 0x16, 0x3, 0x17, 0x3, 0x17, 0x3, 0x17, 
    0x3, 0x17, 0x3, 0x17, 0x7, 0x17, 0x180, 0xa, 0x17, 0xc, 0x17, 0xe, 0x17, 
    0x183, 0xb, 0x17, 0x3, 0x17, 0x3, 0x17, 0x5, 0x17, 0x187, 0xa, 0x17, 
    0x3, 0x18, 0x3, 0x18, 0x3, 0x18, 0x3, 0x18, 0x5, 0x18, 0x18d, 0xa, 0x18, 
    0x3, 0x18, 0x3, 0x18, 0x5, 0x18, 0x191, 0xa, 0x18, 0x3, 0x18, 0x3, 0x18, 
    0x3, 0x18, 0x3, 0x18, 0x3, 0x18, 0x3, 0x18, 0x3, 0x18, 0x3, 0x18, 0x3, 
    0x18, 0x3, 0x18, 0x3, 0x18, 0x3, 0x18, 0x3, 0x18, 0x3, 0x18, 0x3, 0x18, 
    0x3, 0x18, 0x3, 0x18, 0x7, 0x18, 0x1a4, 0xa, 0x18, 0xc, 0x18, 0xe, 0x18, 
    0x1a7, 0xb, 0x18, 0x3, 0x18, 0x3, 0x18, 0x3, 0x18, 0x3, 0x18, 0x3, 0x18, 
    0x5, 0x18, 0x1ae, 0xa, 0x18, 0x3, 0x18, 0x3, 0x18, 0x5, 0x18, 0x1b2, 
    0xa, 0x18, 0x3, 0x19, 0x3, 0x19, 0x3, 0x19, 0x3, 0x19, 0x5, 0x19, 0x1b8, 
    0xa, 0x19, 0x3, 0x19, 0x5, 0x19, 0x1bb, 0xa, 0x19, 0x3, 0x19, 0x3, 0x19, 
    0x3, 0x19, 0x3, 0x19, 0x3, 0x19, 0x3, 0x19, 0x3, 0x19, 0x7, 0x19, 0x1c4, 
    0xa, 0x19, 0xc, 0x19, 0xe, 0x19, 0x1c7, 0xb, 0x19, 0x3, 0x19, 0x3, 0x19, 
    0x5, 0x19, 0x1cb, 0xa, 0x19, 0x3, 0x1a, 0x3, 0x1a, 0x3, 0x1a, 0x3, 0x1a, 
    0x3, 0x1a, 0x3, 0x1a, 0x3, 0x1a, 0x3, 0x1a, 0x3, 0x1a, 0x3, 0x1a, 0x3, 
    0x1a, 0x3, 0x1a, 0x3, 0x1a, 0x3, 0x1a, 0x3, 0x1a, 0x3, 0x1a, 0x3, 0x1a, 
    0x3, 0x1a, 0x3, 0x1a, 0x7, 0x1a, 0x1e0, 0xa, 0x1a, 0xc, 0x1a, 0xe, 0x1a, 
    0x1e3, 0xb, 0x1a, 0x3, 0x1a, 0x3, 0x1a, 0x5, 0x1a, 0x1e7, 0xa, 0x1a, 
    0x3, 0x1b, 0x3, 0x1b, 0x3, 0x1b, 0x3, 0x1b, 0x6, 0x1b, 0x1ed, 0xa, 0x1b, 
    0xd, 0x1b, 0xe, 0x1b, 0x1ee, 0x3, 0x1b, 0x3, 0x1b, 0x3, 0x1c, 0x3, 0x1c, 
    0x3, 0x1c, 0x3, 0x1c, 0x3, 0x1d, 0x3, 0x1d, 0x3, 0x1d, 0x3, 0x1d, 0x3, 
    0x1d, 0x3, 0x1d, 0x7, 0x1d, 0x1fd, 0xa, 0x1d, 0xc, 0x1d, 0xe, 0x1d, 
    0x200, 0xb, 0x1d, 0x3, 0x1d, 0x3, 0x1d, 0x3, 0x1e, 0x3, 0x1e, 0x3, 0x1e, 
    0x3, 0x1e, 0x3, 0x1e, 0x3, 0x1e, 0x3, 0x1e, 0x3, 0x1e, 0x3, 0x1e, 0x3, 
    0x1e, 0x3, 0x1e, 0x3, 0x1e, 0x3, 0x1e, 0x3, 0x1e, 0x3, 0x1e, 0x3, 0x1e, 
    0x3, 0x1e, 0x3, 0x1e, 0x3, 0x1e, 0x3, 0x1e, 0x7, 0x1e, 0x218, 0xa, 0x1e, 
    0xc, 0x1e, 0xe, 0x1e, 0x21b, 0xb, 0x1e, 0x3, 0x1e, 0x3, 0x1e, 0x3, 0x1e, 
    0x5, 0x1e, 0x220, 0xa, 0x1e, 0x3, 0x1e, 0x3, 0x1e, 0x3, 0x1f, 0x3, 0x1f, 
    0x5, 0x1f, 0x226, 0xa, 0x1f, 0x3, 0x1f, 0x5, 0x1f, 0x229, 0xa, 0x1f, 
    0x3, 0x20, 0x3, 0x20, 0x3, 0x20, 0x3, 0x20, 0x3, 0x20, 0x3, 0x20, 0x7, 
    0x20, 0x231, 0xa, 0x20, 0xc, 0x20, 0xe, 0x20, 0x234, 0xb, 0x20, 0x5, 
    0x20, 0x236, 0xa, 0x20, 0x3, 0x20, 0x3, 0x20, 0x5, 0x20, 0x23a, 0xa, 
    0x20, 0x3, 0x20, 0x3, 0x20, 0x3, 0x21, 0x3, 0x21, 0x3, 0x21, 0x3, 0x21, 
    0x3, 0x21, 0x5, 0x21, 0x243, 0xa, 0x21, 0x3, 0x21, 0x3, 0x21, 0x3, 0x22, 
    0x3, 0x22, 0x3, 0x22, 0x3, 0x22, 0x3, 0x22, 0x5, 0x22, 0x24c, 0xa, 0x22, 
    0x3, 0x22, 0x3, 0x22, 0x3, 0x22, 0x5, 0x22, 0x251, 0xa, 0x22, 0x3, 0x22, 
    0x7, 0x22, 0x254, 0xa, 0x22, 0xc, 0x22, 0xe, 0x22, 0x257, 0xb, 0x22, 
    0x3, 0x23, 0x3, 0x23, 0x3, 0x23, 0x3, 0x23, 0x3, 0x23, 0x3, 0x23, 0x3, 
    0x23, 0x3, 0x23, 0x3, 0x23, 0x3, 0x23, 0x3, 0x23, 0x3, 0x23, 0x5, 0x23, 
    0x265, 0xa, 0x23, 0x3, 0x24, 0x3, 0x24, 0x3, 0x24, 0x5, 0x24, 0x26a, 
    0xa, 0x24, 0x3, 0x24, 0x3, 0x24, 0x3, 0x24, 0x3, 0x24, 0x3, 0x24, 0x3, 
    0x24, 0x3, 0x24, 0x3, 0x24, 0x3, 0x24, 0x7, 0x24, 0x275, 0xa, 0x24, 
    0xc, 0x24, 0xe, 0x24, 0x278, 0xb, 0x24, 0x3, 0x24, 0x3, 0x24, 0x3, 0x24, 
    0x3, 0x24, 0x3, 0x24, 0x5, 0x24, 0x27f, 0xa, 0x24, 0x3, 0x25, 0x3, 0x25, 
    0x5, 0x25, 0x283, 0xa, 0x25, 0x3, 0x25, 0x3, 0x25, 0x3, 0x26, 0x3, 0x26, 
    0x3, 0x27, 0x3, 0x27, 0x3, 0x27, 0x3, 0x27, 0x3, 0x27, 0x3, 0x27, 0x3, 
    0x27, 0x3, 0x27, 0x3, 0x27, 0x3, 0x27, 0x3, 0x27, 0x3, 0x27, 0x3, 0x27, 
    0x3, 0x27, 0x3, 0x27, 0x3, 0x27, 0x3, 0x27, 0x3, 0x27, 0x5, 0x27, 0x29b, 
    0xa, 0x27, 0x5, 0x27, 0x29d, 0xa, 0x27, 0x3, 0x27, 0x3, 0x27, 0x3, 0x27, 
    0x3, 0x27, 0x3, 0x27, 0x3, 0x27, 0x3, 0x27, 0x3, 0x27, 0x3, 0x27, 0x3, 
    0x27, 0x3, 0x27, 0x3, 0x27, 0x3, 0x27, 0x3, 0x27, 0x3, 0x27, 0x3, 0x27, 
    0x3, 0x27, 0x3, 0x27, 0x3, 0x27, 0x3, 0x27, 0x3, 0x27, 0x3, 0x27, 0x3, 
    0x27, 0x3, 0x27, 0x3, 0x27, 0x3, 0x27, 0x3, 0x27, 0x3, 0x27, 0x3, 0x27, 
    0x3, 0x27, 0x3, 0x27, 0x3, 0x27, 0x3, 0x27, 0x3, 0x27, 0x3, 0x27, 0x3, 
    0x27, 0x3, 0x27, 0x3, 0x27, 0x3, 0x27, 0x3, 0x27, 0x3, 0x27, 0x3, 0x27, 
    0x3, 0x27, 0x3, 0x27, 0x3, 0x27, 0x3, 0x27, 0x5, 0x27, 0x2cd, 0xa, 0x27, 
    0x3, 0x27, 0x3, 0x27, 0x3, 0x27, 0x3, 0x27, 0x5, 0x27, 0x2d3, 0xa, 0x27, 
    0x3, 0x27, 0x3, 0x27, 0x5, 0x27, 0x2d7, 0xa, 0x27, 0x3, 0x27, 0x3, 0x27, 
    0x3, 0x27, 0x3, 0x27, 0x3, 0x27, 0x5, 0x27, 0x2de, 0xa, 0x27, 0x3, 0x27, 
    0x3, 0x27, 0x3, 0x27, 0x3, 0x27, 0x3, 0x27, 0x7, 0x27, 0x2e5, 0xa, 0x27, 
    0xc, 0x27, 0xe, 0x27, 0x2e8, 0xb, 0x27, 0x5, 0x27, 0x2ea, 0xa, 0x27, 
    0x3, 0x27, 0x3, 0x27, 0x3, 0x27, 0x3, 0x27, 0x3, 0x27, 0x7, 0x27, 0x2f1, 
    0xa, 0x27, 0xc, 0x27, 0xe, 0x27, 0x2f4, 0xb, 0x27, 0x3, 0x28, 0x3, 0x28, 
    0x3, 0x29, 0x3, 0x29, 0x5, 0x29, 0x2fa, 0xa, 0x29, 0x3, 0x29, 0x3, 0x29, 
    0x5, 0x29, 0x2fe, 0xa, 0x29, 0x7, 0x29, 0x300, 0xa, 0x29, 0xc, 0x29, 
    0xe, 0x29, 0x303, 0xb, 0x29, 0x3, 0x29, 0x3, 0x29, 0x3, 0x2a, 0x3, 0x2a, 
    0x3, 0x2a, 0x3, 0x2a, 0x7, 0x2a, 0x30b, 0xa, 0x2a, 0xc, 0x2a, 0xe, 0x2a, 
    0x30e, 0xb, 0x2a, 0x3, 0x2a, 0x3, 0x2a, 0x3, 0x2b, 0x3, 0x2b, 0x3, 0x2c, 
    0x3, 0x2c, 0x3, 0x2c, 0x3, 0x2c, 0x3, 0x2c, 0x5, 0x2c, 0x319, 0xa, 0x2c, 
    0x3, 0x2d, 0x3, 0x2d, 0x3, 0x2e, 0x6, 0x2e, 0x31e, 0xa, 0x2e, 0xd, 0x2e, 
    0xe, 0x2e, 0x31f, 0x3, 0x2f, 0x6, 0x2f, 0x323, 0xa, 0x2f, 0xd, 0x2f, 
    0xe, 0x2f, 0x324, 0x3, 0x30, 0x6, 0x30, 0x328, 0xa, 0x30, 0xd, 0x30, 
    0xe, 0x30, 0x329, 0x3, 0x31, 0x3, 0x31, 0x5, 0x31, 0x32e, 0xa, 0x31, 
    0x3, 0x32, 0x3, 0x32, 0x7, 0x32, 0x332, 0xa, 0x32, 0xc, 0x32, 0xe, 0x32, 
    0x335, 0xb, 0x32, 0x3, 0x32, 0x3, 0x32, 0x3, 0x33, 0x3, 0x33, 0x3, 0x33, 
    0x3, 0x33, 0x3, 0x33, 0x3, 0x33, 0x3, 0x33, 0x3, 0x33, 0x3, 0x33, 0x3, 
    0x33, 0x3, 0x33, 0x3, 0x33, 0x5, 0x33, 0x345, 0xa, 0x33, 0x3, 0x34, 
    0x3, 0x34, 0x5, 0x34, 0x349, 0xa, 0x34, 0x3, 0x35, 0x3, 0x35, 0x3, 0x35, 
    0x3, 0x35, 0x3, 0x35, 0x3, 0x35, 0x3, 0x35, 0x5, 0x35, 0x352, 0xa, 0x35, 
    0x3, 0x36, 0x3, 0x36, 0x3, 0x36, 0x3, 0x36, 0x5, 0x36, 0x358, 0xa, 0x36, 
    0x3, 0x36, 0x3, 0x36, 0x5, 0x36, 0x35c, 0xa, 0x36, 0x3, 0x36, 0x5, 0x36, 
    0x35f, 0xa, 0x36, 0x3, 0x36, 0x3, 0x36, 0x3, 0x36, 0x3, 0x37, 0x3, 0x37, 
    0x3, 0x37, 0x3, 0x37, 0x3, 0x37, 0x3, 0x37, 0x3, 0x38, 0x3, 0x38, 0x3, 
    0x38, 0x3, 0x38, 0x3, 0x38, 0x3, 0x38, 0x3, 0x38, 0x3, 0x38, 0x3, 0x39, 
    0x3, 0x39, 0x3, 0x39, 0x3, 0x3a, 0x3, 0x3a, 0x3, 0x3a, 0x3, 0x3b, 0x3, 
    0x3b, 0x3, 0x3b, 0x3, 0x3b, 0x3, 0x3b, 0x3, 0x3b, 0x3, 0x3b, 0x5, 0x3b, 
    0x37f, 0xa, 0x3b, 0x3, 0x3b, 0x3, 0x3b, 0x6, 0x3b, 0x383, 0xa, 0x3b, 
    0xd, 0x3b, 0xe, 0x3b, 0x384, 0x3, 0x3c, 0x3, 0x3c, 0x5, 0x3c, 0x389, 
    0xa, 0x3c, 0x3, 0x3c, 0x3, 0x3c, 0x3, 0x3c, 0x3, 0x3c, 0x5, 0x3c, 0x38f, 
    0xa, 0x3c, 0x3, 0x3c, 0x3, 0x3c, 0x3, 0x3d, 0x3, 0x3d, 0x5, 0x3d, 0x395, 
    0xa, 0x3d, 0x3, 0x3d, 0x3, 0x3d, 0x3, 0x3e, 0x3, 0x3e, 0x3, 0x3e, 0x3, 
    0x3e, 0x3, 0x3e, 0x3, 0x3f, 0x3, 0x3f, 0x5, 0x3f, 0x3a0, 0xa, 0x3f, 
    0x3, 0x3f, 0x3, 0x3f, 0x7, 0x3f, 0x3a4, 0xa, 0x3f, 0xc, 0x3f, 0xe, 0x3f, 
    0x3a7, 0xb, 0x3f, 0x3, 0x3f, 0x3, 0x3f, 0x3, 0x40, 0x3, 0x40, 0x3, 0x40, 
    0x7, 0x40, 0x3ae, 0xa, 0x40, 0xc, 0x40, 0xe, 0x40, 0x3b1, 0xb, 0x40, 
    0x3, 0x41, 0x3, 0x41, 0x7, 0x41, 0x3b5, 0xa, 0x41, 0xc, 0x41, 0xe, 0x41, 
    0x3b8, 0xb, 0x41, 0x3, 0x41, 0x3, 0x41, 0x3, 0x41, 0x3, 0x41, 0x5, 0x41, 
    0x3be, 0xa, 0x41, 0x7, 0x41, 0x3c0, 0xa, 0x41, 0xc, 0x41, 0xe, 0x41, 
    0x3c3, 0xb, 0x41, 0x3, 0x41, 0x3, 0x41, 0x3, 0x42, 0x3, 0x42, 0x3, 0x42, 
    0x5, 0x42, 0x3ca, 0xa, 0x42, 0x3, 0x42, 0x3, 0x42, 0x3, 0x42, 0x3, 0x42, 
    0x5, 0x42, 0x3d0, 0xa, 0x42, 0x3, 0x42, 0x3, 0x42, 0x3, 0x43, 0x3, 0x43, 
    0x3, 0x43, 0x3, 0x44, 0x3, 0x44, 0x3, 0x44, 0x3, 0x44, 0x3, 0x44, 0x3, 
    0x44, 0x3, 0x44, 0x3, 0x45, 0x3, 0x45, 0x5, 0x45, 0x3e0, 0xa, 0x45, 
    0x3, 0x46, 0x3, 0x46, 0x3, 0x46, 0x3, 0x46, 0x3, 0x46, 0x3, 0x46, 0x3, 
    0x46, 0x3, 0x46, 0x3, 0x46, 0x3, 0x46, 0x3, 0x46, 0x5, 0x46, 0x3ed, 
    0xa, 0x46, 0x3, 0x47, 0x3, 0x47, 0x7, 0x47, 0x3f1, 0xa, 0x47, 0xc, 0x47, 
    0xe, 0x47, 0x3f4, 0xb, 0x47, 0x3, 0x47, 0x3, 0x47, 0x3, 0x48, 0x3, 0x48, 
    0x3, 0x48, 0x3, 0x48, 0x5, 0x48, 0x3fc, 0xa, 0x48, 0x3, 0x48, 0x3, 0x48, 
    0x3, 0x48, 0x3, 0x48, 0x7, 0x48, 0x402, 0xa, 0x48, 0xc, 0x48, 0xe, 0x48, 
    0x405, 0xb, 0x48, 0x3, 0x48, 0x3, 0x48, 0x5, 0x48, 0x409, 0xa, 0x48, 
    0x5, 0x48, 0x40b, 0xa, 0x48, 0x3, 0x49, 0x3, 0x49, 0x3, 0x49, 0x3, 0x49, 
    0x3, 0x49, 0x3, 0x49, 0x3, 0x49, 0x6, 0x49, 0x414, 0xa, 0x49, 0xd, 0x49, 
    0xe, 0x49, 0x415, 0x3, 0x49, 0x3, 0x49, 0x3, 0x49, 0x5, 0x49, 0x41b, 
    0xa, 0x49, 0x3, 0x4a, 0x3, 0x4a, 0x3, 0x4a, 0x3, 0x4a, 0x3, 0x4b, 0x3, 
    0x4b, 0x3, 0x4b, 0x3, 0x4b, 0x3, 0x4b, 0x3, 0x4b, 0x3, 0x4c, 0x3, 0x4c, 
    0x3, 0x4c, 0x3, 0x4c, 0x3, 0x4d, 0x3, 0x4d, 0x3, 0x4d, 0x6, 0x4d, 0x42e, 
    0xa, 0x4d, 0xd, 0x4d, 0xe, 0x4d, 0x42f, 0x3, 0x4d, 0x3, 0x4d, 0x5, 0x4d, 
    0x434, 0xa, 0x4d, 0x3, 0x4d, 0x3, 0x4d, 0x5, 0x4d, 0x438, 0xa, 0x4d, 
    0x3, 0x4e, 0x3, 0x4e, 0x3, 0x4e, 0x3, 0x4e, 0x3, 0x4e, 0x3, 0x4e, 0x7, 
    0x4e, 0x440, 0xa, 0x4e, 0xc, 0x4e, 0xe, 0x4e, 0x443, 0xb, 0x4e, 0x5, 
    0x4e, 0x445, 0xa, 0x4e, 0x3, 0x4e, 0x3, 0x4e, 0x3, 0x4e, 0x3, 0x4e, 
    0x3, 0x4e, 0x7, 0x4e, 0x44c, 0xa, 0x4e, 0xc, 0x4e, 0xe, 0x4e, 0x44f, 
    0xb, 0x4e, 0x5, 0x4e, 0x451, 0xa, 0x4e, 0x3, 0x4e, 0x3, 0x4e, 0x3, 0x4f, 
    0x3, 0x4f, 0x3, 0x4f, 0x7, 0x4f, 0x458, 0xa, 0x4f, 0xc, 0x4f, 0xe, 0x4f, 
    0x45b, 0xb, 0x4f, 0x3, 0x50, 0x3, 0x50, 0x3, 0x50, 0x3, 0x50, 0x3, 0x50, 
    0x7, 0x50, 0x462, 0xa, 0x50, 0xc, 0x50, 0xe, 0x50, 0x465, 0xb, 0x50, 
    0x5, 0x50, 0x467, 0xa, 0x50, 0x3, 0x50, 0x3, 0x50, 0x3, 0x51, 0x3, 0x51, 
    0x3, 0x52, 0x3, 0x52, 0x3, 0x52, 0x3, 0x52, 0x5, 0x52, 0x471, 0xa, 0x52, 
    0x3, 0x53, 0x3, 0x53, 0x3, 0x53, 0x5, 0x53, 0x476, 0xa, 0x53, 0x3, 0x53, 
    0x3, 0x113, 0x4, 0x42, 0x4c, 0x54, 0x2, 0x4, 0x6, 0x8, 0xa, 0xc, 0xe, 
    0x10, 0x12, 0x14, 0x16, 0x18, 0x1a, 0x1c, 0x1e, 0x20, 0x22, 0x24, 0x26, 
    0x28, 0x2a, 0x2c, 0x2e, 0x30, 0x32, 0x34, 0x36, 0x38, 0x3a, 0x3c, 0x3e, 
    0x40, 0x42, 0x44, 0x46, 0x48, 0x4a, 0x4c, 0x4e, 0x50, 0x52, 0x54, 0x56, 
    0x58, 0x5a, 0x5c, 0x5e, 0x60, 0x62, 0x64, 0x66, 0x68, 0x6a, 0x6c, 0x6e, 
    0x70, 0x72, 0x74, 0x76, 0x78, 0x7a, 0x7c, 0x7e, 0x80, 0x82, 0x84, 0x86, 
    0x88, 0x8a, 0x8c, 0x8e, 0x90, 0x92, 0x94, 0x96, 0x98, 0x9a, 0x9c, 0x9e, 
    0xa0, 0xa2, 0xa4, 0x2, 0x13, 0x5, 0x2, 0x19, 0x19, 0x27, 0x27, 0x31, 
    0x32, 0x5, 0x2, 0x30, 0x30, 0x33, 0x33, 0x41, 0x41, 0x4, 0x2, 0x1a, 
    0x1a, 0x34, 0x34, 0x5, 0x2, 0xd, 0xd, 0x2b, 0x2b, 0x38, 0x38, 0x5, 0x2, 
    0x13, 0x13, 0x65, 0x65, 0x70, 0x73, 0x3, 0x2, 0x66, 0x68, 0x3, 0x2, 
    0x64, 0x65, 0x3, 0x2, 0x61, 0x63, 0x3, 0x2, 0x6c, 0x6f, 0x3, 0x2, 0x6a, 
    0x6b, 0x3, 0x2, 0x72, 0x73, 0x3, 0x2, 0x4f, 0x5a, 0x4, 0x2, 0x1d, 0x1d, 
    0x7a, 0x7a, 0x4, 0x2, 0x1b, 0x1b, 0x3b, 0x3b, 0x3, 0x2, 0x78, 0x79, 
    0x4, 0x2, 0x8f, 0x8f, 0x98, 0x98, 0x4, 0x2, 0x87, 0x87, 0x8e, 0x8e, 
    0x2, 0x4f8, 0x2, 0xb0, 0x3, 0x2, 0x2, 0x2, 0x4, 0xb5, 0x3, 0x2, 0x2, 
    0x2, 0x6, 0xbd, 0x3, 0x2, 0x2, 0x2, 0x8, 0xd1, 0x3, 0x2, 0x2, 0x2, 0xa, 
    0xd6, 0x3, 0x2, 0x2, 0x2, 0xc, 0xd8, 0x3, 0x2, 0x2, 0x2, 0xe, 0xe4, 
    0x3, 0x2, 0x2, 0x2, 0x10, 0xf4, 0x3, 0x2, 0x2, 0x2, 0x12, 0x102, 0x3, 
    0x2, 0x2, 0x2, 0x14, 0x10d, 0x3, 0x2, 0x2, 0x2, 0x16, 0x116, 0x3, 0x2, 
    0x2, 0x2, 0x18, 0x123, 0x3, 0x2, 0x2, 0x2, 0x1a, 0x125, 0x3, 0x2, 0x2, 
    0x2, 0x1c, 0x129, 0x3, 0x2, 0x2, 0x2, 0x1e, 0x144, 0x3, 0x2, 0x2, 0x2, 
    0x20, 0x14c, 0x3, 0x2, 0x2, 0x2, 0x22, 0x150, 0x3, 0x2, 0x2, 0x2, 0x24, 
    0x152, 0x3, 0x2, 0x2, 0x2, 0x26, 0x15a, 0x3, 0x2, 0x2, 0x2, 0x28, 0x161, 
    0x3, 0x2, 0x2, 0x2, 0x2a, 0x178, 0x3, 0x2, 0x2, 0x2, 0x2c, 0x17a, 0x3, 
    0x2, 0x2, 0x2, 0x2e, 0x188, 0x3, 0x2, 0x2, 0x2, 0x30, 0x1b3, 0x3, 0x2, 
    0x2, 0x2, 0x32, 0x1cc, 0x3, 0x2, 0x2, 0x2, 0x34, 0x1e8, 0x3, 0x2, 0x2, 
    0x2, 0x36, 0x1f2, 0x3, 0x2, 0x2, 0x2, 0x38, 0x1f6, 0x3, 0x2, 0x2, 0x2, 
    0x3a, 0x203, 0x3, 0x2, 0x2, 0x2, 0x3c, 0x223, 0x3, 0x2, 0x2, 0x2, 0x3e, 
    0x22a, 0x3, 0x2, 0x2, 0x2, 0x40, 0x23d, 0x3, 0x2, 0x2, 0x2, 0x42, 0x24b, 
    0x3, 0x2, 0x2, 0x2, 0x44, 0x264, 0x3, 0x2, 0x2, 0x2, 0x46, 0x266, 0x3, 
    0x2, 0x2, 0x2, 0x48, 0x280, 0x3, 0x2, 0x2, 0x2, 0x4a, 0x286, 0x3, 0x2, 
    0x2, 0x2, 0x4c, 0x29c, 0x3, 0x2, 0x2, 0x2, 0x4e, 0x2f5, 0x3, 0x2, 0x2, 
    0x2, 0x50, 0x2f7, 0x3, 0x2, 0x2, 0x2, 0x52, 0x306, 0x3, 0x2, 0x2, 0x2, 
    0x54, 0x311, 0x3, 0x2, 0x2, 0x2, 0x56, 0x318, 0x3, 0x2, 0x2, 0x2, 0x58, 
    0x31a, 0x3, 0x2, 0x2, 0x2, 0x5a, 0x31d, 0x3, 0x2, 0x2, 0x2, 0x5c, 0x322, 
    0x3, 0x2, 0x2, 0x2, 0x5e, 0x327, 0x3, 0x2, 0x2, 0x2, 0x60, 0x32b, 0x3, 
    0x2, 0x2, 0x2, 0x62, 0x32f, 0x3, 0x2, 0x2, 0x2, 0x64, 0x344, 0x3, 0x2, 
    0x2, 0x2, 0x66, 0x348, 0x3, 0x2, 0x2, 0x2, 0x68, 0x34a, 0x3, 0x2, 0x2, 
    0x2, 0x6a, 0x353, 0x3, 0x2, 0x2, 0x2, 0x6c, 0x363, 0x3, 0x2, 0x2, 0x2, 
    0x6e, 0x369, 0x3, 0x2, 0x2, 0x2, 0x70, 0x371, 0x3, 0x2, 0x2, 0x2, 0x72, 
    0x374, 0x3, 0x2, 0x2, 0x2, 0x74, 0x377, 0x3, 0x2, 0x2, 0x2, 0x76, 0x386, 
    0x3, 0x2, 0x2, 0x2, 0x78, 0x392, 0x3, 0x2, 0x2, 0x2, 0x7a, 0x398, 0x3, 
    0x2, 0x2, 0x2, 0x7c, 0x39d, 0x3, 0x2, 0x2, 0x2, 0x7e, 0x3aa, 0x3, 0x2, 
    0x2, 0x2, 0x80, 0x3b2, 0x3, 0x2, 0x2, 0x2, 0x82, 0x3cf, 0x3, 0x2, 0x2, 
    0x2, 0x84, 0x3d3, 0x3, 0x2, 0x2, 0x2, 0x86, 0x3d6, 0x3, 0x2, 0x2, 0x2, 
    0x88, 0x3df, 0x3, 0x2, 0x2, 0x2, 0x8a, 0x3ec, 0x3, 0x2, 0x2, 0x2, 0x8c, 
    0x3ee, 0x3, 0x2, 0x2, 0x2, 0x8e, 0x40a, 0x3, 0x2, 0x2, 0x2, 0x90, 0x41a, 
    0x3, 0x2, 0x2, 0x2, 0x92, 0x41c, 0x3, 0x2, 0x2, 0x2, 0x94, 0x420, 0x3, 
    0x2, 0x2, 0x2, 0x96, 0x426, 0x3, 0x2, 0x2, 0x2, 0x98, 0x42a, 0x3, 0x2, 
    0x2, 0x2, 0x9a, 0x439, 0x3, 0x2, 0x2, 0x2, 0x9c, 0x454, 0x3, 0x2, 0x2, 
    0x2, 0x9e, 0x45c, 0x3, 0x2, 0x2, 0x2, 0xa0, 0x46a, 0x3, 0x2, 0x2, 0x2, 
    0xa2, 0x470, 0x3, 0x2, 0x2, 0x2, 0xa4, 0x475, 0x3, 0x2, 0x2, 0x2, 0xa6, 
    0xaf, 0x5, 0x4, 0x3, 0x2, 0xa7, 0xaf, 0x5, 0x6, 0x4, 0x2, 0xa8, 0xaf, 
    0x5, 0xe, 0x8, 0x2, 0xa9, 0xaf, 0x5, 0x10, 0x9, 0x2, 0xaa, 0xaf, 0x5, 
    0x12, 0xa, 0x2, 0xab, 0xaf, 0x5, 0x2e, 0x18, 0x2, 0xac, 0xaf, 0x5, 0x34, 
    0x1b, 0x2, 0xad, 0xaf, 0x5, 0x38, 0x1d, 0x2, 0xae, 0xa6, 0x3, 0x2, 0x2, 
    0x2, 0xae, 0xa7, 0x3, 0x2, 0x2, 0x2, 0xae, 0xa8, 0x3, 0x2, 0x2, 0x2, 
    0xae, 0xa9, 0x3, 0x2, 0x2, 0x2, 0xae, 0xaa, 0x3, 0x2, 0x2, 0x2, 0xae, 
    0xab, 0x3, 0x2, 0x2, 0x2, 0xae, 0xac, 0x3, 0x2, 0x2, 0x2, 0xae, 0xad, 
    0x3, 0x2, 0x2, 0x2, 0xaf, 0xb2, 0x3, 0x2, 0x2, 0x2, 0xb0, 0xae, 0x3, 
    0x2, 0x2, 0x2, 0xb0, 0xb1, 0x3, 0x2, 0x2, 0x2, 0xb1, 0xb3, 0x3, 0x2, 
    0x2, 0x2, 0xb2, 0xb0, 0x3, 0x2, 0x2, 0x2, 0xb3, 0xb4, 0x7, 0x2, 0x2, 
    0x3, 0xb4, 0x3, 0x3, 0x2, 0x2, 0x2, 0xb5, 0xb7, 0x7, 0x4, 0x2, 0x2, 
    0xb6, 0xb8, 0x7, 0x9f, 0x2, 0x2, 0xb7, 0xb6, 0x3, 0x2, 0x2, 0x2, 0xb8, 
    0xb9, 0x3, 0x2, 0x2, 0x2, 0xb9, 0xb7, 0x3, 0x2, 0x2, 0x2, 0xb9, 0xba, 
    0x3, 0x2, 0x2, 0x2, 0xba, 0xbb, 0x3, 0x2, 0x2, 0x2, 0xbb, 0xbc, 0x7, 
    0xa0, 0x2, 0x2, 0xbc, 0x5, 0x3, 0x2, 0x2, 0x2, 0xbd, 0xcd, 0x7, 0x24, 
    0x2, 0x2, 0xbe, 0xc1, 0x5, 0xa, 0x6, 0x2, 0xbf, 0xc0, 0x7, 0x8, 0x2, 
    0x2, 0xc0, 0xc2, 0x5, 0x54, 0x2b, 0x2, 0xc1, 0xbf, 0x3, 0x2, 0x2, 0x2, 
    0xc1, 0xc2, 0x3, 0x2, 0x2, 0x2, 0xc2, 0xce, 0x3, 0x2, 0x2, 0x2, 0xc3, 
    0xc4, 0x5, 0xc, 0x7, 0x2, 0xc4, 0xc5, 0x7, 0x1d, 0x2, 0x2, 0xc5, 0xc6, 
    0x5, 0xa, 0x6, 0x2, 0xc6, 0xce, 0x3, 0x2, 0x2, 0x2, 0xc7, 0xc8, 0x7, 
    0x66, 0x2, 0x2, 0xc8, 0xc9, 0x7, 0x8, 0x2, 0x2, 0xc9, 0xca, 0x5, 0x54, 
    0x2b, 0x2, 0xca, 0xcb, 0x7, 0x1d, 0x2, 0x2, 0xcb, 0xcc, 0x5, 0xa, 0x6, 
    0x2, 0xcc, 0xce, 0x3, 0x2, 0x2, 0x2, 0xcd, 0xbe, 0x3, 0x2, 0x2, 0x2, 
    0xcd, 0xc3, 0x3, 0x2, 0x2, 0x2, 0xcd, 0xc7, 0x3, 0x2, 0x2, 0x2, 0xce, 
    0xcf, 0x3, 0x2, 0x2, 0x2, 0xcf, 0xd0, 0x7, 0x4b, 0x2, 0x2, 0xd0, 0x7, 
    0x3, 0x2, 0x2, 0x2, 0xd1, 0xd4, 0x5, 0x54, 0x2b, 0x2, 0xd2, 0xd3, 0x7, 
    0x8, 0x2, 0x2, 0xd3, 0xd5, 0x5, 0x54, 0x2b, 0x2, 0xd4, 0xd2, 0x3, 0x2, 
    0x2, 0x2, 0xd4, 0xd5, 0x3, 0x2, 0x2, 0x2, 0xd5, 0x9, 0x3, 0x2, 0x2, 
    0x2, 0xd6, 0xd7, 0x7, 0x75, 0x2, 0x2, 0xd7, 0xb, 0x3, 0x2, 0x2, 0x2, 
    0xd8, 0xd9, 0x7, 0x48, 0x2, 0x2, 0xd9, 0xde, 0x5, 0x8, 0x5, 0x2, 0xda, 
    0xdb, 0x7, 0x5b, 0x2, 0x2, 0xdb, 0xdd, 0x5, 0x8, 0x5, 0x2, 0xdc, 0xda, 
    0x3, 0x2, 0x2, 0x2, 0xdd, 0xe0, 0x3, 0x2, 0x2, 0x2, 0xde, 0xdc, 0x3, 
    0x2, 0x2, 0x2, 0xde, 0xdf, 0x3, 0x2, 0x2, 0x2, 0xdf, 0xe1, 0x3, 0x2, 
    0x2, 0x2, 0xe0, 0xde, 0x3, 0x2, 0x2, 0x2, 0xe1, 0xe2, 0x7, 0x49, 0x2, 
    0x2, 0xe2, 0xd, 0x3, 0x2, 0x2, 0x2, 0xe3, 0xe5, 0x7, 0x5, 0x2, 0x2, 
    0xe4, 0xe3, 0x3, 0x2, 0x2, 0x2, 0xe4, 0xe5, 0x3, 0x2, 0x2, 0x2, 0xe5, 
    0xe6, 0x3, 0x2, 0x2, 0x2, 0xe6, 0xe7, 0x7, 0x12, 0x2, 0x2, 0xe7, 0xe9, 
    0x5, 0x54, 0x2b, 0x2, 0xe8, 0xea, 0x5, 0x14, 0xb, 0x2, 0xe9, 0xe8, 0x3, 
    0x2, 0x2, 0x2, 0xe9, 0xea, 0x3, 0x2, 0x2, 0x2, 0xea, 0xeb, 0x3, 0x2, 
    0x2, 0x2, 0xeb, 0xef, 0x7, 0x48, 0x2, 0x2, 0xec, 0xee, 0x5, 0x18, 0xd, 
    0x2, 0xed, 0xec, 0x3, 0x2, 0x2, 0x2, 0xee, 0xf1, 0x3, 0x2, 0x2, 0x2, 
    0xef, 0xed, 0x3, 0x2, 0x2, 0x2, 0xef, 0xf0, 0x3, 0x2, 0x2, 0x2, 0xf0, 
    0xf2, 0x3, 0x2, 0x2, 0x2, 0xf1, 0xef, 0x3, 0x2, 0x2, 0x2, 0xf2, 0xf3, 
    0x7, 0x49, 0x2, 0x2, 0xf3, 0xf, 0x3, 0x2, 0x2, 0x2, 0xf4, 0xf5, 0x7, 
    0x26, 0x2, 0x2, 0xf5, 0xf7, 0x5, 0x54, 0x2b, 0x2, 0xf6, 0xf8, 0x5, 0x14, 
    0xb, 0x2, 0xf7, 0xf6, 0x3, 0x2, 0x2, 0x2, 0xf7, 0xf8, 0x3, 0x2, 0x2, 
    0x2, 0xf8, 0xf9, 0x3, 0x2, 0x2, 0x2, 0xf9, 0xfd, 0x7, 0x48, 0x2, 0x2, 
    0xfa, 0xfc, 0x5, 0x18, 0xd, 0x2, 0xfb, 0xfa, 0x3, 0x2, 0x2, 0x2, 0xfc, 
    0xff, 0x3, 0x2, 0x2, 0x2, 0xfd, 0xfb, 0x3, 0x2, 0x2, 0x2, 0xfd, 0xfe, 
    0x3, 0x2, 0x2, 0x2, 0xfe, 0x100, 0x3, 0x2, 0x2, 0x2, 0xff, 0xfd, 0x3, 
    0x2, 0x2, 0x2, 0x100, 0x101, 0x7, 0x49, 0x2, 0x2, 0x101, 0x11, 0x3, 
    0x2, 0x2, 0x2, 0x102, 0x103, 0x7, 0x29, 0x2, 0x2, 0x103, 0x104, 0x5, 
    0x54, 0x2b, 0x2, 0x104, 0x108, 0x7, 0x48, 0x2, 0x2, 0x105, 0x107, 0x5, 
    0x18, 0xd, 0x2, 0x106, 0x105, 0x3, 0x2, 0x2, 0x2, 0x107, 0x10a, 0x3, 
    0x2, 0x2, 0x2, 0x108, 0x106, 0x3, 0x2, 0x2, 0x2, 0x108, 0x109, 0x3, 
    0x2, 0x2, 0x2, 0x109, 0x10b, 0x3, 0x2, 0x2, 0x2, 0x10a, 0x108, 0x3, 
    0x2, 0x2, 0x2, 0x10b, 0x10c, 0x7, 0x49, 0x2, 0x2, 0x10c, 0x13, 0x3, 
    0x2, 0x2, 0x2, 0x10d, 0x10e, 0x7, 0x28, 0x2, 0x2, 0x10e, 0x113, 0x5, 
    0x16, 0xc, 0x2, 0x10f, 0x110, 0x7, 0x5b, 0x2, 0x2, 0x110, 0x112, 0x5, 
    0x16, 0xc, 0x2, 0x111, 0x10f, 0x3, 0x2, 0x2, 0x2, 0x112, 0x115, 0x3, 
    0x2, 0x2, 0x2, 0x113, 0x114, 0x3, 0x2, 0x2, 0x2, 0x113, 0x111, 0x3, 
    0x2, 0x2, 0x2, 0x114, 0x15, 0x3, 0x2, 0x2, 0x2, 0x115, 0x113, 0x3, 0x2, 
    0x2, 0x2, 0x116, 0x118, 0x5, 0x1e, 0x10, 0x2, 0x117, 0x119, 0x5, 0x1c, 
    0xf, 0x2, 0x118, 0x117, 0x3, 0x2, 0x2, 0x2, 0x118, 0x119, 0x3, 0x2, 
    0x2, 0x2, 0x119, 0x17, 0x3, 0x2, 0x2, 0x2, 0x11a, 0x124, 0x5, 0x28, 
    0x15, 0x2, 0x11b, 0x124, 0x5, 0x2e, 0x18, 0x2, 0x11c, 0x124, 0x5, 0x30, 
    0x19, 0x2, 0x11d, 0x124, 0x5, 0x32, 0x1a, 0x2, 0x11e, 0x124, 0x5, 0x34, 
    0x1b, 0x2, 0x11f, 0x124, 0x5, 0x38, 0x1d, 0x2, 0x120, 0x124, 0x5, 0x3a, 
    0x1e, 0x2, 0x121, 0x124, 0x5, 0x3e, 0x20, 0x2, 0x122, 0x124, 0x5, 0x40, 
    0x21, 0x2, 0x123, 0x11a, 0x3, 0x2, 0x2, 0x2, 0x123, 0x11b, 0x3, 0x2, 
    0x2, 0x2, 0x123, 0x11c, 0x3, 0x2, 0x2, 0x2, 0x123, 0x11d, 0x3, 0x2, 
    0x2, 0x2, 0x123, 0x11e, 0x3, 0x2, 0x2, 0x2, 0x123, 0x11f, 0x3, 0x2, 
    0x2, 0x2, 0x123, 0x120, 0x3, 0x2, 0x2, 0x2, 0x123, 0x121, 0x3, 0x2, 
    0x2, 0x2, 0x123, 0x122, 0x3, 0x2, 0x2, 0x2, 0x124, 0x19, 0x3, 0x2, 0x2, 
    0x2, 0x125, 0x126, 0x5, 0x54, 0x2b, 0x2, 0x126, 0x127, 0x7, 0x4a, 0x2, 
    0x2, 0x127, 0x128, 0x5, 0x4c, 0x27, 0x2, 0x128, 0x1b, 0x3, 0x2, 0x2, 
    0x2, 0x129, 0x140, 0x7, 0x44, 0x2, 0x2, 0x12a, 0x12f, 0x5, 0x4c, 0x27, 
    0x2, 0x12b, 0x12c, 0x7, 0x5b, 0x2, 0x2, 0x12c, 0x12e, 0x5, 0x4c, 0x27, 
    0x2, 0x12d, 0x12b, 0x3, 0x2, 0x2, 0x2, 0x12e, 0x131, 0x3, 0x2, 0x2, 
    0x2, 0x12f, 0x12d, 0x3, 0x2, 0x2, 0x2, 0x12f, 0x130, 0x3, 0x2, 0x2, 
    0x2, 0x130, 0x133, 0x3, 0x2, 0x2, 0x2, 0x131, 0x12f, 0x3, 0x2, 0x2, 
    0x2, 0x132, 0x12a, 0x3, 0x2, 0x2, 0x2, 0x132, 0x133, 0x3, 0x2, 0x2, 
    0x2, 0x133, 0x141, 0x3, 0x2, 0x2, 0x2, 0x134, 0x13d, 0x7, 0x48, 0x2, 
    0x2, 0x135, 0x13a, 0x5, 0x1a, 0xe, 0x2, 0x136, 0x137, 0x7, 0x5b, 0x2, 
    0x2, 0x137, 0x139, 0x5, 0x1a, 0xe, 0x2, 0x138, 0x136, 0x3, 0x2, 0x2, 
    0x2, 0x139, 0x13c, 0x3, 0x2, 0x2, 0x2, 0x13a, 0x138, 0x3, 0x2, 0x2, 
    0x2, 0x13a, 0x13b, 0x3, 0x2, 0x2, 0x2, 0x13b, 0x13e, 0x3, 0x2, 0x2, 
    0x2, 0x13c, 0x13a, 0x3, 0x2, 0x2, 0x2, 0x13d, 0x135, 0x3, 0x2, 0x2, 
    0x2, 0x13d, 0x13e, 0x3, 0x2, 0x2, 0x2, 0x13e, 0x13f, 0x3, 0x2, 0x2, 
    0x2, 0x13f, 0x141, 0x7, 0x49, 0x2, 0x2, 0x140, 0x132, 0x3, 0x2, 0x2, 
    0x2, 0x140, 0x134, 0x3, 0x2, 0x2, 0x2, 0x141, 0x142, 0x3, 0x2, 0x2, 
    0x2, 0x142, 0x143, 0x7, 0x45, 0x2, 0x2, 0x143, 0x1d, 0x3, 0x2, 0x2, 
    0x2, 0x144, 0x149, 0x5, 0x54, 0x2b, 0x2, 0x145, 0x146, 0x7, 0x4c, 0x2, 
    0x2, 0x146, 0x148, 0x5, 0x54, 0x2b, 0x2, 0x147, 0x145, 0x3, 0x2, 0x2, 
    0x2, 0x148, 0x14b, 0x3, 0x2, 0x2, 0x2, 0x149, 0x147, 0x3, 0x2, 0x2, 
    0x2, 0x149, 0x14a, 0x3, 0x2, 0x2, 0x2, 0x14a, 0x1f, 0x3, 0x2, 0x2, 0x2, 
    0x14b, 0x149, 0x3, 0x2, 0x2, 0x2, 0x14c, 0x14e, 0x5, 0x54, 0x2b, 0x2, 
    0x14d, 0x14f, 0x5, 0x1c, 0xf, 0x2, 0x14e, 0x14d, 0x3, 0x2, 0x2, 0x2, 
    0x14e, 0x14f, 0x3, 0x2, 0x2, 0x2, 0x14f, 0x21, 0x3, 0x2, 0x2, 0x2, 0x150, 
    0x151, 0x9, 0x2, 0x2, 0x2, 0x151, 0x23, 0x3, 0x2, 0x2, 0x2, 0x152, 0x157, 
    0x5, 0x26, 0x14, 0x2, 0x153, 0x154, 0x7, 0x5b, 0x2, 0x2, 0x154, 0x156, 
    0x5, 0x26, 0x14, 0x2, 0x155, 0x153, 0x3, 0x2, 0x2, 0x2, 0x156, 0x159, 
    0x3, 0x2, 0x2, 0x2, 0x157, 0x155, 0x3, 0x2, 0x2, 0x2, 0x157, 0x158, 
    0x3, 0x2, 0x2, 0x2, 0x158, 0x25, 0x3, 0x2, 0x2, 0x2, 0x159, 0x157, 0x3, 
    0x2, 0x2, 0x2, 0x15a, 0x15c, 0x5, 0x42, 0x22, 0x2, 0x15b, 0x15d, 0x5, 
    0x4a, 0x26, 0x2, 0x15c, 0x15b, 0x3, 0x2, 0x2, 0x2, 0x15c, 0x15d, 0x3, 
    0x2, 0x2, 0x2, 0x15d, 0x15f, 0x3, 0x2, 0x2, 0x2, 0x15e, 0x160, 0x5, 
    0x54, 0x2b, 0x2, 0x15f, 0x15e, 0x3, 0x2, 0x2, 0x2, 0x15f, 0x160, 0x3, 
    0x2, 0x2, 0x2, 0x160, 0x27, 0x3, 0x2, 0x2, 0x2, 0x161, 0x162, 0x7, 0x10, 
    0x2, 0x2, 0x162, 0x164, 0x7, 0x44, 0x2, 0x2, 0x163, 0x165, 0x5, 0x24, 
    0x13, 0x2, 0x164, 0x163, 0x3, 0x2, 0x2, 0x2, 0x164, 0x165, 0x3, 0x2, 
    0x2, 0x2, 0x165, 0x166, 0x3, 0x2, 0x2, 0x2, 0x166, 0x173, 0x7, 0x45, 
    0x2, 0x2, 0x167, 0x172, 0x5, 0x20, 0x11, 0x2, 0x168, 0x169, 0x6, 0x15, 
    0x2, 0x3, 0x169, 0x16a, 0x7, 0x30, 0x2, 0x2, 0x16a, 0x172, 0x8, 0x15, 
    0x1, 0x2, 0x16b, 0x16c, 0x6, 0x15, 0x3, 0x3, 0x16c, 0x16d, 0x7, 0x27, 
    0x2, 0x2, 0x16d, 0x172, 0x8, 0x15, 0x1, 0x2, 0x16e, 0x16f, 0x6, 0x15, 
    0x4, 0x3, 0x16f, 0x170, 0x7, 0x32, 0x2, 0x2, 0x170, 0x172, 0x8, 0x15, 
    0x1, 0x2, 0x171, 0x167, 0x3, 0x2, 0x2, 0x2, 0x171, 0x168, 0x3, 0x2, 
    0x2, 0x2, 0x171, 0x16b, 0x3, 0x2, 0x2, 0x2, 0x171, 0x16e, 0x3, 0x2, 
    0x2, 0x2, 0x172, 0x175, 0x3, 0x2, 0x2, 0x2, 0x173, 0x171, 0x3, 0x2, 
    0x2, 0x2, 0x173, 0x174, 0x3, 0x2, 0x2, 0x2, 0x174, 0x176, 0x3, 0x2, 
    0x2, 0x2, 0x175, 0x173, 0x3, 0x2, 0x2, 0x2, 0x176, 0x177, 0x5, 0x62, 
    0x32, 0x2, 0x177, 0x29, 0x3, 0x2, 0x2, 0x2, 0x178, 0x179, 0x9, 0x3, 
    0x2, 0x2, 0x179, 0x2b, 0x3, 0x2, 0x2, 0x2, 0x17a, 0x186, 0x7, 0x2f, 
    0x2, 0x2, 0x17b, 0x17c, 0x7, 0x44, 0x2, 0x2, 0x17c, 0x181, 0x5, 0x1e, 
    0x10, 0x2, 0x17d, 0x17e, 0x7, 0x5b, 0x2, 0x2, 0x17e, 0x180, 0x5, 0x1e, 
    0x10, 0x2, 0x17f, 0x17d, 0x3, 0x2, 0x2, 0x2, 0x180, 0x183, 0x3, 0x2, 
    0x2, 0x2, 0x181, 0x17f, 0x3, 0x2, 0x2, 0x2, 0x181, 0x182, 0x3, 0x2, 
    0x2, 0x2, 0x182, 0x184, 0x3, 0x2, 0x2, 0x2, 0x183, 0x181, 0x3, 0x2, 
    0x2, 0x2, 0x184, 0x185, 0x7, 0x45, 0x2, 0x2, 0x185, 0x187, 0x3, 0x2, 
    0x2, 0x2, 0x186, 0x17b, 0x3, 0x2, 0x2, 0x2, 0x186, 0x187, 0x3, 0x2, 
    0x2, 0x2, 0x187, 0x2d, 0x3, 0x2, 0x2, 0x2, 0x188, 0x18c, 0x7, 0x20, 
    0x2, 0x2, 0x189, 0x18d, 0x5, 0x54, 0x2b, 0x2, 0x18a, 0x18d, 0x7, 0x1a, 
    0x2, 0x2, 0x18b, 0x18d, 0x7, 0x34, 0x2, 0x2, 0x18c, 0x189, 0x3, 0x2, 
    0x2, 0x2, 0x18c, 0x18a, 0x3, 0x2, 0x2, 0x2, 0x18c, 0x18b, 0x3, 0x2, 
    0x2, 0x2, 0x18d, 0x18e, 0x3, 0x2, 0x2, 0x2, 0x18e, 0x190, 0x7, 0x44, 
    0x2, 0x2, 0x18f, 0x191, 0x5, 0x24, 0x13, 0x2, 0x190, 0x18f, 0x3, 0x2, 
    0x2, 0x2, 0x190, 0x191, 0x3, 0x2, 0x2, 0x2, 0x191, 0x192, 0x3, 0x2, 
    0x2, 0x2, 0x192, 0x1a5, 0x7, 0x45, 0x2, 0x2, 0x193, 0x194, 0x6, 0x18, 
    0x5, 0x3, 0x194, 0x195, 0x5, 0x22, 0x12, 0x2, 0x195, 0x196, 0x8, 0x18, 
    0x1, 0x2, 0x196, 0x1a4, 0x3, 0x2, 0x2, 0x2, 0x197, 0x198, 0x6, 0x18, 
    0x6, 0x3, 0x198, 0x199, 0x5, 0x2a, 0x16, 0x2, 0x199, 0x19a, 0x8, 0x18, 
    0x1, 0x2, 0x19a, 0x1a4, 0x3, 0x2, 0x2, 0x2, 0x19b, 0x1a4, 0x5, 0x20, 
    0x11, 0x2, 0x19c, 0x19d, 0x6, 0x18, 0x7, 0x3, 0x19d, 0x19e, 0x7, 0x42, 
    0x2, 0x2, 0x19e, 0x1a4, 0x8, 0x18, 0x1, 0x2, 0x19f, 0x1a0, 0x6, 0x18, 
    0x8, 0x3, 0x1a0, 0x1a1, 0x5, 0x2c, 0x17, 0x2, 0x1a1, 0x1a2, 0x8, 0x18, 
    0x1, 0x2, 0x1a2, 0x1a4, 0x3, 0x2, 0x2, 0x2, 0x1a3, 0x193, 0x3, 0x2, 
    0x2, 0x2, 0x1a3, 0x197, 0x3, 0x2, 0x2, 0x2, 0x1a3, 0x19b, 0x3, 0x2, 
    0x2, 0x2, 0x1a3, 0x19c, 0x3, 0x2, 0x2, 0x2, 0x1a3, 0x19f, 0x3, 0x2, 
    0x2, 0x2, 0x1a4, 0x1a7, 0x3, 0x2, 0x2, 0x2, 0x1a5, 0x1a3, 0x3, 0x2, 
    0x2, 0x2, 0x1a5, 0x1a6, 0x3, 0x2, 0x2, 0x2, 0x1a6, 0x1ad, 0x3, 0x2, 
    0x2, 0x2, 0x1a7, 0x1a5, 0x3, 0x2, 0x2, 0x2, 0x1a8, 0x1a9, 0x7, 0x36, 
    0x2, 0x2, 0x1a9, 0x1aa, 0x7, 0x44, 0x2, 0x2, 0x1aa, 0x1ab, 0x5, 0x24, 
    0x13, 0x2, 0x1ab, 0x1ac, 0x7, 0x45, 0x2, 0x2, 0x1ac, 0x1ae, 0x3, 0x2, 
    0x2, 0x2, 0x1ad, 0x1a8, 0x3, 0x2, 0x2, 0x2, 0x1ad, 0x1ae, 0x3, 0x2, 
    0x2, 0x2, 0x1ae, 0x1b1, 0x3, 0x2, 0x2, 0x2, 0x1af, 0x1b2, 0x7, 0x4b, 
    0x2, 0x2, 0x1b0, 0x1b2, 0x5, 0x62, 0x32, 0x2, 0x1b1, 0x1af, 0x3, 0x2, 
    0x2, 0x2, 0x1b1, 0x1b0, 0x3, 0x2, 0x2, 0x2, 0x1b2, 0x2f, 0x3, 0x2, 0x2, 
    0x2, 0x1b3, 0x1b4, 0x7, 0x2c, 0x2, 0x2, 0x1b4, 0x1ba, 0x5, 0x54, 0x2b, 
    0x2, 0x1b5, 0x1b7, 0x7, 0x44, 0x2, 0x2, 0x1b6, 0x1b8, 0x5, 0x24, 0x13, 
    0x2, 0x1b7, 0x1b6, 0x3, 0x2, 0x2, 0x2, 0x1b7, 0x1b8, 0x3, 0x2, 0x2, 
    0x2, 0x1b8, 0x1b9, 0x3, 0x2, 0x2, 0x2, 0x1b9, 0x1bb, 0x7, 0x45, 0x2, 
    0x2, 0x1ba, 0x1b5, 0x3, 0x2, 0x2, 0x2, 0x1ba, 0x1bb, 0x3, 0x2, 0x2, 
    0x2, 0x1bb, 0x1c5, 0x3, 0x2, 0x2, 0x2, 0x1bc, 0x1bd, 0x6, 0x19, 0x9, 
    0x3, 0x1bd, 0x1be, 0x7, 0x42, 0x2, 0x2, 0x1be, 0x1c4, 0x8, 0x19, 0x1, 
    0x2, 0x1bf, 0x1c0, 0x6, 0x19, 0xa, 0x3, 0x1c0, 0x1c1, 0x5, 0x2c, 0x17, 
    0x2, 0x1c1, 0x1c2, 0x8, 0x19, 0x1, 0x2, 0x1c2, 0x1c4, 0x3, 0x2, 0x2, 
    0x2, 0x1c3, 0x1bc, 0x3, 0x2, 0x2, 0x2, 0x1c3, 0x1bf, 0x3, 0x2, 0x2, 
    0x2, 0x1c4, 0x1c7, 0x3, 0x2, 0x2, 0x2, 0x1c5, 0x1c3, 0x3, 0x2, 0x2, 
    0x2, 0x1c5, 0x1c6, 0x3, 0x2, 0x2, 0x2, 0x1c6, 0x1ca, 0x3, 0x2, 0x2, 
    0x2, 0x1c7, 0x1c5, 0x3, 0x2, 0x2, 0x2, 0x1c8, 0x1cb, 0x7, 0x4b, 0x2, 
    0x2, 0x1c9, 0x1cb, 0x5, 0x62, 0x32, 0x2, 0x1ca, 0x1c8, 0x3, 0x2, 0x2, 
    0x2, 0x1ca, 0x1c9, 0x3, 0x2, 0x2, 0x2, 0x1cb, 0x31, 0x3, 0x2, 0x2, 0x2, 
    0x1cc, 0x1cd, 0x9, 0x4, 0x2, 0x2, 0x1cd, 0x1ce, 0x7, 0x44, 0x2, 0x2, 
    0x1ce, 0x1e1, 0x7, 0x45, 0x2, 0x2, 0x1cf, 0x1d0, 0x6, 0x1a, 0xb, 0x3, 
    0x1d0, 0x1d1, 0x5, 0x22, 0x12, 0x2, 0x1d1, 0x1d2, 0x8, 0x1a, 0x1, 0x2, 
    0x1d2, 0x1e0, 0x3, 0x2, 0x2, 0x2, 0x1d3, 0x1d4, 0x6, 0x1a, 0xc, 0x3, 
    0x1d4, 0x1d5, 0x5, 0x2a, 0x16, 0x2, 0x1d5, 0x1d6, 0x8, 0x1a, 0x1, 0x2, 
    0x1d6, 0x1e0, 0x3, 0x2, 0x2, 0x2, 0x1d7, 0x1e0, 0x5, 0x20, 0x11, 0x2, 
    0x1d8, 0x1d9, 0x6, 0x1a, 0xd, 0x3, 0x1d9, 0x1da, 0x7, 0x42, 0x2, 0x2, 
    0x1da, 0x1e0, 0x8, 0x1a, 0x1, 0x2, 0x1db, 0x1dc, 0x6, 0x1a, 0xe, 0x3, 
    0x1dc, 0x1dd, 0x5, 0x2c, 0x17, 0x2, 0x1dd, 0x1de, 0x8, 0x1a, 0x1, 0x2, 
    0x1de, 0x1e0, 0x3, 0x2, 0x2, 0x2, 0x1df, 0x1cf, 0x3, 0x2, 0x2, 0x2, 
    0x1df, 0x1d3, 0x3, 0x2, 0x2, 0x2, 0x1df, 0x1d7, 0x3, 0x2, 0x2, 0x2, 
    0x1df, 0x1d8, 0x3, 0x2, 0x2, 0x2, 0x1df, 0x1db, 0x3, 0x2, 0x2, 0x2, 
    0x1e0, 0x1e3, 0x3, 0x2, 0x2, 0x2, 0x1e1, 0x1df, 0x3, 0x2, 0x2, 0x2, 
    0x1e1, 0x1e2, 0x3, 0x2, 0x2, 0x2, 0x1e2, 0x1e6, 0x3, 0x2, 0x2, 0x2, 
    0x1e3, 0x1e1, 0x3, 0x2, 0x2, 0x2, 0x1e4, 0x1e7, 0x7, 0x4b, 0x2, 0x2, 
    0x1e5, 0x1e7, 0x5, 0x62, 0x32, 0x2, 0x1e6, 0x1e4, 0x3, 0x2, 0x2, 0x2, 
    0x1e6, 0x1e5, 0x3, 0x2, 0x2, 0x2, 0x1e7, 0x33, 0x3, 0x2, 0x2, 0x2, 0x1e8, 
    0x1e9, 0x7, 0x3a, 0x2, 0x2, 0x1e9, 0x1ea, 0x5, 0x54, 0x2b, 0x2, 0x1ea, 
    0x1ec, 0x7, 0x48, 0x2, 0x2, 0x1eb, 0x1ed, 0x5, 0x36, 0x1c, 0x2, 0x1ec, 
    0x1eb, 0x3, 0x2, 0x2, 0x2, 0x1ed, 0x1ee, 0x3, 0x2, 0x2, 0x2, 0x1ee, 
    0x1ec, 0x3, 0x2, 0x2, 0x2, 0x1ee, 0x1ef, 0x3, 0x2, 0x2, 0x2, 0x1ef, 
    0x1f0, 0x3, 0x2, 0x2, 0x2, 0x1f0, 0x1f1, 0x7, 0x49, 0x2, 0x2, 0x1f1, 
    0x35, 0x3, 0x2, 0x2, 0x2, 0x1f2, 0x1f3, 0x5, 0x42, 0x22, 0x2, 0x1f3, 
    0x1f4, 0x5, 0x54, 0x2b, 0x2, 0x1f4, 0x1f5, 0x7, 0x4b, 0x2, 0x2, 0x1f5, 
    0x37, 0x3, 0x2, 0x2, 0x2, 0x1f6, 0x1f7, 0x7, 0x17, 0x2, 0x2, 0x1f7, 
    0x1f8, 0x5, 0x54, 0x2b, 0x2, 0x1f8, 0x1f9, 0x7, 0x48, 0x2, 0x2, 0x1f9, 
    0x1fe, 0x5, 0x54, 0x2b, 0x2, 0x1fa, 0x1fb, 0x7, 0x5b, 0x2, 0x2, 0x1fb, 
    0x1fd, 0x5, 0x54, 0x2b, 0x2, 0x1fc, 0x1fa, 0x3, 0x2, 0x2, 0x2, 0x1fd, 
    0x200, 0x3, 0x2, 0x2, 0x2, 0x1fe, 0x1fc, 0x3, 0x2, 0x2, 0x2, 0x1fe, 
    0x1ff, 0x3, 0x2, 0x2, 0x2, 0x1ff, 0x201, 0x3, 0x2, 0x2, 0x2, 0x200, 
    0x1fe, 0x3, 0x2, 0x2, 0x2, 0x201, 0x202, 0x7, 0x49, 0x2, 0x2, 0x202, 
    0x39, 0x3, 0x2, 0x2, 0x2, 0x203, 0x219, 0x5, 0x42, 0x22, 0x2, 0x204, 
    0x205, 0x6, 0x1e, 0xf, 0x3, 0x205, 0x206, 0x7, 0x32, 0x2, 0x2, 0x206, 
    0x218, 0x8, 0x1e, 0x1, 0x2, 0x207, 0x208, 0x6, 0x1e, 0x10, 0x3, 0x208, 
    0x209, 0x7, 0x31, 0x2, 0x2, 0x209, 0x218, 0x8, 0x1e, 0x1, 0x2, 0x20a, 
    0x20b, 0x6, 0x1e, 0x11, 0x3, 0x20b, 0x20c, 0x7, 0x27, 0x2, 0x2, 0x20c, 
    0x218, 0x8, 0x1e, 0x1, 0x2, 0x20d, 0x20e, 0x6, 0x1e, 0x12, 0x3, 0x20e, 
    0x20f, 0x7, 0xf, 0x2, 0x2, 0x20f, 0x218, 0x8, 0x1e, 0x1, 0x2, 0x210, 
    0x211, 0x6, 0x1e, 0x13, 0x3, 0x211, 0x212, 0x5, 0x2c, 0x17, 0x2, 0x212, 
    0x213, 0x8, 0x1e, 0x1, 0x2, 0x213, 0x218, 0x3, 0x2, 0x2, 0x2, 0x214, 
    0x215, 0x6, 0x1e, 0x14, 0x3, 0x215, 0x216, 0x7, 0x23, 0x2, 0x2, 0x216, 
    0x218, 0x8, 0x1e, 0x1, 0x2, 0x217, 0x204, 0x3, 0x2, 0x2, 0x2, 0x217, 
    0x207, 0x3, 0x2, 0x2, 0x2, 0x217, 0x20a, 0x3, 0x2, 0x2, 0x2, 0x217, 
    0x20d, 0x3, 0x2, 0x2, 0x2, 0x217, 0x210, 0x3, 0x2, 0x2, 0x2, 0x217, 
    0x214, 0x3, 0x2, 0x2, 0x2, 0x218, 0x21b, 0x3, 0x2, 0x2, 0x2, 0x219, 
    0x217, 0x3, 0x2, 0x2, 0x2, 0x219, 0x21a, 0x3, 0x2, 0x2, 0x2, 0x21a, 
    0x21c, 0x3, 0x2, 0x2, 0x2, 0x21b, 0x219, 0x3, 0x2, 0x2, 0x2, 0x21c, 
    0x21f, 0x5, 0x54, 0x2b, 0x2, 0x21d, 0x21e, 0x7, 0x4f, 0x2, 0x2, 0x21e, 
    0x220, 0x5, 0x4c, 0x27, 0x2, 0x21f, 0x21d, 0x3, 0x2, 0x2, 0x2, 0x21f, 
    0x220, 0x3, 0x2, 0x2, 0x2, 0x220, 0x221, 0x3, 0x2, 0x2, 0x2, 0x221, 
    0x222, 0x7, 0x4b, 0x2, 0x2, 0x222, 0x3b, 0x3, 0x2, 0x2, 0x2, 0x223, 
    0x225, 0x5, 0x42, 0x22, 0x2, 0x224, 0x226, 0x7, 0x25, 0x2, 0x2, 0x225, 
    0x224, 0x3, 0x2, 0x2, 0x2, 0x225, 0x226, 0x3, 0x2, 0x2, 0x2, 0x226, 
    0x228, 0x3, 0x2, 0x2, 0x2, 0x227, 0x229, 0x5, 0x54, 0x2b, 0x2, 0x228, 
    0x227, 0x3, 0x2, 0x2, 0x2, 0x228, 0x229, 0x3, 0x2, 0x2, 0x2, 0x229, 
    0x3d, 0x3, 0x2, 0x2, 0x2, 0x22a, 0x22b, 0x7, 0x18, 0x2, 0x2, 0x22b, 
    0x22c, 0x5, 0x54, 0x2b, 0x2, 0x22c, 0x235, 0x7, 0x44, 0x2, 0x2, 0x22d, 
    0x232, 0x5, 0x3c, 0x1f, 0x2, 0x22e, 0x22f, 0x7, 0x5b, 0x2, 0x2, 0x22f, 
    0x231, 0x5, 0x3c, 0x1f, 0x2, 0x230, 0x22e, 0x3, 0x2, 0x2, 0x2, 0x231, 
    0x234, 0x3, 0x2, 0x2, 0x2, 0x232, 0x230, 0x3, 0x2, 0x2, 0x2, 0x232, 
    0x233, 0x3, 0x2, 0x2, 0x2, 0x233, 0x236, 0x3, 0x2, 0x2, 0x2, 0x234, 
    0x232, 0x3, 0x2, 0x2, 0x2, 0x235, 0x22d, 0x3, 0x2, 0x2, 0x2, 0x235, 
    0x236, 0x3, 0x2, 0x2, 0x2, 0x236, 0x237, 0x3, 0x2, 0x2, 0x2, 0x237, 
    0x239, 0x7, 0x45, 0x2, 0x2, 0x238, 0x23a, 0x7, 0x6, 0x2, 0x2, 0x239, 
    0x238, 0x3, 0x2, 0x2, 0x2, 0x239, 0x23a, 0x3, 0x2, 0x2, 0x2, 0x23a, 
    0x23b, 0x3, 0x2, 0x2, 0x2, 0x23b, 0x23c, 0x7, 0x4b, 0x2, 0x2, 0x23c, 
    0x3f, 0x3, 0x2, 0x2, 0x2, 0x23d, 0x23e, 0x7, 0x40, 0x2, 0x2, 0x23e, 
    0x23f, 0x5, 0x1e, 0x10, 0x2, 0x23f, 0x242, 0x7, 0x1f, 0x2, 0x2, 0x240, 
    0x243, 0x7, 0x66, 0x2, 0x2, 0x241, 0x243, 0x5, 0x42, 0x22, 0x2, 0x242, 
    0x240, 0x3, 0x2, 0x2, 0x2, 0x242, 0x241, 0x3, 0x2, 0x2, 0x2, 0x243, 
    0x244, 0x3, 0x2, 0x2, 0x2, 0x244, 0x245, 0x7, 0x4b, 0x2, 0x2, 0x245, 
    0x41, 0x3, 0x2, 0x2, 0x2, 0x246, 0x247, 0x8, 0x22, 0x1, 0x2, 0x247, 
    0x24c, 0x5, 0x44, 0x23, 0x2, 0x248, 0x24c, 0x5, 0x46, 0x24, 0x2, 0x249, 
    0x24c, 0x5, 0x86, 0x44, 0x2, 0x24a, 0x24c, 0x5, 0x1e, 0x10, 0x2, 0x24b, 
    0x246, 0x3, 0x2, 0x2, 0x2, 0x24b, 0x248, 0x3, 0x2, 0x2, 0x2, 0x24b, 
    0x249, 0x3, 0x2, 0x2, 0x2, 0x24b, 0x24a, 0x3, 0x2, 0x2, 0x2, 0x24c, 
    0x255, 0x3, 0x2, 0x2, 0x2, 0x24d, 0x24e, 0xc, 0x3, 0x2, 0x2, 0x24e, 
    0x250, 0x7, 0x46, 0x2, 0x2, 0x24f, 0x251, 0x5, 0x4c, 0x27, 0x2, 0x250, 
    0x24f, 0x3, 0x2, 0x2, 0x2, 0x250, 0x251, 0x3, 0x2, 0x2, 0x2, 0x251, 
    0x252, 0x3, 0x2, 0x2, 0x2, 0x252, 0x254, 0x7, 0x47, 0x2, 0x2, 0x253, 
    0x24d, 0x3, 0x2, 0x2, 0x2, 0x254, 0x257, 0x3, 0x2, 0x2, 0x2, 0x255, 
    0x253, 0x3, 0x2, 0x2, 0x2, 0x255, 0x256, 0x3, 0x2, 0x2, 0x2, 0x256, 
    0x43, 0x3, 0x2, 0x2, 0x2, 0x257, 0x255, 0x3, 0x2, 0x2, 0x2, 0x258, 0x265, 
    0x7, 0x7, 0x2, 0x2, 0x259, 0x25a, 0x6, 0x23, 0x16, 0x3, 0x25a, 0x25b, 
    0x7, 0x7, 0x2, 0x2, 0x25b, 0x265, 0x7, 0x30, 0x2, 0x2, 0x25c, 0x265, 
    0x7, 0xa, 0x2, 0x2, 0x25d, 0x265, 0x7, 0x39, 0x2, 0x2, 0x25e, 0x265, 
    0x7, 0xc, 0x2, 0x2, 0x25f, 0x265, 0x7, 0x37, 0x2, 0x2, 0x260, 0x265, 
    0x7, 0x3f, 0x2, 0x2, 0x261, 0x265, 0x7, 0x1e, 0x2, 0x2, 0x262, 0x265, 
    0x7, 0x1c, 0x2, 0x2, 0x263, 0x265, 0x7, 0x3e, 0x2, 0x2, 0x264, 0x258, 
    0x3, 0x2, 0x2, 0x2, 0x264, 0x259, 0x3, 0x2, 0x2, 0x2, 0x264, 0x25c, 
    0x3, 0x2, 0x2, 0x2, 0x264, 0x25d, 0x3, 0x2, 0x2, 0x2, 0x264, 0x25e, 
    0x3, 0x2, 0x2, 0x2, 0x264, 0x25f, 0x3, 0x2, 0x2, 0x2, 0x264, 0x260, 
    0x3, 0x2, 0x2, 0x2, 0x264, 0x261, 0x3, 0x2, 0x2, 0x2, 0x264, 0x262, 
    0x3, 0x2, 0x2, 0x2, 0x264, 0x263, 0x3, 0x2, 0x2, 0x2, 0x265, 0x45, 0x3, 
    0x2, 0x2, 0x2, 0x266, 0x267, 0x7, 0x20, 0x2, 0x2, 0x267, 0x269, 0x7, 
    0x44, 0x2, 0x2, 0x268, 0x26a, 0x5, 0x24, 0x13, 0x2, 0x269, 0x268, 0x3, 
    0x2, 0x2, 0x2, 0x269, 0x26a, 0x3, 0x2, 0x2, 0x2, 0x26a, 0x26b, 0x3, 
    0x2, 0x2, 0x2, 0x26b, 0x276, 0x7, 0x45, 0x2, 0x2, 0x26c, 0x26d, 0x6, 
    0x24, 0x17, 0x3, 0x26d, 0x26e, 0x5, 0x22, 0x12, 0x2, 0x26e, 0x26f, 0x8, 
    0x24, 0x1, 0x2, 0x26f, 0x275, 0x3, 0x2, 0x2, 0x2, 0x270, 0x271, 0x6, 
    0x24, 0x18, 0x3, 0x271, 0x272, 0x5, 0x2a, 0x16, 0x2, 0x272, 0x273, 0x8, 
    0x24, 0x1, 0x2, 0x273, 0x275, 0x3, 0x2, 0x2, 0x2, 0x274, 0x26c, 0x3, 
    0x2, 0x2, 0x2, 0x274, 0x270, 0x3, 0x2, 0x2, 0x2, 0x275, 0x278, 0x3, 
    0x2, 0x2, 0x2, 0x276, 0x274, 0x3, 0x2, 0x2, 0x2, 0x276, 0x277, 0x3, 
    0x2, 0x2, 0x2, 0x277, 0x27e, 0x3, 0x2, 0x2, 0x2, 0x278, 0x276, 0x3, 
    0x2, 0x2, 0x2, 0x279, 0x27a, 0x7, 0x36, 0x2, 0x2, 0x27a, 0x27b, 0x7, 
    0x44, 0x2, 0x2, 0x27b, 0x27c, 0x5, 0x24, 0x13, 0x2, 0x27c, 0x27d, 0x7, 
    0x45, 0x2, 0x2, 0x27d, 0x27f, 0x3, 0x2, 0x2, 0x2, 0x27e, 0x279, 0x3, 
    0x2, 0x2, 0x2, 0x27e, 0x27f, 0x3, 0x2, 0x2, 0x2, 0x27f, 0x47, 0x3, 0x2, 
    0x2, 0x2, 0x280, 0x282, 0x5, 0x42, 0x22, 0x2, 0x281, 0x283, 0x5, 0x4a, 
    0x26, 0x2, 0x282, 0x281, 0x3, 0x2, 0x2, 0x2, 0x282, 0x283, 0x3, 0x2, 
    0x2, 0x2, 0x283, 0x284, 0x3, 0x2, 0x2, 0x2, 0x284, 0x285, 0x5, 0x54, 
    0x2b, 0x2, 0x285, 0x49, 0x3, 0x2, 0x2, 0x2, 0x286, 0x287, 0x9, 0x5, 
    0x2, 0x2, 0x287, 0x4b, 0x3, 0x2, 0x2, 0x2, 0x288, 0x289, 0x8, 0x27, 
    0x1, 0x2, 0x289, 0x28a, 0x7, 0x30, 0x2, 0x2, 0x28a, 0x29d, 0x5, 0x1c, 
    0xf, 0x2, 0x28b, 0x28c, 0x7, 0x3d, 0x2, 0x2, 0x28c, 0x28d, 0x7, 0x44, 
    0x2, 0x2, 0x28d, 0x28e, 0x5, 0x42, 0x22, 0x2, 0x28e, 0x28f, 0x7, 0x45, 
    0x2, 0x2, 0x28f, 0x29d, 0x3, 0x2, 0x2, 0x2, 0x290, 0x291, 0x9, 0x6, 
    0x2, 0x2, 0x291, 0x29d, 0x5, 0x4c, 0x27, 0x15, 0x292, 0x293, 0x7, 0x2d, 
    0x2, 0x2, 0x293, 0x29d, 0x5, 0x42, 0x22, 0x2, 0x294, 0x29d, 0x5, 0x50, 
    0x29, 0x2, 0x295, 0x29d, 0x5, 0x52, 0x2a, 0x2, 0x296, 0x29b, 0x5, 0x54, 
    0x2b, 0x2, 0x297, 0x29b, 0x5, 0x56, 0x2c, 0x2, 0x298, 0x29b, 0x5, 0x44, 
    0x23, 0x2, 0x299, 0x29b, 0x5, 0x1e, 0x10, 0x2, 0x29a, 0x296, 0x3, 0x2, 
    0x2, 0x2, 0x29a, 0x297, 0x3, 0x2, 0x2, 0x2, 0x29a, 0x298, 0x3, 0x2, 
    0x2, 0x2, 0x29a, 0x299, 0x3, 0x2, 0x2, 0x2, 0x29b, 0x29d, 0x3, 0x2, 
    0x2, 0x2, 0x29c, 0x288, 0x3, 0x2, 0x2, 0x2, 0x29c, 0x28b, 0x3, 0x2, 
    0x2, 0x2, 0x29c, 0x290, 0x3, 0x2, 0x2, 0x2, 0x29c, 0x292, 0x3, 0x2, 
    0x2, 0x2, 0x29c, 0x294, 0x3, 0x2, 0x2, 0x2, 0x29c, 0x295, 0x3, 0x2, 
    0x2, 0x2, 0x29c, 0x29a, 0x3, 0x2, 0x2, 0x2, 0x29d, 0x2f2, 0x3, 0x2, 
    0x2, 0x2, 0x29e, 0x29f, 0xc, 0x13, 0x2, 0x2, 0x29f, 0x2a0, 0x7, 0x69, 
    0x2, 0x2, 0x2a0, 0x2f1, 0x5, 0x4c, 0x27, 0x13, 0x2a1, 0x2a2, 0xc, 0x12, 
    0x2, 0x2, 0x2a2, 0x2a3, 0x9, 0x7, 0x2, 0x2, 0x2a3, 0x2f1, 0x5, 0x4c, 
    0x27, 0x13, 0x2a4, 0x2a5, 0xc, 0x11, 0x2, 0x2, 0x2a5, 0x2a6, 0x9, 0x8, 
    0x2, 0x2, 0x2a6, 0x2f1, 0x5, 0x4c, 0x27, 0x12, 0x2a7, 0x2a8, 0xc, 0x10, 
    0x2, 0x2, 0x2a8, 0x2a9, 0x9, 0x9, 0x2, 0x2, 0x2a9, 0x2f1, 0x5, 0x4c, 
    0x27, 0x11, 0x2aa, 0x2ab, 0xc, 0xf, 0x2, 0x2, 0x2ab, 0x2ac, 0x7, 0x60, 
    0x2, 0x2, 0x2ac, 0x2f1, 0x5, 0x4c, 0x27, 0x10, 0x2ad, 0x2ae, 0xc, 0xe, 
    0x2, 0x2, 0x2ae, 0x2af, 0x7, 0x5f, 0x2, 0x2, 0x2af, 0x2f1, 0x5, 0x4c, 
    0x27, 0xf, 0x2b0, 0x2b1, 0xc, 0xd, 0x2, 0x2, 0x2b1, 0x2b2, 0x7, 0x5e, 
    0x2, 0x2, 0x2b2, 0x2f1, 0x5, 0x4c, 0x27, 0xe, 0x2b3, 0x2b4, 0xc, 0xc, 
    0x2, 0x2, 0x2b4, 0x2b5, 0x9, 0xa, 0x2, 0x2, 0x2b5, 0x2f1, 0x5, 0x4c, 
    0x27, 0xd, 0x2b6, 0x2b7, 0xc, 0xb, 0x2, 0x2, 0x2b7, 0x2b8, 0x9, 0xb, 
    0x2, 0x2, 0x2b8, 0x2f1, 0x5, 0x4c, 0x27, 0xc, 0x2b9, 0x2ba, 0xc, 0xa, 
    0x2, 0x2, 0x2ba, 0x2bb, 0x7, 0x5d, 0x2, 0x2, 0x2bb, 0x2f1, 0x5, 0x4c, 
    0x27, 0xb, 0x2bc, 0x2bd, 0xc, 0x9, 0x2, 0x2, 0x2bd, 0x2be, 0x7, 0x5c, 
    0x2, 0x2, 0x2be, 0x2f1, 0x5, 0x4c, 0x27, 0xa, 0x2bf, 0x2c0, 0xc, 0x8, 
    0x2, 0x2, 0x2c0, 0x2c1, 0x7, 0x4d, 0x2, 0x2, 0x2c1, 0x2c2, 0x5, 0x4c, 
    0x27, 0x2, 0x2c2, 0x2c3, 0x7, 0x4a, 0x2, 0x2, 0x2c3, 0x2c4, 0x5, 0x4c, 
    0x27, 0x8, 0x2c4, 0x2f1, 0x3, 0x2, 0x2, 0x2, 0x2c5, 0x2c6, 0xc, 0x7, 
    0x2, 0x2, 0x2c6, 0x2c7, 0x5, 0x4e, 0x28, 0x2, 0x2c7, 0x2c8, 0x5, 0x4c, 
    0x27, 0x7, 0x2c8, 0x2f1, 0x3, 0x2, 0x2, 0x2, 0x2c9, 0x2ca, 0xc, 0x1c, 
    0x2, 0x2, 0x2ca, 0x2cc, 0x7, 0x46, 0x2, 0x2, 0x2cb, 0x2cd, 0x5, 0x4c, 
    0x27, 0x2, 0x2cc, 0x2cb, 0x3, 0x2, 0x2, 0x2, 0x2cc, 0x2cd, 0x3, 0x2, 
    0x2, 0x2, 0x2cd, 0x2ce, 0x3, 0x2, 0x2, 0x2, 0x2ce, 0x2f1, 0x7, 0x47, 
    0x2, 0x2, 0x2cf, 0x2d0, 0xc, 0x1b, 0x2, 0x2, 0x2d0, 0x2d2, 0x7, 0x46, 
    0x2, 0x2, 0x2d1, 0x2d3, 0x5, 0x4c, 0x27, 0x2, 0x2d2, 0x2d1, 0x3, 0x2, 
    0x2, 0x2, 0x2d2, 0x2d3, 0x3, 0x2, 0x2, 0x2, 0x2d3, 0x2d4, 0x3, 0x2, 
    0x2, 0x2, 0x2d4, 0x2d6, 0x7, 0x4a, 0x2, 0x2, 0x2d5, 0x2d7, 0x5, 0x4c, 
    0x27, 0x2, 0x2d6, 0x2d5, 0x3, 0x2, 0x2, 0x2, 0x2d6, 0x2d7, 0x3, 0x2, 
    0x2, 0x2, 0x2d7, 0x2d8, 0x3, 0x2, 0x2, 0x2, 0x2d8, 0x2f1, 0x7, 0x47, 
    0x2, 0x2, 0x2d9, 0x2da, 0xc, 0x1a, 0x2, 0x2, 0x2da, 0x2dd, 0x7, 0x4c, 
    0x2, 0x2, 0x2db, 0x2de, 0x5, 0x54, 0x2b, 0x2, 0x2dc, 0x2de, 0x7, 0x7, 
    0x2, 0x2, 0x2dd, 0x2db, 0x3, 0x2, 0x2, 0x2, 0x2dd, 0x2dc, 0x3, 0x2, 
    0x2, 0x2, 0x2de, 0x2f1, 0x3, 0x2, 0x2, 0x2, 0x2df, 0x2e0, 0xc, 0x19, 
    0x2, 0x2, 0x2e0, 0x2e9, 0x7, 0x48, 0x2, 0x2, 0x2e1, 0x2e6, 0x5, 0x1a, 
    0xe, 0x2, 0x2e2, 0x2e3, 0x7, 0x5b, 0x2, 0x2, 0x2e3, 0x2e5, 0x5, 0x1a, 
    0xe, 0x2, 0x2e4, 0x2e2, 0x3, 0x2, 0x2, 0x2, 0x2e5, 0x2e8, 0x3, 0x2, 
    0x2, 0x2, 0x2e6, 0x2e4, 0x3, 0x2, 0x2, 0x2, 0x2e6, 0x2e7, 0x3, 0x2, 
    0x2, 0x2, 0x2e7, 0x2ea, 0x3, 0x2, 0x2, 0x2, 0x2e8, 0x2e6, 0x3, 0x2, 
    0x2, 0x2, 0x2e9, 0x2e1, 0x3, 0x2, 0x2, 0x2, 0x2e9, 0x2ea, 0x3, 0x2, 
    0x2, 0x2, 0x2ea, 0x2eb, 0x3, 0x2, 0x2, 0x2, 0x2eb, 0x2f1, 0x7, 0x49, 
    0x2, 0x2, 0x2ec, 0x2ed, 0xc, 0x18, 0x2, 0x2, 0x2ed, 0x2f1, 0x5, 0x1c, 
    0xf, 0x2, 0x2ee, 0x2ef, 0xc, 0x14, 0x2, 0x2, 0x2ef, 0x2f1, 0x9, 0xc, 
    0x2, 0x2, 0x2f0, 0x29e, 0x3, 0x2, 0x2, 0x2, 0x2f0, 0x2a1, 0x3, 0x2, 
    0x2, 0x2, 0x2f0, 0x2a4, 0x3, 0x2, 0x2, 0x2, 0x2f0, 0x2a7, 0x3, 0x2, 
    0x2, 0x2, 0x2f0, 0x2aa, 0x3, 0x2, 0x2, 0x2, 0x2f0, 0x2ad, 0x3, 0x2, 
    0x2, 0x2, 0x2f0, 0x2b0, 0x3, 0x2, 0x2, 0x2, 0x2f0, 0x2b3, 0x3, 0x2, 
    0x2, 0x2, 0x2f0, 0x2b6, 0x3, 0x2, 0x2, 0x2, 0x2f0, 0x2b9, 0x3, 0x2, 
    0x2, 0x2, 0x2f0, 0x2bc, 0x3, 0x2, 0x2, 0x2, 0x2f0, 0x2bf, 0x3, 0x2, 
    0x2, 0x2, 0x2f0, 0x2c5, 0x3, 0x2, 0x2, 0x2, 0x2f0, 0x2c9, 0x3, 0x2, 
    0x2, 0x2, 0x2f0, 0x2cf, 0x3, 0x2, 0x2, 0x2, 0x2f0, 0x2d9, 0x3, 0x2, 
    0x2, 0x2, 0x2f0, 0x2df, 0x3, 0x2, 0x2, 0x2, 0x2f0, 0x2ec, 0x3, 0x2, 
    0x2, 0x2, 0x2f0, 0x2ee, 0x3, 0x2, 0x2, 0x2, 0x2f1, 0x2f4, 0x3, 0x2, 
    0x2, 0x2, 0x2f2, 0x2f0, 0x3, 0x2, 0x2, 0x2, 0x2f2, 0x2f3, 0x3, 0x2, 
    0x2, 0x2, 0x2f3, 0x4d, 0x3, 0x2, 0x2, 0x2, 0x2f4, 0x2f2, 0x3, 0x2, 0x2, 
    0x2, 0x2f5, 0x2f6, 0x9, 0xd, 0x2, 0x2, 0x2f6, 0x4f, 0x3, 0x2, 0x2, 0x2, 
    0x2f7, 0x2f9, 0x7, 0x44, 0x2, 0x2, 0x2f8, 0x2fa, 0x5, 0x4c, 0x27, 0x2, 
    0x2f9, 0x2f8, 0x3, 0x2, 0x2, 0x2, 0x2f9, 0x2fa, 0x3, 0x2, 0x2, 0x2, 
    0x2fa, 0x301, 0x3, 0x2, 0x2, 0x2, 0x2fb, 0x2fd, 0x7, 0x5b, 0x2, 0x2, 
    0x2fc, 0x2fe, 0x5, 0x4c, 0x27, 0x2, 0x2fd, 0x2fc, 0x3, 0x2, 0x2, 0x2, 
    0x2fd, 0x2fe, 0x3, 0x2, 0x2, 0x2, 0x2fe, 0x300, 0x3, 0x2, 0x2, 0x2, 
    0x2ff, 0x2fb, 0x3, 0x2, 0x2, 0x2, 0x300, 0x303, 0x3, 0x2, 0x2, 0x2, 
    0x301, 0x2ff, 0x3, 0x2, 0x2, 0x2, 0x301, 0x302, 0x3, 0x2, 0x2, 0x2, 
    0x302, 0x304, 0x3, 0x2, 0x2, 0x2, 0x303, 0x301, 0x3, 0x2, 0x2, 0x2, 
    0x304, 0x305, 0x7, 0x45, 0x2, 0x2, 0x305, 0x51, 0x3, 0x2, 0x2, 0x2, 
    0x306, 0x307, 0x7, 0x46, 0x2, 0x2, 0x307, 0x30c, 0x5, 0x4c, 0x27, 0x2, 
    0x308, 0x309, 0x7, 0x5b, 0x2, 0x2, 0x309, 0x30b, 0x5, 0x4c, 0x27, 0x2, 
    0x30a, 0x308, 0x3, 0x2, 0x2, 0x2, 0x30b, 0x30e, 0x3, 0x2, 0x2, 0x2, 
    0x30c, 0x30a, 0x3, 0x2, 0x2, 0x2, 0x30c, 0x30d, 0x3, 0x2, 0x2, 0x2, 
    0x30d, 0x30f, 0x3, 0x2, 0x2, 0x2, 0x30e, 0x30c, 0x3, 0x2, 0x2, 0x2, 
    0x30f, 0x310, 0x7, 0x47, 0x2, 0x2, 0x310, 0x53, 0x3, 0x2, 0x2, 0x2, 
    0x311, 0x312, 0x9, 0xe, 0x2, 0x2, 0x312, 0x55, 0x3, 0x2, 0x2, 0x2, 0x313, 
    0x319, 0x5, 0x5a, 0x2e, 0x2, 0x314, 0x319, 0x5, 0x60, 0x31, 0x2, 0x315, 
    0x319, 0x5, 0x58, 0x2d, 0x2, 0x316, 0x319, 0x5, 0x5c, 0x2f, 0x2, 0x317, 
    0x319, 0x5, 0x5e, 0x30, 0x2, 0x318, 0x313, 0x3, 0x2, 0x2, 0x2, 0x318, 
    0x314, 0x3, 0x2, 0x2, 0x2, 0x318, 0x315, 0x3, 0x2, 0x2, 0x2, 0x318, 
    0x316, 0x3, 0x2, 0x2, 0x2, 0x318, 0x317, 0x3, 0x2, 0x2, 0x2, 0x319, 
    0x57, 0x3, 0x2, 0x2, 0x2, 0x31a, 0x31b, 0x9, 0xf, 0x2, 0x2, 0x31b, 0x59, 
    0x3, 0x2, 0x2, 0x2, 0x31c, 0x31e, 0x7, 0x74, 0x2, 0x2, 0x31d, 0x31c, 
    0x3, 0x2, 0x2, 0x2, 0x31e, 0x31f, 0x3, 0x2, 0x2, 0x2, 0x31f, 0x31d, 
    0x3, 0x2, 0x2, 0x2, 0x31f, 0x320, 0x3, 0x2, 0x2, 0x2, 0x320, 0x5b, 0x3, 
    0x2, 0x2, 0x2, 0x321, 0x323, 0x7, 0x77, 0x2, 0x2, 0x322, 0x321, 0x3, 
    0x2, 0x2, 0x2, 0x323, 0x324, 0x3, 0x2, 0x2, 0x2, 0x324, 0x322, 0x3, 
    0x2, 0x2, 0x2, 0x324, 0x325, 0x3, 0x2, 0x2, 0x2, 0x325, 0x5d, 0x3, 0x2, 
    0x2, 0x2, 0x326, 0x328, 0x7, 0x76, 0x2, 0x2, 0x327, 0x326, 0x3, 0x2, 
    0x2, 0x2, 0x328, 0x329, 0x3, 0x2, 0x2, 0x2, 0x329, 0x327, 0x3, 0x2, 
    0x2, 0x2, 0x329, 0x32a, 0x3, 0x2, 0x2, 0x2, 0x32a, 0x5f, 0x3, 0x2, 0x2, 
    0x2, 0x32b, 0x32d, 0x9, 0x10, 0x2, 0x2, 0x32c, 0x32e, 0x7, 0x2e, 0x2, 
    0x2, 0x32d, 0x32c, 0x3, 0x2, 0x2, 0x2, 0x32d, 0x32e, 0x3, 0x2, 0x2, 
    0x2, 0x32e, 0x61, 0x3, 0x2, 0x2, 0x2, 0x32f, 0x333, 0x7, 0x48, 0x2, 
    0x2, 0x330, 0x332, 0x5, 0x64, 0x33, 0x2, 0x331, 0x330, 0x3, 0x2, 0x2, 
    0x2, 0x332, 0x335, 0x3, 0x2, 0x2, 0x2, 0x333, 0x331, 0x3, 0x2, 0x2, 
    0x2, 0x333, 0x334, 0x3, 0x2, 0x2, 0x2, 0x334, 0x336, 0x3, 0x2, 0x2, 
    0x2, 0x335, 0x333, 0x3, 0x2, 0x2, 0x2, 0x336, 0x337, 0x7, 0x49, 0x2, 
    0x2, 0x337, 0x63, 0x3, 0x2, 0x2, 0x2, 0x338, 0x345, 0x5, 0x62, 0x32, 
    0x2, 0x339, 0x345, 0x5, 0x66, 0x34, 0x2, 0x33a, 0x345, 0x5, 0x68, 0x35, 
    0x2, 0x33b, 0x345, 0x5, 0x6a, 0x36, 0x2, 0x33c, 0x345, 0x5, 0x6c, 0x37, 
    0x2, 0x33d, 0x345, 0x5, 0x6e, 0x38, 0x2, 0x33e, 0x345, 0x5, 0x70, 0x39, 
    0x2, 0x33f, 0x345, 0x5, 0x72, 0x3a, 0x2, 0x340, 0x345, 0x5, 0x74, 0x3b, 
    0x2, 0x341, 0x345, 0x5, 0x78, 0x3d, 0x2, 0x342, 0x345, 0x5, 0x7a, 0x3e, 
    0x2, 0x343, 0x345, 0x5, 0x7c, 0x3f, 0x2, 0x344, 0x338, 0x3, 0x2, 0x2, 
    0x2, 0x344, 0x339, 0x3, 0x2, 0x2, 0x2, 0x344, 0x33a, 0x3, 0x2, 0x2, 
    0x2, 0x344, 0x33b, 0x3, 0x2, 0x2, 0x2, 0x344, 0x33c, 0x3, 0x2, 0x2, 
    0x2, 0x344, 0x33d, 0x3, 0x2, 0x2, 0x2, 0x344, 0x33e, 0x3, 0x2, 0x2, 
    0x2, 0x344, 0x33f, 0x3, 0x2, 0x2, 0x2, 0x344, 0x340, 0x3, 0x2, 0x2, 
    0x2, 0x344, 0x341, 0x3, 0x2, 0x2, 0x2, 0x344, 0x342, 0x3, 0x2, 0x2, 
    0x2, 0x344, 0x343, 0x3, 0x2, 0x2, 0x2, 0x345, 0x65, 0x3, 0x2, 0x2, 0x2, 
    0x346, 0x349, 0x5, 0x82, 0x42, 0x2, 0x347, 0x349, 0x5, 0x84, 0x43, 0x2, 
    0x348, 0x346, 0x3, 0x2, 0x2, 0x2, 0x348, 0x347, 0x3, 0x2, 0x2, 0x2, 
    0x349, 0x67, 0x3, 0x2, 0x2, 0x2, 0x34a, 0x34b, 0x7, 0x22, 0x2, 0x2, 
    0x34b, 0x34c, 0x7, 0x44, 0x2, 0x2, 0x34c, 0x34d, 0x5, 0x4c, 0x27, 0x2, 
    0x34d, 0x34e, 0x7, 0x45, 0x2, 0x2, 0x34e, 0x351, 0x5, 0x64, 0x33, 0x2, 
    0x34f, 0x350, 0x7, 0x15, 0x2, 0x2, 0x350, 0x352, 0x5, 0x64, 0x33, 0x2, 
    0x351, 0x34f, 0x3, 0x2, 0x2, 0x2, 0x351, 0x352, 0x3, 0x2, 0x2, 0x2, 
    0x352, 0x69, 0x3, 0x2, 0x2, 0x2, 0x353, 0x354, 0x7, 0x1f, 0x2, 0x2, 
    0x354, 0x357, 0x7, 0x44, 0x2, 0x2, 0x355, 0x358, 0x5, 0x66, 0x34, 0x2, 
    0x356, 0x358, 0x7, 0x4b, 0x2, 0x2, 0x357, 0x355, 0x3, 0x2, 0x2, 0x2, 
    0x357, 0x356, 0x3, 0x2, 0x2, 0x2, 0x358, 0x35b, 0x3, 0x2, 0x2, 0x2, 
    0x359, 0x35c, 0x5, 0x84, 0x43, 0x2, 0x35a, 0x35c, 0x7, 0x4b, 0x2, 0x2, 
    0x35b, 0x359, 0x3, 0x2, 0x2, 0x2, 0x35b, 0x35a, 0x3, 0x2, 0x2, 0x2, 
    0x35c, 0x35e, 0x3, 0x2, 0x2, 0x2, 0x35d, 0x35f, 0x5, 0x4c, 0x27, 0x2, 
    0x35e, 0x35d, 0x3, 0x2, 0x2, 0x2, 0x35e, 0x35f, 0x3, 0x2, 0x2, 0x2, 
    0x35f, 0x360, 0x3, 0x2, 0x2, 0x2, 0x360, 0x361, 0x7, 0x45, 0x2, 0x2, 
    0x361, 0x362, 0x5, 0x64, 0x33, 0x2, 0x362, 0x6b, 0x3, 0x2, 0x2, 0x2, 
    0x363, 0x364, 0x7, 0x43, 0x2, 0x2, 0x364, 0x365, 0x7, 0x44, 0x2, 0x2, 
    0x365, 0x366, 0x5, 0x4c, 0x27, 0x2, 0x366, 0x367, 0x7, 0x45, 0x2, 0x2, 
    0x367, 0x368, 0x5, 0x64, 0x33, 0x2, 0x368, 0x6d, 0x3, 0x2, 0x2, 0x2, 
    0x369, 0x36a, 0x7, 0x14, 0x2, 0x2, 0x36a, 0x36b, 0x5, 0x64, 0x33, 0x2, 
    0x36b, 0x36c, 0x7, 0x43, 0x2, 0x2, 0x36c, 0x36d, 0x7, 0x44, 0x2, 0x2, 
    0x36d, 0x36e, 0x5, 0x4c, 0x27, 0x2, 0x36e, 0x36f, 0x7, 0x45, 0x2, 0x2, 
    0x36f, 0x370, 0x7, 0x4b, 0x2, 0x2, 0x370, 0x6f, 0x3, 0x2, 0x2, 0x2, 
    0x371, 0x372, 0x7, 0x11, 0x2, 0x2, 0x372, 0x373, 0x7, 0x4b, 0x2, 0x2, 
    0x373, 0x71, 0x3, 0x2, 0x2, 0x2, 0x374, 0x375, 0x7, 0xb, 0x2, 0x2, 0x375, 
    0x376, 0x7, 0x4b, 0x2, 0x2, 0x376, 0x73, 0x3, 0x2, 0x2, 0x2, 0x377, 
    0x378, 0x7, 0x3c, 0x2, 0x2, 0x378, 0x37e, 0x5, 0x4c, 0x27, 0x2, 0x379, 
    0x37a, 0x7, 0x36, 0x2, 0x2, 0x37a, 0x37b, 0x7, 0x44, 0x2, 0x2, 0x37b, 
    0x37c, 0x5, 0x24, 0x13, 0x2, 0x37c, 0x37d, 0x7, 0x45, 0x2, 0x2, 0x37d, 
    0x37f, 0x3, 0x2, 0x2, 0x2, 0x37e, 0x379, 0x3, 0x2, 0x2, 0x2, 0x37e, 
    0x37f, 0x3, 0x2, 0x2, 0x2, 0x37f, 0x380, 0x3, 0x2, 0x2, 0x2, 0x380, 
    0x382, 0x5, 0x62, 0x32, 0x2, 0x381, 0x383, 0x5, 0x76, 0x3c, 0x2, 0x382, 
    0x381, 0x3, 0x2, 0x2, 0x2, 0x383, 0x384, 0x3, 0x2, 0x2, 0x2, 0x384, 
    0x382, 0x3, 0x2, 0x2, 0x2, 0x384, 0x385, 0x3, 0x2, 0x2, 0x2, 0x385, 
    0x75, 0x3, 0x2, 0x2, 0x2, 0x386, 0x38e, 0x7, 0xe, 0x2, 0x2, 0x387, 0x389, 
    0x5, 0x54, 0x2b, 0x2, 0x388, 0x387, 0x3, 0x2, 0x2, 0x2, 0x388, 0x389, 
    0x3, 0x2, 0x2, 0x2, 0x389, 0x38a, 0x3, 0x2, 0x2, 0x2, 0x38a, 0x38b, 
    0x7, 0x44, 0x2, 0x2, 0x38b, 0x38c, 0x5, 0x24, 0x13, 0x2, 0x38c, 0x38d, 
    0x7, 0x45, 0x2, 0x2, 0x38d, 0x38f, 0x3, 0x2, 0x2, 0x2, 0x38e, 0x388, 
    0x3, 0x2, 0x2, 0x2, 0x38e, 0x38f, 0x3, 0x2, 0x2, 0x2, 0x38f, 0x390, 
    0x3, 0x2, 0x2, 0x2, 0x390, 0x391, 0x5, 0x62, 0x32, 0x2, 0x391, 0x77, 
    0x3, 0x2, 0x2, 0x2, 0x392, 0x394, 0x7, 0x35, 0x2, 0x2, 0x393, 0x395, 
    0x5, 0x4c, 0x27, 0x2, 0x394, 0x393, 0x3, 0x2, 0x2, 0x2, 0x394, 0x395, 
    0x3, 0x2, 0x2, 0x2, 0x395, 0x396, 0x3, 0x2, 0x2, 0x2, 0x396, 0x397, 
    0x7, 0x4b, 0x2, 0x2, 0x397, 0x79, 0x3, 0x2, 0x2, 0x2, 0x398, 0x399, 
    0x7, 0x16, 0x2, 0x2, 0x399, 0x39a, 0x5, 0x4c, 0x27, 0x2, 0x39a, 0x39b, 
    0x5, 0x1c, 0xf, 0x2, 0x39b, 0x39c, 0x7, 0x4b, 0x2, 0x2, 0x39c, 0x7b, 
    0x3, 0x2, 0x2, 0x2, 0x39d, 0x39f, 0x7, 0x9, 0x2, 0x2, 0x39e, 0x3a0, 
    0x7, 0x7e, 0x2, 0x2, 0x39f, 0x39e, 0x3, 0x2, 0x2, 0x2, 0x39f, 0x3a0, 
    0x3, 0x2, 0x2, 0x2, 0x3a0, 0x3a1, 0x3, 0x2, 0x2, 0x2, 0x3a1, 0x3a5, 
    0x7, 0x7f, 0x2, 0x2, 0x3a2, 0x3a4, 0x5, 0x8a, 0x46, 0x2, 0x3a3, 0x3a2, 
    0x3, 0x2, 0x2, 0x2, 0x3a4, 0x3a7, 0x3, 0x2, 0x2, 0x2, 0x3a5, 0x3a3, 
    0x3, 0x2, 0x2, 0x2, 0x3a5, 0x3a6, 0x3, 0x2, 0x2, 0x2, 0x3a6, 0x3a8, 
    0x3, 0x2, 0x2, 0x2, 0x3a7, 0x3a5, 0x3, 0x2, 0x2, 0x2, 0x3a8, 0x3a9, 
    0x7, 0x91, 0x2, 0x2, 0x3a9, 0x7d, 0x3, 0x2, 0x2, 0x2, 0x3aa, 0x3af, 
    0x5, 0x48, 0x25, 0x2, 0x3ab, 0x3ac, 0x7, 0x5b, 0x2, 0x2, 0x3ac, 0x3ae, 
    0x5, 0x48, 0x25, 0x2, 0x3ad, 0x3ab, 0x3, 0x2, 0x2, 0x2, 0x3ae, 0x3b1, 
    0x3, 0x2, 0x2, 0x2, 0x3af, 0x3ad, 0x3, 0x2, 0x2, 0x2, 0x3af, 0x3b0, 
    0x3, 0x2, 0x2, 0x2, 0x3b0, 0x7f, 0x3, 0x2, 0x2, 0x2, 0x3b1, 0x3af, 0x3, 
    0x2, 0x2, 0x2, 0x3b2, 0x3b6, 0x7, 0x44, 0x2, 0x2, 0x3b3, 0x3b5, 0x7, 
    0x5b, 0x2, 0x2, 0x3b4, 0x3b3, 0x3, 0x2, 0x2, 0x2, 0x3b5, 0x3b8, 0x3, 
    0x2, 0x2, 0x2, 0x3b6, 0x3b4, 0x3, 0x2, 0x2, 0x2, 0x3b6, 0x3b7, 0x3, 
    0x2, 0x2, 0x2, 0x3b7, 0x3b9, 0x3, 0x2, 0x2, 0x2, 0x3b8, 0x3b6, 0x3, 
    0x2, 0x2, 0x2, 0x3b9, 0x3ba, 0x5, 0x48, 0x25, 0x2, 0x3ba, 0x3c1, 0x3, 
    0x2, 0x2, 0x2, 0x3bb, 0x3bd, 0x7, 0x5b, 0x2, 0x2, 0x3bc, 0x3be, 0x5, 
    0x48, 0x25, 0x2, 0x3bd, 0x3bc, 0x3, 0x2, 0x2, 0x2, 0x3bd, 0x3be, 0x3, 
    0x2, 0x2, 0x2, 0x3be, 0x3c0, 0x3, 0x2, 0x2, 0x2, 0x3bf, 0x3bb, 0x3, 
    0x2, 0x2, 0x2, 0x3c0, 0x3c3, 0x3, 0x2, 0x2, 0x2, 0x3c1, 0x3bf, 0x3, 
    0x2, 0x2, 0x2, 0x3c1, 0x3c2, 0x3, 0x2, 0x2, 0x2, 0x3c2, 0x3c4, 0x3, 
    0x2, 0x2, 0x2, 0x3c3, 0x3c1, 0x3, 0x2, 0x2, 0x2, 0x3c4, 0x3c5, 0x7, 
    0x45, 0x2, 0x2, 0x3c5, 0x81, 0x3, 0x2, 0x2, 0x2, 0x3c6, 0x3c9, 0x5, 
    0x48, 0x25, 0x2, 0x3c7, 0x3c8, 0x7, 0x4f, 0x2, 0x2, 0x3c8, 0x3ca, 0x5, 
    0x4c, 0x27, 0x2, 0x3c9, 0x3c7, 0x3, 0x2, 0x2, 0x2, 0x3c9, 0x3ca, 0x3, 
    0x2, 0x2, 0x2, 0x3ca, 0x3d0, 0x3, 0x2, 0x2, 0x2, 0x3cb, 0x3cc, 0x5, 
    0x80, 0x41, 0x2, 0x3cc, 0x3cd, 0x7, 0x4f, 0x2, 0x2, 0x3cd, 0x3ce, 0x5, 
    0x4c, 0x27, 0x2, 0x3ce, 0x3d0, 0x3, 0x2, 0x2, 0x2, 0x3cf, 0x3c6, 0x3, 
    0x2, 0x2, 0x2, 0x3cf, 0x3cb, 0x3, 0x2, 0x2, 0x2, 0x3d0, 0x3d1, 0x3, 
    0x2, 0x2, 0x2, 0x3d1, 0x3d2, 0x7, 0x4b, 0x2, 0x2, 0x3d2, 0x83, 0x3, 
    0x2, 0x2, 0x2, 0x3d3, 0x3d4, 0x5, 0x4c, 0x27, 0x2, 0x3d4, 0x3d5, 0x7, 
    0x4b, 0x2, 0x2, 0x3d5, 0x85, 0x3, 0x2, 0x2, 0x2, 0x3d6, 0x3d7, 0x7, 
    0x2a, 0x2, 0x2, 0x3d7, 0x3d8, 0x7, 0x44, 0x2, 0x2, 0x3d8, 0x3d9, 0x5, 
    0x88, 0x45, 0x2, 0x3d9, 0x3da, 0x7, 0x4e, 0x2, 0x2, 0x3da, 0x3db, 0x5, 
    0x42, 0x22, 0x2, 0x3db, 0x3dc, 0x7, 0x45, 0x2, 0x2, 0x3dc, 0x87, 0x3, 
    0x2, 0x2, 0x2, 0x3dd, 0x3e0, 0x5, 0x44, 0x23, 0x2, 0x3de, 0x3e0, 0x5, 
    0x1e, 0x10, 0x2, 0x3df, 0x3dd, 0x3, 0x2, 0x2, 0x2, 0x3df, 0x3de, 0x3, 
    0x2, 0x2, 0x2, 0x3e0, 0x89, 0x3, 0x2, 0x2, 0x2, 0x3e1, 0x3ed, 0x5, 0x8c, 
    0x47, 0x2, 0x3e2, 0x3ed, 0x5, 0x8e, 0x48, 0x2, 0x3e3, 0x3ed, 0x5, 0x90, 
    0x49, 0x2, 0x3e4, 0x3ed, 0x5, 0x9e, 0x50, 0x2, 0x3e5, 0x3ed, 0x5, 0x92, 
    0x4a, 0x2, 0x3e6, 0x3ed, 0x5, 0x94, 0x4b, 0x2, 0x3e7, 0x3ed, 0x5, 0x98, 
    0x4d, 0x2, 0x3e8, 0x3ed, 0x7, 0x8b, 0x2, 0x2, 0x3e9, 0x3ed, 0x7, 0x83, 
    0x2, 0x2, 0x3ea, 0x3ed, 0x7, 0x85, 0x2, 0x2, 0x3eb, 0x3ed, 0x5, 0x9a, 
    0x4e, 0x2, 0x3ec, 0x3e1, 0x3, 0x2, 0x2, 0x2, 0x3ec, 0x3e2, 0x3, 0x2, 
    0x2, 0x2, 0x3ec, 0x3e3, 0x3, 0x2, 0x2, 0x2, 0x3ec, 0x3e4, 0x3, 0x2, 
    0x2, 0x2, 0x3ec, 0x3e5, 0x3, 0x2, 0x2, 0x2, 0x3ec, 0x3e6, 0x3, 0x2, 
    0x2, 0x2, 0x3ec, 0x3e7, 0x3, 0x2, 0x2, 0x2, 0x3ec, 0x3e8, 0x3, 0x2, 
    0x2, 0x2, 0x3ec, 0x3e9, 0x3, 0x2, 0x2, 0x2, 0x3ec, 0x3ea, 0x3, 0x2, 
    0x2, 0x2, 0x3ec, 0x3eb, 0x3, 0x2, 0x2, 0x2, 0x3ed, 0x8b, 0x3, 0x2, 0x2, 
    0x2, 0x3ee, 0x3f2, 0x7, 0x90, 0x2, 0x2, 0x3ef, 0x3f1, 0x5, 0x8a, 0x46, 
    0x2, 0x3f0, 0x3ef, 0x3, 0x2, 0x2, 0x2, 0x3f1, 0x3f4, 0x3, 0x2, 0x2, 
    0x2, 0x3f2, 0x3f0, 0x3, 0x2, 0x2, 0x2, 0x3f2, 0x3f3, 0x3, 0x2, 0x2, 
    0x2, 0x3f3, 0x3f5, 0x3, 0x2, 0x2, 0x2, 0x3f4, 0x3f2, 0x3, 0x2, 0x2, 
    0x2, 0x3f5, 0x3f6, 0x7, 0x91, 0x2, 0x2, 0x3f6, 0x8d, 0x3, 0x2, 0x2, 
    0x2, 0x3f7, 0x3f8, 0x7, 0x8c, 0x2, 0x2, 0x3f8, 0x3fb, 0x7, 0x98, 0x2, 
    0x2, 0x3f9, 0x3fa, 0x7, 0x94, 0x2, 0x2, 0x3fa, 0x3fc, 0x5, 0xa4, 0x53, 
    0x2, 0x3fb, 0x3f9, 0x3, 0x2, 0x2, 0x2, 0x3fb, 0x3fc, 0x3, 0x2, 0x2, 
    0x2, 0x3fc, 0x40b, 0x3, 0x2, 0x2, 0x2, 0x3fd, 0x3fe, 0x7, 0x8c, 0x2, 
    0x2, 0x3fe, 0x403, 0x7, 0x98, 0x2, 0x2, 0x3ff, 0x400, 0x7, 0x96, 0x2, 
    0x2, 0x400, 0x402, 0x7, 0x98, 0x2, 0x2, 0x401, 0x3ff, 0x3, 0x2, 0x2, 
    0x2, 0x402, 0x405, 0x3, 0x2, 0x2, 0x2, 0x403, 0x401, 0x3, 0x2, 0x2, 
    0x2, 0x403, 0x404, 0x3, 0x2, 0x2, 0x2, 0x404, 0x408, 0x3, 0x2, 0x2, 
    0x2, 0x405, 0x403, 0x3, 0x2, 0x2, 0x2, 0x406, 0x407, 0x7, 0x94, 0x2, 
    0x2, 0x407, 0x409, 0x5, 0x9e, 0x50, 0x2, 0x408, 0x406, 0x3, 0x2, 0x2, 
    0x2, 0x408, 0x409, 0x3, 0x2, 0x2, 0x2, 0x409, 0x40b, 0x3, 0x2, 0x2, 
    0x2, 0x40a, 0x3f7, 0x3, 0x2, 0x2, 0x2, 0x40a, 0x3fd, 0x3, 0x2, 0x2, 
    0x2, 0x40b, 0x8f, 0x3, 0x2, 0x2, 0x2, 0x40c, 0x40d, 0x5, 0x9c, 0x4f, 
    0x2, 0x40d, 0x40e, 0x7, 0x94, 0x2, 0x2, 0x40e, 0x40f, 0x5, 0xa4, 0x53, 
    0x2, 0x40f, 0x41b, 0x3, 0x2, 0x2, 0x2, 0x410, 0x413, 0x5, 0x9c, 0x4f, 
    0x2, 0x411, 0x412, 0x7, 0x96, 0x2, 0x2, 0x412, 0x414, 0x5, 0x9c, 0x4f, 
    0x2, 0x413, 0x411, 0x3, 0x2, 0x2, 0x2, 0x414, 0x415, 0x3, 0x2, 0x2, 
    0x2, 0x415, 0x413, 0x3, 0x2, 0x2, 0x2, 0x415, 0x416, 0x3, 0x2, 0x2, 
    0x2, 0x416, 0x417, 0x3, 0x2, 0x2, 0x2, 0x417, 0x418, 0x7, 0x94, 0x2, 
    0x2, 0x418, 0x419, 0x5, 0x9e, 0x50, 0x2, 0x419, 0x41b, 0x3, 0x2, 0x2, 
    0x2, 0x41a, 0x40c, 0x3, 0x2, 0x2, 0x2, 0x41a, 0x410, 0x3, 0x2, 0x2, 
    0x2, 0x41b, 0x91, 0x3, 0x2, 0x2, 0x2, 0x41c, 0x41d, 0x7, 0x8a, 0x2, 
    0x2, 0x41d, 0x41e, 0x5, 0xa4, 0x53, 0x2, 0x41e, 0x41f, 0x5, 0x8c, 0x47, 
    0x2, 0x41f, 0x93, 0x3, 0x2, 0x2, 0x2, 0x420, 0x421, 0x7, 0x88, 0x2, 
    0x2, 0x421, 0x422, 0x5, 0x8c, 0x47, 0x2, 0x422, 0x423, 0x5, 0xa4, 0x53, 
    0x2, 0x423, 0x424, 0x5, 0x8c, 0x47, 0x2, 0x424, 0x425, 0x5, 0x8c, 0x47, 
    0x2, 0x425, 0x95, 0x3, 0x2, 0x2, 0x2, 0x426, 0x427, 0x7, 0x84, 0x2, 
    0x2, 0x427, 0x428, 0x5, 0xa2, 0x52, 0x2, 0x428, 0x429, 0x5, 0x8c, 0x47, 
    0x2, 0x429, 0x97, 0x3, 0x2, 0x2, 0x2, 0x42a, 0x42b, 0x7, 0x8d, 0x2, 
    0x2, 0x42b, 0x437, 0x5, 0xa4, 0x53, 0x2, 0x42c, 0x42e, 0x5, 0x96, 0x4c, 
    0x2, 0x42d, 0x42c, 0x3, 0x2, 0x2, 0x2, 0x42e, 0x42f, 0x3, 0x2, 0x2, 
    0x2, 0x42f, 0x42d, 0x3, 0x2, 0x2, 0x2, 0x42f, 0x430, 0x3, 0x2, 0x2, 
    0x2, 0x430, 0x433, 0x3, 0x2, 0x2, 0x2, 0x431, 0x432, 0x7, 0x86, 0x2, 
    0x2, 0x432, 0x434, 0x5, 0x8c, 0x47, 0x2, 0x433, 0x431, 0x3, 0x2, 0x2, 
    0x2, 0x433, 0x434, 0x3, 0x2, 0x2, 0x2, 0x434, 0x438, 0x3, 0x2, 0x2, 
    0x2, 0x435, 0x436, 0x7, 0x86, 0x2, 0x2, 0x436, 0x438, 0x5, 0x8c, 0x47, 
    0x2, 0x437, 0x42d, 0x3, 0x2, 0x2, 0x2, 0x437, 0x435, 0x3, 0x2, 0x2, 
    0x2, 0x438, 0x99, 0x3, 0x2, 0x2, 0x2, 0x439, 0x43a, 0x7, 0x89, 0x2, 
    0x2, 0x43a, 0x43b, 0x7, 0x98, 0x2, 0x2, 0x43b, 0x444, 0x7, 0x92, 0x2, 
    0x2, 0x43c, 0x441, 0x7, 0x98, 0x2, 0x2, 0x43d, 0x43e, 0x7, 0x96, 0x2, 
    0x2, 0x43e, 0x440, 0x7, 0x98, 0x2, 0x2, 0x43f, 0x43d, 0x3, 0x2, 0x2, 
    0x2, 0x440, 0x443, 0x3, 0x2, 0x2, 0x2, 0x441, 0x43f, 0x3, 0x2, 0x2, 
    0x2, 0x441, 0x442, 0x3, 0x2, 0x2, 0x2, 0x442, 0x445, 0x3, 0x2, 0x2, 
    0x2, 0x443, 0x441, 0x3, 0x2, 0x2, 0x2, 0x444, 0x43c, 0x3, 0x2, 0x2, 
    0x2, 0x444, 0x445, 0x3, 0x2, 0x2, 0x2, 0x445, 0x446, 0x3, 0x2, 0x2, 
    0x2, 0x446, 0x450, 0x7, 0x93, 0x2, 0x2, 0x447, 0x448, 0x7, 0x97, 0x2, 
    0x2, 0x448, 0x44d, 0x7, 0x98, 0x2, 0x2, 0x449, 0x44a, 0x7, 0x96, 0x2, 
    0x2, 0x44a, 0x44c, 0x7, 0x98, 0x2, 0x2, 0x44b, 0x449, 0x3, 0x2, 0x2, 
    0x2, 0x44c, 0x44f, 0x3, 0x2, 0x2, 0x2, 0x44d, 0x44b, 0x3, 0x2, 0x2, 
    0x2, 0x44d, 0x44e, 0x3, 0x2, 0x2, 0x2, 0x44e, 0x451, 0x3, 0x2, 0x2, 
    0x2, 0x44f, 0x44d, 0x3, 0x2, 0x2, 0x2, 0x450, 0x447, 0x3, 0x2, 0x2, 
    0x2, 0x450, 0x451, 0x3, 0x2, 0x2, 0x2, 0x451, 0x452, 0x3, 0x2, 0x2, 
    0x2, 0x452, 0x453, 0x5, 0x8c, 0x47, 0x2, 0x453, 0x9b, 0x3, 0x2, 0x2, 
    0x2, 0x454, 0x459, 0x7, 0x98, 0x2, 0x2, 0x455, 0x456, 0x7, 0x95, 0x2, 
    0x2, 0x456, 0x458, 0x7, 0x98, 0x2, 0x2, 0x457, 0x455, 0x3, 0x2, 0x2, 
    0x2, 0x458, 0x45b, 0x3, 0x2, 0x2, 0x2, 0x459, 0x457, 0x3, 0x2, 0x2, 
    0x2, 0x459, 0x45a, 0x3, 0x2, 0x2, 0x2, 0x45a, 0x9d, 0x3, 0x2, 0x2, 0x2, 
    0x45b, 0x459, 0x3, 0x2, 0x2, 0x2, 0x45c, 0x45d, 0x9, 0x11, 0x2, 0x2, 
    0x45d, 0x466, 0x7, 0x92, 0x2, 0x2, 0x45e, 0x463, 0x5, 0xa4, 0x53, 0x2, 
    0x45f, 0x460, 0x7, 0x96, 0x2, 0x2, 0x460, 0x462, 0x5, 0xa4, 0x53, 0x2, 
    0x461, 0x45f, 0x3, 0x2, 0x2, 0x2, 0x462, 0x465, 0x3, 0x2, 0x2, 0x2, 
    0x463, 0x461, 0x3, 0x2, 0x2, 0x2, 0x463, 0x464, 0x3, 0x2, 0x2, 0x2, 
    0x464, 0x467, 0x3, 0x2, 0x2, 0x2, 0x465, 0x463, 0x3, 0x2, 0x2, 0x2, 
    0x466, 0x45e, 0x3, 0x2, 0x2, 0x2, 0x466, 0x467, 0x3, 0x2, 0x2, 0x2, 
    0x467, 0x468, 0x3, 0x2, 0x2, 0x2, 0x468, 0x469, 0x7, 0x93, 0x2, 0x2, 
    0x469, 0x9f, 0x3, 0x2, 0x2, 0x2, 0x46a, 0x46b, 0x9, 0x12, 0x2, 0x2, 
    0x46b, 0xa1, 0x3, 0x2, 0x2, 0x2, 0x46c, 0x471, 0x7, 0x9a, 0x2, 0x2, 
    0x46d, 0x471, 0x7, 0x9b, 0x2, 0x2, 0x46e, 0x471, 0x7, 0x99, 0x2, 0x2, 
    0x46f, 0x471, 0x5, 0xa0, 0x51, 0x2, 0x470, 0x46c, 0x3, 0x2, 0x2, 0x2, 
    0x470, 0x46d, 0x3, 0x2, 0x2, 0x2, 0x470, 0x46e, 0x3, 0x2, 0x2, 0x2, 
    0x470, 0x46f, 0x3, 0x2, 0x2, 0x2, 0x471, 0xa3, 0x3, 0x2, 0x2, 0x2, 0x472, 
    0x476, 0x5, 0x9c, 0x4f, 0x2, 0x473, 0x476, 0x5, 0x9e, 0x50, 0x2, 0x474, 
    0x476, 0x5, 0xa2, 0x52, 0x2, 0x475, 0x472, 0x3, 0x2, 0x2, 0x2, 0x475, 
    0x473, 0x3, 0x2, 0x2, 0x2, 0x475, 0x474, 0x3, 0x2, 0x2, 0x2, 0x476, 
    0xa5, 0x3, 0x2, 0x2, 0x2, 0x7f, 0xae, 0xb0, 0xb9, 0xc1, 0xcd, 0xd4, 
    0xde, 0xe4, 0xe9, 0xef, 0xf7, 0xfd, 0x108, 0x113, 0x118, 0x123, 0x12f, 
    0x132, 0x13a, 0x13d, 0x140, 0x149, 0x14e, 0x157, 0x15c, 0x15f, 0x164, 
    0x171, 0x173, 0x181, 0x186, 0x18c, 0x190, 0x1a3, 0x1a5, 0x1ad, 0x1b1, 
    0x1b7, 0x1ba, 0x1c3, 0x1c5, 0x1ca, 0x1df, 0x1e1, 0x1e6, 0x1ee, 0x1fe, 
    0x217, 0x219, 0x21f, 0x225, 0x228, 0x232, 0x235, 0x239, 0x242, 0x24b, 
    0x250, 0x255, 0x264, 0x269, 0x274, 0x276, 0x27e, 0x282, 0x29a, 0x29c, 
    0x2cc, 0x2d2, 0x2d6, 0x2dd, 0x2e6, 0x2e9, 0x2f0, 0x2f2, 0x2f9, 0x2fd, 
    0x301, 0x30c, 0x318, 0x31f, 0x324, 0x329, 0x32d, 0x333, 0x344, 0x348, 
    0x351, 0x357, 0x35b, 0x35e, 0x37e, 0x384, 0x388, 0x38e, 0x394, 0x39f, 
    0x3a5, 0x3af, 0x3b6, 0x3bd, 0x3c1, 0x3c9, 0x3cf, 0x3df, 0x3ec, 0x3f2, 
    0x3fb, 0x403, 0x408, 0x40a, 0x415, 0x41a, 0x42f, 0x433, 0x437, 0x441, 
    0x444, 0x44d, 0x450, 0x459, 0x463, 0x466, 0x470, 0x475, 
  };

  atn::ATNDeserializer deserializer;
  _atn = deserializer.deserialize(_serializedATN);

  size_t count = _atn.getNumberOfDecisions();
  _decisionToDFA.reserve(count);
  for (size_t i = 0; i < count; i++) { 
    _decisionToDFA.emplace_back(_atn.getDecisionState(i), i);
  }
}

SolidityParser::Initializer SolidityParser::_init;
