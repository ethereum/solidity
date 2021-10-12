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

#include <libsolidity/formal/ModelCheckerSettings.h>

#include <optional>
#include <range/v3/view.hpp>

using namespace std;
using namespace solidity;
using namespace solidity::frontend;

map<string, InvariantType> const ModelCheckerInvariants::validInvariants{
	{"contract", InvariantType::Contract},
	{"reentrancy", InvariantType::Reentrancy}
};

std::optional<ModelCheckerInvariants> ModelCheckerInvariants::fromString(string const& _invs)
{
	set<InvariantType> chosenInvs;
	if (_invs == "default")
	{
		// The default is that no invariants are reported.
	}
	else if (_invs == "all")
		for (auto&& v: validInvariants | ranges::views::values)
			chosenInvs.insert(v);
	else
		for (auto&& t: _invs | ranges::views::split(',') | ranges::to<vector<string>>())
		{
			if (!validInvariants.count(t))
				return {};
			chosenInvs.insert(validInvariants.at(t));
		}

	return ModelCheckerInvariants{chosenInvs};
}

bool ModelCheckerInvariants::setFromString(string const& _inv)
{
	if (!validInvariants.count(_inv))
		return false;
	invariants.insert(validInvariants.at(_inv));
	return true;
}

using TargetType = VerificationTargetType;
map<string, TargetType> const ModelCheckerTargets::targetStrings{
	{"constantCondition", TargetType::ConstantCondition},
	{"underflow", TargetType::Underflow},
	{"overflow", TargetType::Overflow},
	{"divByZero", TargetType::DivByZero},
	{"balance", TargetType::Balance},
	{"assert", TargetType::Assert},
	{"popEmptyArray", TargetType::PopEmptyArray},
	{"outOfBounds", TargetType::OutOfBounds}
};

map<TargetType, string> const ModelCheckerTargets::targetTypeToString{
	{TargetType::ConstantCondition, "Constant condition"},
	{TargetType::Underflow, "Underflow"},
	{TargetType::Overflow, "Overflow"},
	{TargetType::DivByZero, "Division by zero"},
	{TargetType::Balance, "Insufficient balance"},
	{TargetType::Assert, "Assertion failed"},
	{TargetType::PopEmptyArray, "Empty array pop"},
	{TargetType::OutOfBounds, "Out of bounds access"}
};

std::optional<ModelCheckerTargets> ModelCheckerTargets::fromString(string const& _targets)
{
	set<TargetType> chosenTargets;
	if (_targets == "default" || _targets == "all")
	{
		bool all = _targets == "all";
		for (auto&& v: targetStrings | ranges::views::values)
		{
			if (!all && (v == TargetType::Underflow || v == TargetType::Overflow))
				continue;
			chosenTargets.insert(v);
		}
	}
	else
		for (auto&& t: _targets | ranges::views::split(',') | ranges::to<vector<string>>())
		{
			if (!targetStrings.count(t))
				return {};
			chosenTargets.insert(targetStrings.at(t));
		}

	return ModelCheckerTargets{chosenTargets};
}

bool ModelCheckerTargets::setFromString(string const& _target)
{
	if (!targetStrings.count(_target))
		return false;
	targets.insert(targetStrings.at(_target));
	return true;
}

std::optional<ModelCheckerContracts> ModelCheckerContracts::fromString(string const& _contracts)
{
	map<string, set<string>> chosen;
	if (_contracts == "default")
		return ModelCheckerContracts::Default();

	for (auto&& sourceContract: _contracts | ranges::views::split(',') | ranges::to<vector<string>>())
	{
		auto&& names = sourceContract | ranges::views::split(':') | ranges::to<vector<string>>();
		if (names.size() != 2 || names.at(0).empty() || names.at(1).empty())
			return {};
		chosen[names.at(0)].insert(names.at(1));
	}

	return ModelCheckerContracts{chosen};
}

std::optional<ModelCheckerExtCalls> ModelCheckerExtCalls::fromString(string const& _mode)
{
	if (_mode == "untrusted")
		return ModelCheckerExtCalls{Mode::UNTRUSTED};
	if (_mode == "trusted")
		return ModelCheckerExtCalls{Mode::TRUSTED};
	return {};
}
