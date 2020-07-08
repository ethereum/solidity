// SPDX-License-Identifier: GPL-3.0
/**
 * Small useful snippets for the optimiser.
 */

#pragma once

#include <libsolutil/Common.h>
#include <libyul/AsmDataForward.h>

namespace solidity::yul
{

/// Removes statements that are just empty blocks (non-recursive).
void removeEmptyBlocks(Block& _block);

}
