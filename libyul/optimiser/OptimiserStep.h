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

#include <libyul/Exceptions.h>

#include <libsolutil/Common.h>

#include <optional>
#include <string>
#include <set>

namespace solidity::yul
{

struct Dialect;
struct Block;
class YulString;
class NameDispenser;

struct OptimiserStepContext
{
	Dialect const& dialect;
	NameDispenser& dispenser;
	std::set<YulString> const& reservedIdentifiers;
	/// The value nullopt represents creation code
	std::optional<size_t> expectedExecutionsPerDeployment;
	std::shared_ptr<u256> externalFreeMemoryPointerInitializer{};
};


/**
 * Construction to create dynamically callable objects out of the
 * statically callable optimiser steps.
 */
struct OptimiserStep
{
	explicit OptimiserStep(std::string _name): name(std::move(_name)) {}
	virtual ~OptimiserStep() = default;

	virtual void run(OptimiserStepContext&, Block&) const = 0;
	/// @returns non-nullopt if the step cannot be run, for example because it requires
	/// an SMT solver to be loaded, but none is available. In that case, the string
	/// contains a human-readable reason.
	virtual std::optional<std::string> invalidInCurrentEnvironment() const = 0;
	std::string name;
};

template <class Step>
struct OptimiserStepInstance: public OptimiserStep
{
private:
	template<typename T>
	struct HasInvalidInCurrentEnvironmentMethod
	{
	private:
		template<typename U> static auto test(int) -> decltype(U::invalidInCurrentEnvironment(), std::true_type());
		template<typename> static std::false_type test(...);

	public:
		static constexpr bool value = decltype(test<T>(0))::value;
	};

public:
	OptimiserStepInstance(): OptimiserStep{Step::name} {}
	void run(OptimiserStepContext& _context, Block& _ast) const override
	{
		Step::run(_context, _ast);
	}
	std::optional<std::string> invalidInCurrentEnvironment() const override
	{
		if constexpr (HasInvalidInCurrentEnvironmentMethod<Step>::value)
			return Step::invalidInCurrentEnvironment();
		else
			return std::nullopt;
	}
};


}
