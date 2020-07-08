// SPDX-License-Identifier: GPL-3.0

#pragma once

#include <libyul/Exceptions.h>

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
