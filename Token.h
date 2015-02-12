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
#include <libsolidity/Utils.h>
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
	T(LParen, "(", 0)                                                  \
	T(RParen, ")", 0)                                                  \
	T(LBrack, "[", 0)                                                  \
	T(RBrack, "]", 0)                                                  \
	T(LBrace, "{", 0)                                                  \
	T(RBrace, "}", 0)                                                  \
	T(Colon, ":", 0)                                                   \
	T(Semicolon, ";", 0)                                               \
	T(Period, ".", 0)                                                  \
	T(Conditional, "?", 3)                                             \
	T(Arrow, "=>", 0)                                                  \
	\
	/* Assignment operators. */                                        \
	/* IsAssignmentOp() relies on this block of enum values being */   \
	/* contiguous and sorted in the same order!*/                      \
	T(Assign, "=", 2)                                                  \
	/* The following have to be in exactly the same order as the simple binary operators*/ \
	T(AssignBitOr, "|=", 2)                                          \
	T(AssignBitXor, "^=", 2)                                         \
	T(AssignBitAnd, "&=", 2)                                         \
	T(AssignShl, "<<=", 2)                                            \
	T(AssignSar, ">>=", 2)                                            \
	T(AssignShr, ">>>=", 2)                                           \
	T(AssignAdd, "+=", 2)                                             \
	T(AssignSub, "-=", 2)                                             \
	T(AssignMul, "*=", 2)                                             \
	T(AssignDiv, "/=", 2)                                             \
	T(AssignMod, "%=", 2)                                             \
	\
	/* Binary operators sorted by precedence. */                       \
	/* IsBinaryOp() relies on this block of enum values */             \
	/* being contiguous and sorted in the same order! */               \
	T(Comma, ",", 1)                                                   \
	T(Or, "||", 4)                                                     \
	T(And, "&&", 5)                                                    \
	T(BitOr, "|", 8)                                                  \
	T(BitXor, "^", 9)                                                 \
	T(BitAnd, "&", 10)                                                \
	T(SHL, "<<", 11)                                                   \
	T(SAR, ">>", 11)                                                   \
	T(SHR, ">>>", 11)                                                  \
	T(Add, "+", 12)                                                    \
	T(Sub, "-", 12)                                                    \
	T(Mul, "*", 13)                                                    \
	T(Div, "/", 13)                                                    \
	T(Mod, "%", 13)                                                    \
	T(Exp, "**", 14)                                                   \
	\
	/* Compare operators sorted by precedence. */                      \
	/* IsCompareOp() relies on this block of enum values */            \
	/* being contiguous and sorted in the same order! */               \
	T(Equal, "==", 6)                                                     \
	T(NotEqual, "!=", 6)                                                     \
	T(LessThan, "<", 7)                                                      \
	T(GreaterThan, ">", 7)                                                      \
	T(LessThanOrEqual, "<=", 7)                                                    \
	T(GreaterThanOrEqual, ">=", 7)                                                    \
	K(In, "in", 7)                                                     \
	\
	/* Unary operators. */                                             \
	/* IsUnaryOp() relies on this block of enum values */              \
	/* being contiguous and sorted in the same order! */               \
	T(Not, "!", 0)                                                     \
	T(BitNot, "~", 0)                                                 \
	T(Inc, "++", 0)                                                    \
	T(Dec, "--", 0)                                                    \
	K(Delete, "delete", 0)                                             \
	\
	/* Keywords */                                                     \
	K(Break, "break", 0)                                               \
	K(Case, "case", 0)                                                 \
	K(Const, "constant", 0)                                            \
	K(Continue, "continue", 0)                                         \
	K(Contract, "contract", 0)                                         \
	K(Default, "default", 0)                                           \
	K(Do, "do", 0)                                                     \
	K(Else, "else", 0)                                                 \
	K(Event, "event", 0)                                               \
	K(Is, "is", 0)                                                     \
	K(Indexed, "indexed", 0)                                           \
	K(For, "for", 0)                                                   \
	K(Function, "function", 0)                                         \
	K(If, "if", 0)                                                     \
	K(Import, "import", 0)                                             \
	K(Mapping, "mapping", 0)                                           \
	K(Modifier, "modifier", 0)                                         \
	K(New, "new", 0)                                                   \
	K(Public, "public", 0)                                             \
	K(Private, "private", 0)                                           \
	K(Protected, "protected", 0)                                       \
	K(Return, "return", 0)                                             \
	K(Returns, "returns", 0)                                           \
	K(Struct, "struct", 0)                                             \
	K(Switch, "switch", 0)                                             \
	K(Var, "var", 0)                                                   \
	K(While, "while", 0)                                               \
	\
	\
	/* Ether subdenominations */                                        \
	K(SubWei, "wei", 0)                                                 \
	K(SubSzabo, "szabo", 0)                                             \
	K(SubFinney, "finney", 0)                                           \
	K(SubEther, "ether", 0)                                             \
	/* type keywords, keep them in this order, keep int as first keyword
	 * the implementation in Types.cpp has to be synced to this here */\
	K(Int, "int", 0)                                                   \
	K(Int8, "int8", 0)                                                 \
	K(Int16, "int16", 0)                                               \
	K(Int24, "int24", 0)                                               \
	K(Int32, "int32", 0)                                               \
	K(Int40, "int40", 0)                                               \
	K(Int48, "int48", 0)                                               \
	K(Int56, "int56", 0)                                               \
	K(Int64, "int64", 0)                                               \
	K(Int72, "int72", 0)                                               \
	K(Int80, "int80", 0)                                               \
	K(Int88, "int88", 0)                                               \
	K(Int96, "int96", 0)                                               \
	K(Int104, "int104", 0)                                             \
	K(Int112, "int112", 0)                                             \
	K(Int120, "int120", 0)                                             \
	K(Int128, "int128", 0)                                             \
	K(Int136, "int136", 0)                                             \
	K(Int144, "int144", 0)                                             \
	K(Int152, "int152", 0)                                             \
	K(Int160, "int160", 0)                                             \
	K(Int168, "int168", 0)                                             \
	K(Int176, "int178", 0)                                             \
	K(Int184, "int184", 0)                                             \
	K(Int192, "int192", 0)                                             \
	K(Int200, "int200", 0)                                             \
	K(Int208, "int208", 0)                                             \
	K(Int216, "int216", 0)                                             \
	K(Int224, "int224", 0)                                             \
	K(Int232, "int232", 0)                                             \
	K(Int240, "int240", 0)                                             \
	K(Int248, "int248", 0)                                             \
	K(Int256, "int256", 0)                                             \
	K(UInt, "uint", 0)                                                 \
	K(UInt8, "uint8", 0)                                               \
	K(UInt16, "uint16", 0)                                             \
	K(UInt24, "uint24", 0)                                             \
	K(UInt32, "uint32", 0)                                             \
	K(UInt40, "uint40", 0)                                             \
	K(UInt48, "uint48", 0)                                             \
	K(UInt56, "uint56", 0)                                             \
	K(UInt64, "uint64", 0)                                             \
	K(UInt72, "uint72", 0)                                             \
	K(UInt80, "uint80", 0)                                             \
	K(UInt88, "uint88", 0)                                             \
	K(UInt96, "uint96", 0)                                             \
	K(UInt104, "uint104", 0)                                           \
	K(UInt112, "uint112", 0)                                           \
	K(UInt120, "uint120", 0)                                           \
	K(UInt128, "uint128", 0)                                           \
	K(UInt136, "uint136", 0)                                           \
	K(UInt144, "uint144", 0)                                           \
	K(UInt152, "uint152", 0)                                           \
	K(UInt160, "uint160", 0)                                           \
	K(UInt168, "uint168", 0)                                           \
	K(UInt176, "uint178", 0)                                           \
	K(UInt184, "uint184", 0)                                           \
	K(UInt192, "uint192", 0)                                           \
	K(UInt200, "uint200", 0)                                           \
	K(UInt208, "uint208", 0)                                           \
	K(UInt216, "uint216", 0)                                           \
	K(UInt224, "uint224", 0)                                           \
	K(UInt232, "uint232", 0)                                           \
	K(UInt240, "uint240", 0)                                           \
	K(UInt248, "uint248", 0)                                           \
	K(UInt256, "uint256", 0)                                           \
	K(Hash, "hash", 0)                                                 \
	K(Hash8, "hash8", 0)                                               \
	K(Hash16, "hash16", 0)                                             \
	K(Hash24, "hash24", 0)                                             \
	K(Hash32, "hash32", 0)                                             \
	K(Hash40, "hash40", 0)                                             \
	K(Hash48, "hash48", 0)                                             \
	K(Hash56, "hash56", 0)                                             \
	K(Hash64, "hash64", 0)                                             \
	K(Hash72, "hash72", 0)                                             \
	K(Hash80, "hash80", 0)                                             \
	K(Hash88, "hash88", 0)                                             \
	K(Hash96, "hash96", 0)                                             \
	K(Hash104, "hash104", 0)                                           \
	K(Hash112, "hash112", 0)                                           \
	K(Hash120, "hash120", 0)                                           \
	K(Hash128, "hash128", 0)                                           \
	K(Hash136, "hash136", 0)                                           \
	K(Hash144, "hash144", 0)                                           \
	K(Hash152, "hash152", 0)                                           \
	K(Hash160, "hash160", 0)                                           \
	K(Hash168, "hash168", 0)                                           \
	K(Hash176, "hash178", 0)                                           \
	K(Hash184, "hash184", 0)                                           \
	K(Hash192, "hash192", 0)                                           \
	K(Hash200, "hash200", 0)                                           \
	K(Hash208, "hash208", 0)                                           \
	K(Hash216, "hash216", 0)                                           \
	K(Hash224, "hash224", 0)                                           \
	K(Hash232, "hash232", 0)                                           \
	K(Hash240, "hash240", 0)                                           \
	K(Hash248, "hash248", 0)                                           \
	K(Hash256, "hash256", 0)                                           \
	K(Address, "address", 0)                                           \
	K(Bool, "bool", 0)                                                 \
	K(Bytes, "bytes", 0)                                               \
	K(StringType, "string", 0)                                         \
	K(String0, "string0", 0)                                           \
	K(String1, "string1", 0)                                           \
	K(String2, "string2", 0)                                           \
	K(String3, "string3", 0)                                           \
	K(String4, "string4", 0)                                           \
	K(String5, "string5", 0)                                           \
	K(String6, "string6", 0)                                           \
	K(String7, "string7", 0)                                           \
	K(String8, "string8", 0)                                           \
	K(String9, "string9", 0)                                           \
	K(String10, "string10", 0)                                         \
	K(String11, "string11", 0)                                         \
	K(String12, "string12", 0)                                         \
	K(String13, "string13", 0)                                         \
	K(String14, "string14", 0)                                         \
	K(String15, "string15", 0)                                         \
	K(String16, "string16", 0)                                         \
	K(String17, "string17", 0)                                         \
	K(String18, "string18", 0)                                         \
	K(String19, "string19", 0)                                         \
	K(String20, "string20", 0)                                         \
	K(String21, "string21", 0)                                         \
	K(String22, "string22", 0)                                         \
	K(String23, "string23", 0)                                         \
	K(String24, "string24", 0)                                         \
	K(String25, "string25", 0)                                         \
	K(String26, "string26", 0)                                         \
	K(String27, "string27", 0)                                         \
	K(String28, "string28", 0)                                         \
	K(String29, "string29", 0)                                         \
	K(String30, "string30", 0)                                         \
	K(String31, "string31", 0)                                         \
	K(String32, "string32", 0)                                         \
	K(Text, "text", 0)                                                 \
	K(Real, "real", 0)                                                 \
	K(UReal, "ureal", 0)                                               \
	T(TypesEnd, NULL, 0) /* used as type enum end marker */           \
	\
	/* Literals */                                                     \
	K(NullLiteral, "null", 0)                                         \
	K(TrueLiteral, "true", 0)                                         \
	K(FalseLiteral, "false", 0)                                       \
	T(Number, NULL, 0)                                                 \
	T(StringLiteral, NULL, 0)                                         \
	T(CommentLiteral, NULL, 0)                                        \
	\
	/* Identifiers (not keywords or future reserved words). */         \
	T(Identifier, NULL, 0)                                             \
	\
	/* Illegal token - not able to scan. */                            \
	T(Illegal, "ILLEGAL", 0)                                           \
	\
	/* Scanner-internal use only. */                                   \
	T(Whitespace, NULL, 0)


