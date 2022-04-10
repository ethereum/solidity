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

#include "pch.h"
#include <liblangutil/Exceptions.h>

#include <libsolutil/StringUtils.h>

#include <boost/algorithm/string/trim.hpp>

#include <range/v3/range/conversion.hpp>
#include <range/v3/view/map.hpp>
#include <range/v3/view/split.hpp>

#include "StringUtils.h"
#include <optional>
#include <ostream>
#include <string>
#include <string_view>
#include <vector>

namespace solidity::util
{

using namespace std;
using namespace solidity;
using namespace solidity::langutil;

DEFINE_METHOD_CHECK(hasFlagMap, flagMap, void (*)(void));

/**
 * Represents a generalized interface to work with a set of flags
 *
 * Provides extra functionality for enumerating the flags and serializing/deserializing the
 * flags to/from a comma-separated string.
 */
template<typename Derived, std::enable_if_t<hasFlagMap<Derived>::value, bool> = true>
struct FlagSet
{
	static Derived const All(bool _value = true) noexcept
	{
		Derived result;

		for (bool Derived::*flag: Derived::flagMap() | ranges::views::values)
			result.*flag = _value;

		return result;
	}

	static Derived const None(bool _value = true) noexcept { return All(not _value); }

	static Derived const Only(string const& _flag) noexcept
	{
		Derived result;

		auto memberIt = Derived::flagMap().find(boost::trim_copy(_flag));
		if (memberIt == Derived::flagMap().end())
			return result;

		result.*(memberIt->second) = true;
		return result;
	}

	static Derived const Only(bool Derived::*_flag) noexcept
	{
		Derived result{};
		result.*_flag = true;
		return result;
	}

	static Derived Default() noexcept { return All(); }

	static optional<Derived> fromString(string_view _input)
	{
		solAssert(Derived::flagMap().count("all") == 0, "");
		solAssert(Derived::flagMap().count("none") == 0, "");

		if (_input == "all")
			return All();
		if (_input == "none")
			return None();

		return fromFlags(_input | ranges::views::split(',') | ranges::to<vector<string>>);
	}

	static std::string flagName(bool Derived::*flag) noexcept
	{
		solAssert(flag, "");

		for (auto const& [flagName, member]: Derived::flagMap())
			if (flag == member)
				return flagName;

		solAssert(false, "");
	}

	static optional<Derived> fromFlags(vector<string> const& _flagNames, bool _acceptWildcards = false)
	{
		solAssert(Derived::flagMap().count("*") == 0, "");

		Derived selection;
		for (auto const& flag: _flagNames)
		{
			if (flag == "*")
				return (_acceptWildcards ? make_optional(Derived::All()) : nullopt);

			if (!selection.setFlag(flag))
				return nullopt;
		}

		return selection;
	}

	bool setFlag(string const& _flagName, bool _value = true)
	{
		auto memberIt = Derived::flagMap().find(boost::trim_copy(_flagName));
		if (memberIt == Derived::flagMap().end())
			return false;

		Self()->*(memberIt->second) = _value;
		return true;
	}

	bool setFlag(bool Derived::*flag, bool _value = true)
	{
		solAssert(flag, "");

		for (auto const& [flagName, member]: Derived::flagMap())
			if (flag == member)
			{
				Self().*member = _value;
			}

		solAssert(false, "");
	}

	bool all() const noexcept
	{
		for (bool Derived::*member: Derived::flagMap() | ranges::views::values)
			if (!(ConstSelf()->*member))
				return false;

		return true;
	}

	bool any() const noexcept
	{
		for (bool Derived::*member: Derived::flagMap() | ranges::views::values)
			if (ConstSelf()->*member)
				return true;

		return false;
	}

	bool none() const noexcept { return !any(); }

	bool only(bool Derived::*_member) const noexcept { return *ConstSelf() == Only(_member); }

	Derived& operator&=(Derived const& _other)
	{
		for (bool Derived::*member: Derived::flagMap() | ranges::views::values)
			Self()->*member &= _other.*member;
		return *Self();
	}

	Derived& operator|=(Derived const& _other)
	{
		for (bool Derived::*member: Derived::flagMap() | ranges::views::values)
			Self()->*member |= _other.*member;
		return *Self();
	}

	Derived operator&(Derived _other) const noexcept
	{
		_other &= *ConstSelf();
		return _other;
	}

	Derived operator|(Derived _other) const noexcept
	{
		_other |= *ConstSelf();
		return _other;
	}

	bool operator!=(Derived const& _other) const noexcept { return !(*ConstSelf() == _other); }

	bool operator==(Derived const& _other) const noexcept
	{
		for (bool Derived::*member: Derived::flagMap() | ranges::views::values)
			if (ConstSelf()->*member != _other.*member)
				return false;
		return true;
	}

	friend ostream& operator<<(ostream& _stream, Derived const& _selection)
	{
		vector<string> selectedFlagNames;
		for (auto const& [name, member]: _selection.flagMap())
			if (_selection.*member)
				selectedFlagNames.push_back(name);

		return _stream << joinHumanReadable(selectedFlagNames, ",");
	}

private:
	Derived* Self() noexcept { return static_cast<Derived*>(this); }

	Derived const* ConstSelf() const noexcept { return static_cast<Derived const*>(this); }
};
}
