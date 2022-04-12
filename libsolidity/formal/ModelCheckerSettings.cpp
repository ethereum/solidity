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
#include <cstdarg>
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

ModelCheckerTargets::ModelCheckerTargets(size_t nFlags, ...)
{
	if (nFlags < 1)
		return;

	std::va_list args{};
	va_start(args, nFlags);

	for (size_t i = 0; i < nFlags; ++i)
	{
		auto targetType = va_arg(args, VerificationTargetType);
		setTargetType(targetType, true);
	}

	va_end(args);
}

bool ModelCheckerTargets::setTargetType(VerificationTargetType targetType, bool _value)
{
	switch (targetType)
	{
	case VerificationTargetType::ConstantCondition:
		constantCondition = _value;
		return true;
	case VerificationTargetType::Underflow:
		underflow = _value;
		return true;
	case VerificationTargetType::Overflow:
		overflow = _value;
		return true;
	case VerificationTargetType::UnderOverflow:
		underOverflow = _value;
		return true;
	case VerificationTargetType::DivByZero:
		divByZero = _value;
		return true;
	case VerificationTargetType::Balance:
		balance = _value;
		return true;
	case VerificationTargetType::Assert:
		assert = _value;
		return true;
	case VerificationTargetType::PopEmptyArray:
		popEmptyArray = _value;
		return true;
	case VerificationTargetType::OutOfBounds:
		outOfBounds = _value;
		return true;
	default:
		return false;
	}
}

bool ModelCheckerTargets::has(VerificationTargetType _type) const
{
	switch (_type)
	{
	case VerificationTargetType::ConstantCondition:
		return constantCondition;
	case VerificationTargetType::Underflow:
		return underflow;
	case VerificationTargetType::Overflow:
		return overflow;
	case VerificationTargetType::UnderOverflow:
		return underOverflow;
	case VerificationTargetType::DivByZero:
		return divByZero;
	case VerificationTargetType::Balance:
		return balance;
	case VerificationTargetType::Assert:
		return assert;
	case VerificationTargetType::PopEmptyArray:
		return popEmptyArray;
	case VerificationTargetType::OutOfBounds:
		return outOfBounds;
	default:
		return false;
	}
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
