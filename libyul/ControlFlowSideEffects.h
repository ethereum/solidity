// SPDX-License-Identifier: GPL-3.0

#pragma once

#include <set>

namespace solidity::yul
{

/**
 * Side effects of code related to control flow.
 */
struct ControlFlowSideEffects
{
	/// If true, this code terminates the control flow.
	/// State may or may not be reverted as indicated by the ``reverts`` flag.
	bool terminates = false;
	/// If true, this code reverts all state changes in the transaction.
	/// Whenever this is true, ``terminates`` has to be true as well.
	bool reverts = false;
};

}
