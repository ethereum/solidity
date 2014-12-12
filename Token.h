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

#include <libdevcore/Common.h>
#include <libdevcore/Log.h>
#include <libsolidity/Exceptions.h>

namespace dev
{
namespace solidity
{

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

#define TOKEN_LIST(T, K)                                               \
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
	T(ARROW, "=>", 0)                                                  \
	\
	/* Assignment operators. */                                        \
	/* IsAssignmentOp() relies on this block of enum values being */   \
	/* contiguous and sorted in the same order!*/                      \
	T(ASSIGN, "=", 2)                                                  \
	/* The following have to be in exactly the same order as the simple binary operators*/ \
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
	T(LT, "<", 10)                                                     \
	T(GT, ">", 10)                                                     \
	T(LTE, "<=", 10)                                                   \
	T(GTE, ">=", 10)                                                   \
	K(IN, "in", 10)                                                    \
	\
	/* Unary operators. */                                             \
	/* IsUnaryOp() relies on this block of enum values */              \
	/* being contiguous and sorted in the same order! */               \
	T(NOT, "!", 0)                                                     \
	T(BIT_NOT, "~", 0)                                                 \
	T(INC, "++", 0)                                                    \
	T(DEC, "--", 0)                                                    \
	K(DELETE, "delete", 0)                                             \
	\
	/* Keywords */                                                     \
	K(BREAK, "break", 0)                                               \
	K(CASE, "case", 0)                                                 \
	K(CONST, "const", 0)                                               \
	K(CONTINUE, "continue", 0)                                         \
	K(CONTRACT, "contract", 0)                                         \
	K(DEFAULT, "default", 0)                                           \
	K(DO, "do", 0)                                                     \
	K(ELSE, "else", 0)                                                 \
	K(EXTENDS, "extends", 0)                                           \
	K(FOR, "for", 0)                                                   \
	K(FUNCTION, "function", 0)                                         \
	K(IF, "if", 0)                                                     \
	K(IMPORT, "import", 0)                                             \
	K(MAPPING, "mapping", 0)                                           \
	K(NEW, "new", 0)                                                   \
	K(PUBLIC, "public", 0)                                             \
	K(PRIVATE, "private", 0)                                           \
	K(RETURN, "return", 0)                                             \
	K(RETURNS, "returns", 0)                                           \
	K(STRUCT, "struct", 0)                                             \
	K(SWITCH, "switch", 0)                                             \
	K(VAR, "var", 0)                                                   \
	K(WHILE, "while", 0)                                               \
	\
	\
	/* type keywords, keep them in this order, keep int as first keyword
	 * the implementation in Types.cpp has to be synced to this here
	 *  TODO more to be added */                                       \
	K(INT, "int", 0)                                                   \
	K(INT8, "int8", 0)                                                 \
	K(INT16, "int16", 0)                                               \
	K(INT24, "int24", 0)                                               \
	K(INT32, "int32", 0)                                               \
	K(INT40, "int40", 0)                                               \
	K(INT48, "int48", 0)                                               \
	K(INT56, "int56", 0)                                               \
	K(INT64, "int64", 0)                                               \
	K(INT72, "int72", 0)                                               \
	K(INT80, "int80", 0)                                               \
	K(INT88, "int88", 0)                                               \
	K(INT96, "int96", 0)                                               \
	K(INT104, "int104", 0)                                             \
	K(INT112, "int112", 0)                                             \
	K(INT120, "int120", 0)                                             \
	K(INT128, "int128", 0)                                             \
	K(INT136, "int136", 0)                                             \
	K(INT144, "int144", 0)                                             \
	K(INT152, "int152", 0)                                             \
	K(INT160, "int160", 0)                                             \
	K(INT168, "int168", 0)                                             \
	K(INT176, "int178", 0)                                             \
	K(INT184, "int184", 0)                                             \
	K(INT192, "int192", 0)                                             \
	K(INT200, "int200", 0)                                             \
	K(INT208, "int208", 0)                                             \
	K(INT216, "int216", 0)                                             \
	K(INT224, "int224", 0)                                             \
	K(INT232, "int232", 0)                                             \
	K(INT240, "int240", 0)                                             \
	K(INT248, "int248", 0)                                             \
	K(INT256, "int256", 0)                                             \
	K(UINT, "uint", 0)                                                 \
	K(UINT8, "uint8", 0)                                               \
	K(UINT16, "uint16", 0)                                             \
	K(UINT24, "uint24", 0)                                             \
	K(UINT32, "uint32", 0)                                             \
	K(UINT40, "uint40", 0)                                             \
	K(UINT48, "uint48", 0)                                             \
	K(UINT56, "uint56", 0)                                             \
	K(UINT64, "uint64", 0)                                             \
	K(UINT72, "uint72", 0)                                             \
	K(UINT80, "uint80", 0)                                             \
	K(UINT88, "uint88", 0)                                             \
	K(UINT96, "uint96", 0)                                             \
	K(UINT104, "uint104", 0)                                           \
	K(UINT112, "uint112", 0)                                           \
	K(UINT120, "uint120", 0)                                           \
	K(UINT128, "uint128", 0)                                           \
	K(UINT136, "uint136", 0)                                           \
	K(UINT144, "uint144", 0)                                           \
	K(UINT152, "uint152", 0)                                           \
	K(UINT160, "uint160", 0)                                           \
	K(UINT168, "uint168", 0)                                           \
	K(UINT176, "uint178", 0)                                           \
	K(UINT184, "uint184", 0)                                           \
	K(UINT192, "uint192", 0)                                           \
	K(UINT200, "uint200", 0)                                           \
	K(UINT208, "uint208", 0)                                           \
	K(UINT216, "uint216", 0)                                           \
	K(UINT224, "uint224", 0)                                           \
	K(UINT232, "uint232", 0)                                           \
	K(UINT240, "uint240", 0)                                           \
	K(UINT248, "uint248", 0)                                           \
	K(UINT256, "uint256", 0)                                           \
	K(HASH, "hash", 0)                                                 \
	K(HASH8, "hash8", 0)                                               \
	K(HASH16, "hash16", 0)                                             \
	K(HASH24, "hash24", 0)                                             \
	K(HASH32, "hash32", 0)                                             \
	K(HASH40, "hash40", 0)                                             \
	K(HASH48, "hash48", 0)                                             \
	K(HASH56, "hash56", 0)                                             \
	K(HASH64, "hash64", 0)                                             \
	K(HASH72, "hash72", 0)                                             \
	K(HASH80, "hash80", 0)                                             \
	K(HASH88, "hash88", 0)                                             \
	K(HASH96, "hash96", 0)                                             \
	K(HASH104, "hash104", 0)                                           \
	K(HASH112, "hash112", 0)                                           \
	K(HASH120, "hash120", 0)                                           \
	K(HASH128, "hash128", 0)                                           \
	K(HASH136, "hash136", 0)                                           \
	K(HASH144, "hash144", 0)                                           \
	K(HASH152, "hash152", 0)                                           \
	K(HASH160, "hash160", 0)                                           \
	K(HASH168, "hash168", 0)                                           \
	K(HASH176, "hash178", 0)                                           \
	K(HASH184, "hash184", 0)                                           \
	K(HASH192, "hash192", 0)                                           \
	K(HASH200, "hash200", 0)                                           \
	K(HASH208, "hash208", 0)                                           \
	K(HASH216, "hash216", 0)                                           \
	K(HASH224, "hash224", 0)                                           \
	K(HASH232, "hash232", 0)                                           \
	K(HASH240, "hash240", 0)                                           \
	K(HASH248, "hash248", 0)                                           \
	K(HASH256, "hash256", 0)                                           \
	K(ADDRESS, "address", 0)                                           \
	K(BOOL, "bool", 0)                                                 \
	K(STRING_TYPE, "string", 0)                                        \
	K(STRING0, "string0", 0)                                           \
	K(STRING1, "string1", 0)                                           \
	K(STRING2, "string2", 0)                                           \
	K(STRING3, "string3", 0)                                           \
	K(STRING4, "string4", 0)                                           \
	K(STRING5, "string5", 0)                                           \
	K(STRING6, "string6", 0)                                           \
	K(STRING7, "string7", 0)                                           \
	K(STRING8, "string8", 0)                                           \
	K(STRING9, "string9", 0)                                           \
	K(STRING10, "string10", 0)                                         \
	K(STRING11, "string11", 0)                                         \
	K(STRING12, "string12", 0)                                         \
	K(STRING13, "string13", 0)                                         \
	K(STRING14, "string14", 0)                                         \
	K(STRING15, "string15", 0)                                         \
	K(STRING16, "string16", 0)                                         \
	K(STRING17, "string17", 0)                                         \
	K(STRING18, "string18", 0)                                         \
	K(STRING19, "string19", 0)                                         \
	K(STRING20, "string20", 0)                                         \
	K(STRING21, "string21", 0)                                         \
	K(STRING22, "string22", 0)                                         \
	K(STRING23, "string23", 0)                                         \
	K(STRING24, "string24", 0)                                         \
	K(STRING25, "string25", 0)                                         \
	K(STRING26, "string26", 0)                                         \
	K(STRING27, "string27", 0)                                         \
	K(STRING28, "string28", 0)                                         \
	K(STRING29, "string29", 0)                                         \
	K(STRING30, "string30", 0)                                         \
	K(STRING31, "string31", 0)                                         \
	K(STRING32, "string32", 0)                                         \
	K(TEXT, "text", 0)                                                 \
	K(REAL, "real", 0)                                                 \
	K(UREAL, "ureal", 0)                                               \
	T(TYPES_END, NULL, 0) /* used as type enum end marker */           \
	\
	/* Literals */                                                     \
	K(NULL_LITERAL, "null", 0)                                         \
	K(TRUE_LITERAL, "true", 0)                                         \
	K(FALSE_LITERAL, "false", 0)                                       \
	T(NUMBER, NULL, 0)                                                 \
	T(STRING_LITERAL, NULL, 0)                                         \
	T(COMMENT_LITERAL, NULL, 0)                                        \
	\
	/* Identifiers (not keywords or future reserved words). */         \
	T(IDENTIFIER, NULL, 0)                                             \
	\
	/* Illegal token - not able to scan. */                            \
	T(ILLEGAL, "ILLEGAL", 0)                                           \
	\
	/* Scanner-internal use only. */                                   \
	T(WHITESPACE, NULL, 0)


class Token
{
public:
	// All token values.
#define T(name, string, precedence) name,
	enum Value
	{
		TOKEN_LIST(T, T)
		NUM_TOKENS
	};
#undef T

