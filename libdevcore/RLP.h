/*
	This file is part of cpp-ethereum.

	cpp-ethereum is free software: you can redistribute it and/or modify
	it under the terms of the GNU General Public License as published by
	the Free Software Foundation, either version 3 of the License, or
	(at your option) any later version.

	cpp-ethereum is distributed in the hope that it will be useful,
	but WITHOUT ANY WARRANTY; without even the implied warranty of
	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
	GNU General Public License for more details.

	You should have received a copy of the GNU General Public License
	along with cpp-ethereum.  If not, see <http://www.gnu.org/licenses/>.
*/
/** @file RLP.h
 * @author Gav Wood <i@gavwood.com>
 * @date 2014
 *
 * RLP (de-)serialisation.
 */

#pragma once

#include "Common.h"

namespace dev
{
	/// Export a list of items in RLP format, returning a byte array.
	bytes rlpList();
}
