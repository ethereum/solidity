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

#include <libyul/Exceptions.h>

#include <string>
#include <set>

namespace yul
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
	std::string name;
};

template <class Step>
struct OptimiserStepInstance: public OptimiserStep
{
	OptimiserStepInstance(): OptimiserStep{Step::name} {}
	void run(OptimiserStepContext& _context, Block& _ast) const override
	{
		Step::run(_context, _ast);
	}
};


}
