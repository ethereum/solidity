// Copyright 2006-2012, the V8 project authors. All rights reserved.
// Redistribution and use in source and binary forms, with or without
// modification, are permitted provided that the following conditions are
// met:
//
//    * Redistributions of source code must retain the above copyright
//      notice, this list of conditions and the following disclaimer.
//    * Redistributions in binary form must reproduce the above
//      copyright notice, this list of conditions and the following
//      disclaimer in the documentation and/or other materials provided
//      with the distribution.
//    * Neither the name of Google Inc. nor the names of its
//      contributors may be used to endorse or promote products derived
//      from this software without specific prior written permission.
//
// THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
// "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
// LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
// A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT
// OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
// SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
// LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
// DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
// THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
// (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
// OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
//
// Modifications as part of cpp-ethereum under the following license:
//
// cpp-ethereum is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
//
// cpp-ethereum is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with cpp-ethereum.  If not, see <http://www.gnu.org/licenses/>.

#pragma once

#include <boost/assert.hpp>

#include <libdevcore/Common.h>
#include <libdevcore/Log.h>

namespace dev {
namespace solidity {

// TOKEN_LIST takes a list of 3 macros M, all of which satisfy the
// same signature M(name, string, precedence), where name is the
// symbolic token name, string is the corresponding syntactic symbol
// (or NULL, for literals), and precedence is the precedence (or 0).
// The parameters are invoked for token categories as follows:
//
//   T: Non-keyword tokens
//   K: Keyword tokens

// IGNORE_TOKEN is a convenience macro that can be supplied as
// an argument (at any position) for a TOKEN_LIST call. It does
// nothing with tokens belonging to the respective category.

#define IGNORE_TOKEN(name, string, precedence)

#define TOKEN_LIST(T, K)                                             \
  /* End of source indicator. */                                     \
  T(EOS, "EOS", 0)                                                   \
                                                                     \
  /* Punctuators (ECMA-262, section 7.7, page 15). */                \
  T(LPAREN, "(", 0)                                                  \
  T(RPAREN, ")", 0)                                                  \
  T(LBRACK, "[", 0)                                                  \
  T(RBRACK, "]", 0)                                                  \
  T(LBRACE, "{", 0)                                                  \
  T(RBRACE, "}", 0)                                                  \
  T(COLON, ":", 0)                                                   \
  T(SEMICOLON, ";", 0)                                               \
  T(PERIOD, ".", 0)                                                  \
  T(CONDITIONAL, "?", 3)                                             \
  T(INC, "++", 0)                                                    \
  T(DEC, "--", 0)                                                    \
  T(ARROW, "=>", 0)                                                  \
                                                                     \
  /* Assignment operators. */                                        \
  /* IsAssignmentOp() and Assignment::is_compound() relies on */     \
  /* this block of enum values being contiguous and sorted in the */ \
  /* same order! */                                                  \
  T(INIT_VAR, "=init_var", 2)                   /* AST-use only. */  \
  T(INIT_LET, "=init_let", 2)                   /* AST-use only. */  \
  T(INIT_CONST, "=init_const", 2)               /* AST-use only. */  \
  T(INIT_CONST_LEGACY, "=init_const_legacy", 2) /* AST-use only. */  \
  T(ASSIGN, "=", 2)                                                  \
  T(ASSIGN_BIT_OR, "|=", 2)                                          \
  T(ASSIGN_BIT_XOR, "^=", 2)                                         \
  T(ASSIGN_BIT_AND, "&=", 2)                                         \
  T(ASSIGN_SHL, "<<=", 2)                                            \
  T(ASSIGN_SAR, ">>=", 2)                                            \
  T(ASSIGN_SHR, ">>>=", 2)                                           \
  T(ASSIGN_ADD, "+=", 2)                                             \
  T(ASSIGN_SUB, "-=", 2)                                             \
  T(ASSIGN_MUL, "*=", 2)                                             \
  T(ASSIGN_DIV, "/=", 2)                                             \
  T(ASSIGN_MOD, "%=", 2)                                             \
                                                                     \
  /* Binary operators sorted by precedence. */                       \
  /* IsBinaryOp() relies on this block of enum values */             \
  /* being contiguous and sorted in the same order! */               \
  T(COMMA, ",", 1)                                                   \
  T(OR, "||", 4)                                                     \
  T(AND, "&&", 5)                                                    \
  T(BIT_OR, "|", 6)                                                  \
  T(BIT_XOR, "^", 7)                                                 \
  T(BIT_AND, "&", 8)                                                 \
  T(SHL, "<<", 11)                                                   \
  T(SAR, ">>", 11)                                                   \
  T(SHR, ">>>", 11)                                                  \
  T(ROR, "rotate right", 11) /* only used by Crankshaft */           \
  T(ADD, "+", 12)                                                    \
  T(SUB, "-", 12)                                                    \
  T(MUL, "*", 13)                                                    \
  T(DIV, "/", 13)                                                    \
  T(MOD, "%", 13)                                                    \
                                                                     \
  /* Compare operators sorted by precedence. */                      \
  /* IsCompareOp() relies on this block of enum values */            \
  /* being contiguous and sorted in the same order! */               \
  T(EQ, "==", 9)                                                     \
  T(NE, "!=", 9)                                                     \
  T(EQ_STRICT, "===", 9)                                             \
  T(NE_STRICT, "!==", 9)                                             \
  T(LT, "<", 10)                                                     \
  T(GT, ">", 10)                                                     \
  T(LTE, "<=", 10)                                                   \
  T(GTE, ">=", 10)                                                   \
  K(INSTANCEOF, "instanceof", 10)                                    \
  K(IN, "in", 10)                                                    \
                                                                     \
  /* Unary operators. */                                             \
  /* IsUnaryOp() relies on this block of enum values */              \
  /* being contiguous and sorted in the same order! */               \
  T(NOT, "!", 0)                                                     \
  T(BIT_NOT, "~", 0)                                                 \
  K(DELETE, "delete", 0)                                             \
  K(TYPEOF, "typeof", 0)                                             \
  K(VOID, "void", 0)                                                 \
                                                                     \
  /* Keywords (ECMA-262, section 7.5.2, page 13). */                 \
  K(BREAK, "break", 0)                                               \
  K(CASE, "case", 0)                                                 \
  K(CATCH, "catch", 0)                                               \
  K(CONTINUE, "continue", 0)                                         \
  K(CONTRACT, "contract", 0)                                         \
  K(DEBUGGER, "debugger", 0)                                         \
  K(DEFAULT, "default", 0)                                           \
  /* DELETE */                                                       \
  K(DO, "do", 0)                                                     \
  K(ELSE, "else", 0)                                                 \
  K(FINALLY, "finally", 0)                                           \
  K(FOR, "for", 0)                                                   \
  K(FUNCTION, "function", 0)                                         \
  K(IF, "if", 0)                                                     \
  /* IN */                                                           \
  /* INSTANCEOF */                                                   \
  K(MAPPING, "mapping", 0)                                           \
  K(NEW, "new", 0)                                                   \
  K(PUBLIC, "public", 0)                                             \
  K(PRIVATE, "private", 0)                                           \
  K(RETURN, "return", 0)                                             \
  K(RETURNS, "returns", 0)                                           \
  K(STRUCT, "struct", 0)                                             \
  K(SWITCH, "switch", 0)                                             \
  K(THIS, "this", 0)                                                 \
  K(THROW, "throw", 0)                                               \
  K(TRY, "try", 0)                                                   \
  /* TYPEOF */                                                       \
  K(VAR, "var", 0)                                                   \
  /* VOID */                                                         \
  K(WHILE, "while", 0)                                               \
  K(WITH, "with", 0)                                                 \
                                                                     \
  /* type keywords, keep them in this order, keep int as first keyword TODO more to be added */ \
  K(INT, "int", 0)                                                   \
  K(INT32, "int32", 0)                                               \
  K(INT64, "int64", 0)                                               \
  K(INT128, "int128", 0)                                             \
  K(INT256, "int256", 0)                                             \
  K(UINT, "uint", 0)                                                 \
  K(UINT32, "uint32", 0)                                             \
  K(UINT64, "uint64", 0)                                             \
  K(UINT128, "uint128", 0)                                           \
  K(UINT256, "uint256", 0)                                           \
  K(HASH, "hash", 0)                                                 \
  K(HASH32, "hash32", 0)                                             \
  K(HASH64, "hash64", 0)                                             \
  K(HASH128, "hash128", 0)                                           \
  K(HASH256, "hash256", 0)                                           \
  K(ADDRESS, "address", 0)                                           \
  K(BOOL, "bool", 0)                                                 \
  K(STRING_TYPE, "string", 0)                                        \
  K(TEXT, "text", 0)                                                 \
  K(REAL, "real", 0)                                                 \
  K(UREAL, "ureal", 0)                                               \
  T(TYPES_END, NULL, 0) /* used as type enum end marker */           \
                                                                     \
  /* Literals (ECMA-262, section 7.8, page 16). */                   \
  K(NULL_LITERAL, "null", 0)                                         \
  K(TRUE_LITERAL, "true", 0)                                         \
  K(FALSE_LITERAL, "false", 0)                                       \
  T(NUMBER, NULL, 0)                                                 \
  T(STRING_LITERAL, NULL, 0)                                         \
                                                                     \
  /* Identifiers (not keywords or future reserved words). */         \
  T(IDENTIFIER, NULL, 0)                                             \
                                                                     \
  /* Future reserved words (ECMA-262, section 7.6.1.2). */           \
  T(FUTURE_RESERVED_WORD, NULL, 0)                                   \
  T(FUTURE_STRICT_RESERVED_WORD, NULL, 0)                            \
  K(CLASS, "class", 0)                                               \
  K(CONST, "const", 0)                                               \
  K(EXPORT, "export", 0)                                             \
  K(EXTENDS, "extends", 0)                                           \
  K(IMPORT, "import", 0)                                             \
  K(LET, "let", 0)                                                   \
  K(STATIC, "static", 0)                                             \
/*  K(YIELD, "yield", 0) */                                              \
  K(SUPER, "super", 0)                                               \
                                                                     \
  /* Illegal token - not able to scan. */                            \
  T(ILLEGAL, "ILLEGAL", 0)                                           \
                                                                     \
  /* Scanner-internal use only. */                                   \
  T(WHITESPACE, NULL, 0)


class Token {
 public:
  // All token values.
#define T(name, string, precedence) name,
  enum Value {
    TOKEN_LIST(T, T)
    NUM_TOKENS
  };
#undef T

