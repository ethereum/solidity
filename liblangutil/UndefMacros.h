// SPDX-License-Identifier: GPL-3.0
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