class Token
{
public:
	// All token values.
	// attention! msvc issue:
	// http://stackoverflow.com/questions/9567868/compile-errors-after-adding-v8-to-my-project-c2143-c2059
	// @todo: avoid TOKEN_LIST macro
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
		solAssert(tok < NUM_TOKENS, "");
		return m_name[tok];
	}

	// Predicates
	static bool isElementaryTypeName(Value tok) { return Int <= tok && tok < TypesEnd; }
	static bool isAssignmentOp(Value tok) { return Assign <= tok && tok <= AssignMod; }
	static bool isBinaryOp(Value op) { return Comma <= op && op <= Exp; }
	static bool isCommutativeOp(Value op) { return op == BitOr || op == BitXor || op == BitAnd ||
				op == Add || op == Mul || op == Equal || op == NotEqual; }
	static bool isArithmeticOp(Value op) { return Add <= op && op <= Exp; }
	static bool isCompareOp(Value op) { return Equal <= op && op <= In; }

	static Value AssignmentToBinaryOp(Value op)
	{
		solAssert(isAssignmentOp(op) && op != Assign, "");
		return Token::Value(op + (BitOr - AssignBitOr));
	}

	static bool isBitOp(Value op) { return (BitOr <= op && op <= SHR) || op == BitNot; }
	static bool isUnaryOp(Value op) { return (Not <= op && op <= Delete) || op == Add || op == Sub; }
	static bool isCountOp(Value op) { return op == Inc || op == Dec; }
	static bool isShiftOp(Value op) { return (SHL <= op) && (op <= SHR); }
	static bool isVisibilitySpecifier(Value op) { return op == Public || op == Private || op == Protected; }
	static bool isEtherSubdenomination(Value op) { return op == SubWei || op == SubSzabo || op == SubFinney || op == Token::SubEther; }

	// Returns a string corresponding to the JS token string
	// (.e., "<" for the token LT) or NULL if the token doesn't
	// have a (unique) string (e.g. an IDENTIFIER).
	static char const* toString(Value tok)
	{
		solAssert(tok < NUM_TOKENS, "");
		return m_string[tok];
	}

	// Returns the precedence > 0 for binary and compare
	// operators; returns 0 otherwise.
	static int precedence(Value tok)
	{
		solAssert(tok < NUM_TOKENS, "");
		return m_precedence[tok];
	}

	static Token::Value fromIdentifierOrKeyword(std::string const& _name);

private:
	static char const* const m_name[NUM_TOKENS];
	static char const* const m_string[NUM_TOKENS];
	static int8_t const m_precedence[NUM_TOKENS];
	static char const m_tokenType[NUM_TOKENS];
};

}
}
