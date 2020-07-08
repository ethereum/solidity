// SPDX-License-Identifier: GPL-3.0
#pragma once

#include <libyul/optimiser/ASTWalker.h>
#include <libyul/optimiser/OptimiserStep.h>

namespace solidity::yul
{

class BlockFlattener: public ASTModifier
{
public:
	static constexpr char const* name{"BlockFlattener"};
	static void run(OptimiserStepContext&, Block& _ast) { BlockFlattener{}(_ast); }

	using ASTModifier::operator();
	void operator()(Block& _block) override;

private:
	BlockFlattener() = default;
};

}
