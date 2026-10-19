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

/// Sequences of Yul optimizer steps for property tests

#pragma once

#include <libyul/AST.h>

#include <fuzztest/fuzztest.h>

#include <cstddef>
#include <string>

namespace solidity::yul
{
class Object;
}

namespace solidity::yul::test
{

/// Any sequence of up to @a _maxSize optimizer steps, as their abbreviations.
fuzztest::Domain<std::string> stepsDomain(std::size_t _maxSize);

/// @returns the steps that have to run before @a _steps, in the order they have to run in. Only those that
/// @a _steps need, so that the other steps also get to run on code that is not hoisted, not grouped or has loop
/// initializers.
std::string prerequisitesOf(std::string const& _steps);

/// @returns the code of @a _object, disambiguated and with @a _steps run on it.
Block runSteps(Object const& _object, std::string const& _steps);

}
