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
#pragma once

#include <string>

namespace solidity::util
{

enum class PanicCode
{
	Generic = 0x00, // generic / unspecified error.
	Assert = 0x01, // generic / unspecified error. Used by assert().
	UnderOverflow = 0x11, // arithmetic underflow or overflow
	DivisionByZero = 0x12, // division or modulo by zero
	EnumConversionError = 0x21, // enum conversion error
	StorageEncodingError = 0x22, // invalid encoding in storage
	EmptyArrayPop = 0x31, // empty array pop
	ArrayOutOfBounds = 0x32, // array out of bounds access
	ResourceError = 0x41, // resource error (too large allocation or too large array)
	InvalidInternalFunction = 0x51, // calling invalid internal function
};

}