  // Returns a string corresponding to the C++ token name
  // (e.g. "LT" for the token LT).
  static const char* Name(Value tok) {
    BOOST_ASSERT(tok < NUM_TOKENS);  // tok is unsigned
    return m_name[tok];
  }

  // Predicates
  static bool IsKeyword(Value tok) {
    return m_tokenType[tok] == 'K';
  }

  static bool IsIdentifier(Value tok) {
      return tok == IDENTIFIER;
  }

  static bool IsElementaryTypeName(Value tok) {
      return INT <= tok && tok < TYPES_END;
  }

  static bool IsAssignmentOp(Value tok) {
    return INIT_VAR <= tok && tok <= ASSIGN_MOD;
  }

  static bool IsBinaryOp(Value op) {
    return COMMA <= op && op <= MOD;
  }

  static bool IsTruncatingBinaryOp(Value op) {
    return BIT_OR <= op && op <= ROR;
  }

  static bool IsCompareOp(Value op) {
    return EQ <= op && op <= IN;
  }

  static bool IsOrderedRelationalCompareOp(Value op) {
    return op == LT || op == LTE || op == GT || op == GTE;
  }

  static bool IsEqualityOp(Value op) {
    return op == EQ || op == EQ_STRICT;
  }

  static bool IsInequalityOp(Value op) {
    return op == NE || op == NE_STRICT;
  }

