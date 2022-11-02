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
/** @file Exceptions.h
 * @author Christian <c@ethdev.com>
 * @date 2014
 */

#pragma once

#include <libsolutil/Exceptions.h>

namespace solidity::evmasm
{

struct AssemblyException: virtual util::Exception {};
struct AssemblyImportException: virtual AssemblyException {};
struct OptimizerException: virtual AssemblyException {};
struct StackTooDeepException: virtual OptimizerException {};
struct ItemNotAvailableException: virtual OptimizerException {};

DEV_SIMPLE_EXCEPTION(InvalidDeposit);
DEV_SIMPLE_EXCEPTION(InvalidOpcode);

}
