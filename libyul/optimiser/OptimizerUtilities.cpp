// SPDX-License-Identifier: GPL-3.0
/**
 * Some useful snippets for the optimiser.
 */

#include <libyul/optimiser/OptimizerUtilities.h>

#include <libyul/AsmData.h>

#include <libsolutil/CommonData.h>

#include <boost/range/algorithm_ext/erase.hpp>

using namespace std;
using namespace solidity;
using namespace solidity::util;

void yul::removeEmptyBlocks(Block& _block)
{
	auto isEmptyBlock = [](Statement const& _st) -> bool {
		return holds_alternative<Block>(_st) && std::get<Block>(_st).statements.empty();
	};
	boost::range::remove_erase_if(_block.statements, isEmptyBlock);
}