  static bool IsArithmeticCompareOp(Value op) {
    return IsOrderedRelationalCompareOp(op) ||
        IsEqualityOp(op) || IsInequalityOp(op);
  }

  static Value NegateCompareOp(Value op) {
    BOOST_ASSERT(IsArithmeticCompareOp(op));
    switch (op) {
      case EQ: return NE;
      case NE: return EQ;
      case EQ_STRICT: return NE_STRICT;
      case NE_STRICT: return EQ_STRICT;
      case LT: return GTE;
      case GT: return LTE;
      case LTE: return GT;
      case GTE: return LT;
      default:
        BOOST_ASSERT(false); // should not get here
        return op;
    }
  }

  static Value ReverseCompareOp(Value op) {
    BOOST_ASSERT(IsArithmeticCompareOp(op));
    switch (op) {
      case EQ: return EQ;
      case NE: return NE;
      case EQ_STRICT: return EQ_STRICT;
      case NE_STRICT: return NE_STRICT;
      case LT: return GT;
      case GT: return LT;
      case LTE: return GTE;
      case GTE: return LTE;
      default:
        BOOST_ASSERT(false); // should not get here
        return op;
    }
  }

  static bool IsBitOp(Value op) {
    return (BIT_OR <= op && op <= SHR) || op == BIT_NOT;
  }

  static bool IsUnaryOp(Value op) {
    return (NOT <= op && op <= VOID) || op == ADD || op == SUB;
  }

  static bool IsCountOp(Value op) {
    return op == INC || op == DEC;
  }

  static bool IsShiftOp(Value op) {
    return (SHL <= op) && (op <= SHR);
  }

  // Returns a string corresponding to the JS token string
  // (.e., "<" for the token LT) or NULL if the token doesn't
  // have a (unique) string (e.g. an IDENTIFIER).
  static const char* String(Value tok) {
    BOOST_ASSERT(tok < NUM_TOKENS);  // tok is unsigned.
    return m_string[tok];
  }

  // Returns the precedence > 0 for binary and compare
  // operators; returns 0 otherwise.
  static int Precedence(Value tok) {
    BOOST_ASSERT(tok < NUM_TOKENS);  // tok is unsigned.
    return m_precedence[tok];
  }

 private:
  static const char* const m_name[NUM_TOKENS];
  static const char* const m_string[NUM_TOKENS];
  static const int8_t m_precedence[NUM_TOKENS];
  static const char m_tokenType[NUM_TOKENS];
};

} }
