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

#pragma once

#include <libsolidity/analysis/OverrideChecker.h>

#include <libsolidity/ast/AST.h>

#include <regex>

namespace solidity::tools
{

/**
 * Helper that provides functions which analyze certain source locations
 * on a textual base. They utilize regular expression to search for
 * keywords or to determine formatting.
 */
class SourceAnalysis {
public:
	static bool isMultilineKeyword(
		langutil::SourceLocation const& _location,
		std::string const& _keyword
	)
	{
		return regex_search(
			_location.text(),
			std::regex{"(\\b" + _keyword + "\\b\\n|\\r|\\r\\n)"}
		);
	}

	static bool hasMutabilityKeyword(langutil::SourceLocation const& _location)
	{
		return regex_search(
			_location.text(),
			std::regex{"(\\b(pure|view|nonpayable|payable)\\b)"}
		);
	}

	static bool hasVirtualKeyword(langutil::SourceLocation const& _location)
	{
		return regex_search(_location.text(), std::regex{"(\\b(virtual)\\b)"});
	}

	static bool hasVisibilityKeyword(langutil::SourceLocation const& _location)
	{
		return regex_search(_location.text(), std::regex{"\\bpublic\\b"});
	}
};

/**
 * Helper that provides functions which can analyse declarations and
 * generate source snippets based on the information retrieved.
 */
class SourceGeneration
{
public:
	using CompareFunction = frontend::OverrideChecker::CompareByID;
	using Contracts = std::set<frontend::ContractDefinition const*, CompareFunction>;

	/// Generates an `override` declaration for single overrides
	/// or `override(...)` with contract list for multiple overrides.
	static std::string functionOverride(Contracts const& _contracts)
	{
		if (_contracts.size() <= 1)
			return "override";

		std::string overrideList;
		for (auto inheritedContract: _contracts)
			overrideList += inheritedContract->name() + ",";

		return "override(" + overrideList.substr(0, overrideList.size() - 1) + ")";
	}
};

/**
 * Helper that provides functions which apply changes to Solidity source code
 * on a textual base. In general, these utilize regular expressions applied
 * to the given source location.
 */
class SourceTransform
{
public:
	/// Searches for the keyword given and prepends the expression.
	/// E.g. `function f() view;` -> `function f() public view;`
	static std::string insertBeforeKeyword(
		langutil::SourceLocation const& _location,
		std::string const& _keyword,
		std::string const& _expression
	)
	{
		return regex_replace(
			_location.text(),
			std::regex{"(\\b" + _keyword + "\\b)"},
			_expression + " " + _keyword
		);
	}

	/// Searches for the keyword given and appends the expression.
	/// E.g. `function f() public {}` -> `function f() public override {}`
	static std::string insertAfterKeyword(
		langutil::SourceLocation const& _location,
		std::string const& _keyword,
		std::string const& _expression
	)
	{
		bool isMultiline = SourceAnalysis::isMultilineKeyword(_location, _keyword);
		std::string toAppend = isMultiline ? ("\n        " + _expression) : (" " + _expression);
		std::regex keyword{"(\\b" + _keyword + "\\b)"};

		return regex_replace(_location.text(), keyword, _keyword + toAppend);
	}

	/// Searches for the first right parenthesis and appends the expression
	/// given.
	/// E.g. `function f() {}` -> `function f() public {}`
	static std::string insertAfterRightParenthesis(
		langutil::SourceLocation const& _location,
		std::string const& _expression
	)
	{
		return regex_replace(
			_location.text(),
			std::regex{"(\\))"},
			") " + _expression
		);
	}

	/// Searches for the `function` keyword and its identifier and replaces
	/// both by the expression given.
	/// E.g. `function Storage() {}` -> `constructor() {}`
	static std::string replaceFunctionName(
		langutil::SourceLocation const& _location,
		std::string const& _name,
		std::string const& _expression
	)
	{
		return regex_replace(
			_location.text(),
			std::regex{"(\\bfunction\\s*" + _name + "\\b)"},
			_expression
		);
	}
};

}