	// Returns a string corresponding to the C++ token name
	// (e.g. "LT" for the token LT).
	static char const* getName(Value tok)
	{
		if (asserts(tok < NUM_TOKENS))
			BOOST_THROW_EXCEPTION(InternalCompilerError());
		return m_name[tok];
	}

	// Predicates
	static bool isElementaryTypeName(Value tok) { return INT <= tok && tok < TYPES_END; }
	static bool isAssignmentOp(Value tok) { return ASSIGN <= tok && tok <= ASSIGN_MOD; }
	static bool isBinaryOp(Value op) { return COMMA <= op && op <= MOD; }
	static bool isArithmeticOp(Value op) { return ADD <= op && op <= MOD; }
	static bool isCompareOp(Value op) { return EQ <= op && op <= IN; }

	static Value AssignmentToBinaryOp(Value op)
	{
		if (asserts(isAssignmentOp(op) && op != ASSIGN))
			BOOST_THROW_EXCEPTION(InternalCompilerError());
		return Token::Value(op + (BIT_OR - ASSIGN_BIT_OR));
	}

	static bool isBitOp(Value op) { return (BIT_OR <= op && op <= SHR) || op == BIT_NOT; }
	static bool isUnaryOp(Value op) { return (NOT <= op && op <= DELETE) || op == ADD || op == SUB; }
	static bool isCountOp(Value op) { return op == INC || op == DEC; }
	static bool isShiftOp(Value op) { return (SHL <= op) && (op <= SHR); }

	// Returns a string corresponding to the JS token string
	// (.e., "<" for the token LT) or NULL if the token doesn't
	// have a (unique) string (e.g. an IDENTIFIER).
	static char const* toString(Value tok)
	{
		if (asserts(tok < NUM_TOKENS))
			BOOST_THROW_EXCEPTION(InternalCompilerError());
		return m_string[tok];
	}

	// Returns the precedence > 0 for binary and compare
	// operators; returns 0 otherwise.
	static int precedence(Value tok)
	{
		if (asserts(tok < NUM_TOKENS))
			BOOST_THROW_EXCEPTION(InternalCompilerError());
		return m_precedence[tok];
	}

private:
	static char const* const m_name[NUM_TOKENS];
	static char const* const m_string[NUM_TOKENS];
	static int8_t const m_precedence[NUM_TOKENS];
	static char const m_tokenType[NUM_TOKENS];
};

}
}
