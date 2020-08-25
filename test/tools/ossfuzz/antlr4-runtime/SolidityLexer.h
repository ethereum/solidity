
// Generated from docs/grammar/SolidityLexer.g4 by ANTLR 4.8

#pragma once


#include "antlr4-runtime.h"




class  SolidityLexer : public antlr4::Lexer {
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
    AssemblyBlockMode = 1, YulMode = 2, PragmaMode = 3
  };

  SolidityLexer(antlr4::CharStream *input);
  ~SolidityLexer();

  virtual std::string getGrammarFileName() const override;
  virtual const std::vector<std::string>& getRuleNames() const override;

  virtual const std::vector<std::string>& getChannelNames() const override;
  virtual const std::vector<std::string>& getModeNames() const override;
  virtual const std::vector<std::string>& getTokenNames() const override; // deprecated, use vocabulary instead
  virtual antlr4::dfa::Vocabulary& getVocabulary() const override;

  virtual const std::vector<uint16_t> getSerializedATN() const override;
  virtual const antlr4::atn::ATN& getATN() const override;

private:
  static std::vector<antlr4::dfa::DFA> _decisionToDFA;
  static antlr4::atn::PredictionContextCache _sharedContextCache;
  static std::vector<std::string> _ruleNames;
  static std::vector<std::string> _tokenNames;
  static std::vector<std::string> _channelNames;
  static std::vector<std::string> _modeNames;

  static std::vector<std::string> _literalNames;
  static std::vector<std::string> _symbolicNames;
  static antlr4::dfa::Vocabulary _vocabulary;
  static antlr4::atn::ATN _atn;
  static std::vector<uint16_t> _serializedATN;


  // Individual action functions triggered by action() above.

  // Individual semantic predicate functions triggered by sempred() above.

  struct Initializer {
    Initializer();
  };
  static Initializer _init;
};

