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
using namespace ranges;
using namespace solidity;
using namespace solidity::frontend;

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

std::optional<ModelCheckerTargets> ModelCheckerTargets::fromString(string const& _targets)
{
	set<TargetType> chosenTargets;
	if (_targets == "default")
		for (auto&& v: targetStrings | views::values)
			chosenTargets.insert(v);
	else
		for (auto&& t: _targets | views::split(',') | ranges::to<vector<string>>())
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
