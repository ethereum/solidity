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

#include <libsmtutil/SolverInterface.h>

#include <optional>
#include <set>

namespace solidity::frontend
{

struct ModelCheckerEngine
{
	bool bmc = false;
	bool chc = false;

	static constexpr ModelCheckerEngine All() { return {true, true}; }
	static constexpr ModelCheckerEngine BMC() { return {true, false}; }
	static constexpr ModelCheckerEngine CHC() { return {false, true}; }
	static constexpr ModelCheckerEngine None() { return {false, false}; }

	bool none() const { return !any(); }
	bool any() const { return bmc || chc; }
	bool all() const { return bmc && chc; }

	static std::optional<ModelCheckerEngine> fromString(std::string const& _engine)
	{
		static std::map<std::string, ModelCheckerEngine> engineMap{
			{"all", All()},
			{"bmc", BMC()},
			{"chc", CHC()},
			{"none", None()}
		};
		if (engineMap.count(_engine))
			return engineMap.at(_engine);
		return {};
	}
};

enum class VerificationTargetType { ConstantCondition, Underflow, Overflow, UnderOverflow, DivByZero, Balance, Assert, PopEmptyArray, OutOfBounds };

struct ModelCheckerTargets
{
	static ModelCheckerTargets All() { return *fromString("all"); }
	static ModelCheckerTargets None() { return {}; }

	static std::optional<ModelCheckerTargets> fromString(std::string const& _targets);

	bool has(VerificationTargetType _type) const { return targets.count(_type); }
	std::set<VerificationTargetType> targets;
};

struct ModelCheckerSettings
{
	ModelCheckerEngine engine = ModelCheckerEngine::None();
	ModelCheckerTargets targets = ModelCheckerTargets::All();
	std::optional<unsigned> timeout;
};

}
