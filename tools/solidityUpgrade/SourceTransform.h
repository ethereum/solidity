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

#pragma once

#include <libsolidity/analysis/OverrideChecker.h>

#include <libsolidity/ast/AST.h>

#include <liblangutil/CharStreamProvider.h>
#include <liblangutil/Scanner.h>

#include <sstream>
#include <regex>

namespace solidity::tools
{

/**
 * Helper for displaying location during asserts
 */
class LocationHelper
{
	std::stringstream m_stream;

public:

	template <typename T>
	LocationHelper& operator<<(T const& data)
	{
		m_stream << data;
		return *this;
	}

	operator std::string() { return m_stream.str(); }
};

class SourceTextRetriever
{
public:
	explicit SourceTextRetriever(langutil::CharStreamProvider const& _charStreamProvider):
		m_charStreamProvider(_charStreamProvider) {}
	std::string text(langutil::SourceLocation const& _location) const
	{
		solAssert(_location.hasText(), "");
		return std::string{m_charStreamProvider.charStream(*_location.sourceName).text(_location)};
	}
protected:
	langutil::CharStreamProvider const& m_charStreamProvider;
};

/**
 * Helper that provides functions which analyze certain source locations
 * on a textual base. They utilize regular expression to search for
 * keywords or to determine formatting.
 */
class SourceAnalysis: public SourceTextRetriever
{
public:
	using SourceTextRetriever::SourceTextRetriever;

	bool isMultilineKeyword(
		langutil::SourceLocation const& _location,
		std::string const& _keyword
	) const
	{
		return regex_search(
			text(_location),
			std::regex{"(\\b" + _keyword + "\\b\\n|\\r|\\r\\n)"}
		);
	}

	bool hasMutabilityKeyword(langutil::SourceLocation const& _location) const
	{
		return regex_search(
			text(_location),
			std::regex{"(\\b(pure|view|nonpayable|payable)\\b)"}
		);
	}

	bool hasVirtualKeyword(langutil::SourceLocation const& _location) const
	{
		return regex_search(text(_location), std::regex{"(\\b(virtual)\\b)"});
	}

	bool hasVisibilityKeyword(langutil::SourceLocation const& _location) const
	{
		return regex_search(text(_location), std::regex{"\\bpublic\\b"});
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

		// Note: Can create incorrect replacements
		return "override(" + overrideList.substr(0, overrideList.size() - 1) + ")";
	}
};

/**
 * Helper that provides functions which apply changes to Solidity source code
 * on a textual base. In general, these utilize regular expressions applied
 * to the given source location.
 */
class SourceTransform: public SourceTextRetriever
{
public:
	using SourceTextRetriever::SourceTextRetriever;

	/// Searches for the keyword given and prepends the expression.
	/// E.g. `function f() view;` -> `function f() public view;`
	std::string insertBeforeKeyword(
		langutil::SourceLocation const& _location,
		std::string const& _keyword,
		std::string const& _expression
	) const
	{
		auto _regex = std::regex{"(\\b" + _keyword + "\\b)"};
		if (regex_search(text(_location), _regex))
			return regex_replace(
				text(_location),
				_regex,
				_expression + " " + _keyword,
				std::regex_constants::format_first_only
			);
		else
			solAssert(
				false,
				LocationHelper() << "Could not fix: " << text(_location) << " at " << _location <<
				"\nNeeds to be fixed manually."
			);

		return "";
	}

	/// Searches for the keyword given and appends the expression.
	/// E.g. `function f() public {}` -> `function f() public override {}`
	std::string insertAfterKeyword(
		langutil::SourceLocation const& _location,
		std::string const& _keyword,
		std::string const& _expression
	) const
	{
		bool isMultiline = SourceAnalysis{m_charStreamProvider}.isMultilineKeyword(_location, _keyword);
		std::string toAppend = isMultiline ? ("\n        " + _expression) : (" " + _expression);
		std::regex keyword{"(\\b" + _keyword + "\\b)"};

		if (regex_search(text(_location), keyword))
			return regex_replace(text(_location), keyword, _keyword + toAppend);
		else
			solAssert(
				false,
				LocationHelper() << "Could not fix: " << text(_location) << " at " << _location <<
				"\nNeeds to be fixed manually."
			);

		return "";

	}

	/// Searches for the first right parenthesis and appends the expression
	/// given.
	/// E.g. `function f() {}` -> `function f() public {}`
	std::string insertAfterRightParenthesis(
		langutil::SourceLocation const& _location,
		std::string const& _expression
	) const
	{
		auto _regex = std::regex{"(\\))"};
		if (regex_search(text(_location), _regex))
			return regex_replace(
				text(_location),
				std::regex{"(\\))"},
				") " + _expression
			);
		else
			solAssert(
				false,
				LocationHelper() << "Could not fix: " << text(_location) << " at " << _location <<
				"\nNeeds to be fixed manually."
			);

		return "";
	}

	/// Searches for the `function` keyword and its identifier and replaces
	/// both by the expression given.
	/// E.g. `function Storage() {}` -> `constructor() {}`
	std::string replaceFunctionName(
		langutil::SourceLocation const& _location,
		std::string const& _name,
		std::string const& _expression
	) const
	{
		auto _regex = std::regex{ "(\\bfunction\\s*" + _name + "\\b)"};
		if (regex_search(text(_location), _regex))
			return regex_replace(
				text(_location),
				_regex,
				_expression
			);
		else
			solAssert(
				false,
				LocationHelper() << "Could not fix: " << text(_location) << " at " << _location <<
				"\nNeeds to be fixed manually."
			);

		return "";
	}

	std::string gasUpdate(langutil::SourceLocation const& _location) const
	{
		// dot, "gas", any number of whitespaces, left bracket
		std::regex gasReg{"\\.gas\\s*\\("};

		if (regex_search(text(_location), gasReg))
		{
			std::string out = regex_replace(
				text(_location),
				gasReg,
				"{gas: ",
				std::regex_constants::format_first_only
			);
			return regex_replace(out, std::regex{"\\)$"}, "}");
		}
		else
			solAssert(
				false,
				LocationHelper() << "Could not fix: " << text(_location) << " at " << _location <<
				"\nNeeds to be fixed manually."
			);

		return "";
	}

	std::string valueUpdate(langutil::SourceLocation const& _location) const
	{
		// dot, "value", any number of whitespaces, left bracket
		std::regex valueReg{"\\.value\\s*\\("};

		if (regex_search(text(_location), valueReg))
		{
			std::string out = regex_replace(
					text(_location),
					valueReg,
					"{value: ",
					std::regex_constants::format_first_only
			);
			return regex_replace(out, std::regex{"\\)$"}, "}");
		}
		else
			solAssert(
				false,
				LocationHelper() << "Could not fix: " << text(_location) << " at " << _location <<
				"\nNeeds to be fixed manually."
			);

		return "";
	}

	std::string nowUpdate(langutil::SourceLocation const& _location) const
	{
		return regex_replace(text(_location), std::regex{"now"}, "block.timestamp");
	}

	std::string removeVisibility(langutil::SourceLocation const& _location) const
	{
		std::string replacement = text(_location);
		for (auto const& replace: {"public ", "public", "internal ", "internal", "external ", "external"})
			replacement = regex_replace(replacement, std::regex{replace}, "");
		return replacement;
	}
};

}
