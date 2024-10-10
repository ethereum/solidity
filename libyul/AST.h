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
 * @author Christian <c@ethdev.com>
 * @date 2016
 * Parsed inline assembly to be used by the AST
 */

#pragma once

#include <libyul/ASTForward.h>
#include <libyul/YulName.h>

#include <liblangutil/DebugData.h>

#include <libsolutil/Numeric.h>

#include <memory>
#include <optional>

namespace solidity::yul
{

struct NameWithDebugData { langutil::DebugData::ConstPtr debugData; YulName name; };
using NameWithDebugDataList = std::vector<NameWithDebugData>;

/// Literal number or string (up to 32 bytes)
enum class LiteralKind { Number, Boolean, String };
/// Literal value that holds a u256 word of data, can be of LiteralKind type and - in case of arguments to
/// builtins - exceed the u256 word (32 bytes), in which case the value is stored as string. The former is constructed
/// via u256 word and optional hint and leads to unlimited == false, the latter is
/// constructed via the string constructor and leads to unlimited == true.
class LiteralValue {
public:
	using Data = u256;
	using BuiltinStringLiteralData = std::string;
	using RepresentationHint = std::shared_ptr<std::string>;

	LiteralValue() = default;
	explicit LiteralValue(std::string _builtinStringLiteralValue);
	explicit LiteralValue(Data const& _data, std::optional<std::string> const& _hint = std::nullopt);

	bool operator==(LiteralValue const& _rhs) const;
	bool operator<(LiteralValue const& _rhs) const;
	Data const& value() const;
	BuiltinStringLiteralData const& builtinStringLiteralValue() const;
	bool unlimited() const;
	RepresentationHint const& hint() const;

private:
	std::optional<Data> m_numericValue;
	std::shared_ptr<std::string> m_stringValue;
};
struct Literal { langutil::DebugData::ConstPtr debugData; LiteralKind kind; LiteralValue value; };
/// External / internal identifier or label reference
struct Identifier { langutil::DebugData::ConstPtr debugData; YulName name; };
/// Assignment ("x := mload(20:u256)", expects push-1-expression on the right hand
/// side and requires x to occupy exactly one stack slot.
///
/// Multiple assignment ("x, y := f()"), where the left hand side variables each occupy
/// a single stack slot and expects a single expression on the right hand returning
/// the same amount of items as the number of variables.
struct Assignment { langutil::DebugData::ConstPtr debugData; std::vector<Identifier> variableNames; std::unique_ptr<Expression> value; };
struct FunctionCall { langutil::DebugData::ConstPtr debugData; Identifier functionName; std::vector<Expression> arguments; };
/// Statement that contains only a single expression
struct ExpressionStatement { langutil::DebugData::ConstPtr debugData; Expression expression; };
/// Block-scope variable declaration ("let x:u256 := mload(20:u256)"), non-hoisted
struct VariableDeclaration { langutil::DebugData::ConstPtr debugData; NameWithDebugDataList variables; std::unique_ptr<Expression> value; };
/// Block that creates a scope (frees declared stack variables)
struct Block { langutil::DebugData::ConstPtr debugData; std::vector<Statement> statements; };
/// Function definition ("function f(a, b) -> (d, e) { ... }")
struct FunctionDefinition { langutil::DebugData::ConstPtr debugData; YulName name; NameWithDebugDataList parameters; NameWithDebugDataList returnVariables; Block body; };
/// Conditional execution without "else" part.
struct If { langutil::DebugData::ConstPtr debugData; std::unique_ptr<Expression> condition; Block body; };
/// Switch case or default case
struct Case { langutil::DebugData::ConstPtr debugData; std::unique_ptr<Literal> value; Block body; };
/// Switch statement
struct Switch { langutil::DebugData::ConstPtr debugData; std::unique_ptr<Expression> expression; std::vector<Case> cases; };
struct ForLoop { langutil::DebugData::ConstPtr debugData; Block pre; std::unique_ptr<Expression> condition; Block post; Block body; };
/// Break statement (valid within for loop)
struct Break { langutil::DebugData::ConstPtr debugData; };
/// Continue statement (valid within for loop)
struct Continue { langutil::DebugData::ConstPtr debugData; };
/// Leave statement (valid within function)
struct Leave { langutil::DebugData::ConstPtr debugData; };

/// Immutable AST comprised of its top-level block
class AST
{
public:
	explicit AST(Block _root): m_root(std::move(_root)) {}

	[[nodiscard]] Block const& root() const { return m_root; }
private:
	Block m_root;
};


/// Extracts the IR source location from a Yul node.
template <class T> inline langutil::SourceLocation nativeLocationOf(T const& _node)
{
	return _node.debugData ? _node.debugData->nativeLocation : langutil::SourceLocation{};
}

/// Extracts the IR source location from a Yul node.
template <class... Args> inline langutil::SourceLocation nativeLocationOf(std::variant<Args...> const& _node)
{
	return std::visit([](auto const& _arg) { return nativeLocationOf(_arg); }, _node);
}

/// Extracts the original source location from a Yul node.
template <class T> inline langutil::SourceLocation originLocationOf(T const& _node)
{
	return _node.debugData ? _node.debugData->originLocation : langutil::SourceLocation{};
}

/// Extracts the original source location from a Yul node.
template <class... Args> inline langutil::SourceLocation originLocationOf(std::variant<Args...> const& _node)
{
	return std::visit([](auto const& _arg) { return originLocationOf(_arg); }, _node);
}

/// Extracts the debug data from a Yul node.
template <class T> inline langutil::DebugData::ConstPtr debugDataOf(T const& _node)
{
	return _node.debugData;
}

/// Extracts the debug data from a Yul node.
template <class... Args> inline langutil::DebugData::ConstPtr debugDataOf(std::variant<Args...> const& _node)
{
	return std::visit([](auto const& _arg) { return debugDataOf(_arg); }, _node);
}

inline bool hasDefaultCase(Switch const& _switch)
{
	return std::any_of(
		_switch.cases.begin(),
		_switch.cases.end(),
		[](Case const& _case) { return !_case.value; }
	);
}

}
