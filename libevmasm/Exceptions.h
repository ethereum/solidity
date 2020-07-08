// SPDX-License-Identifier: GPL-3.0
/** @file Exceptions.h
 * @author Christian <c@ethdev.com>
 * @date 2014
 */

#pragma once

#include <libsolutil/Exceptions.h>

namespace solidity::evmasm
{

struct AssemblyException: virtual util::Exception {};
struct OptimizerException: virtual AssemblyException {};
struct StackTooDeepException: virtual OptimizerException {};
struct ItemNotAvailableException: virtual OptimizerException {};

DEV_SIMPLE_EXCEPTION(InvalidDeposit);
DEV_SIMPLE_EXCEPTION(InvalidOpcode);

}
