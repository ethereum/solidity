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
/** @file UndefMacros.h
 * @author Lefteris  <lefteris@ethdev.com>
 * @date 2015
 *
 * This header should be used to #undef some really evil macros defined by
 * windows.h which result in conflict with our Token.h
 */
#pragma once

#if defined(_MSC_VER) || defined(__MINGW32__)

#undef DELETE
#undef IN
#undef VOID
#undef THIS
#undef CONST

// Conflicting define on MinGW in windows.h
// windows.h(19): #define interface struct
#ifdef interface
#undef interface
#endif

#elif defined(DELETE) || defined(IN) || defined(VOID) || defined(THIS) || defined(CONST) || defined(interface)

#error "The preceding macros in this header file are reserved for V8's "\
"TOKEN_LIST. Please add a platform specific define above to undefine "\
"overlapping macros."

#endif
