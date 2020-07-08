// SPDX-License-Identifier: GPL-3.0
/**
 * Changes the topmost block to be a function with a specific name ("main") which has no
 * inputs nor outputs.
 */

#pragma once

#include <libyul/AsmDataForward.h>

namespace solidity::yul
{

struct OptimiserStepContext;

/**
 * Prerequisites: Function Grouper
 */
class MainFunction
{
public:
	static constexpr char const* name{"MainFunction"};
	static void run(OptimiserStepContext&, Block& _ast) { MainFunction{}(_ast); }

	void operator()(Block& _block);

private:
	MainFunction() = default;
};

}
