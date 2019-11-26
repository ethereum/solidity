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
/**
 * @author Christian <c@ethdev.com>
 * @date 2014
 * Public compiler API.
 */

#pragma once

#include <stdbool.h>

#ifdef __cplusplus
#define SOLC_NOEXCEPT noexcept
#else
#define SOLC_NOEXCEPT
#endif

#ifdef __cplusplus
extern "C" {
#endif

/// Callback used to retrieve additional source files or data.
///
/// @param _context The readContext passed to solidity_compile. Can be NULL.
/// @param _kind The kind of callback (a string).
/// @param _data The data for the callback.
/// @param o_contents A pointer to the contents of the file, if found.
/// @param o_error A pointer to an error message, if there is one.
///
/// If the callback is not supported, o_contents and o_error should be set to NULL.
///
/// The two pointers (o_contents and o_error) should be heap-allocated and are free'd by the caller.
typedef void (*CStyleReadFileCallback)(void* _context, char const* _kind, char const* _data, char** o_contents, char** o_error);

/// Returns the complete license document.
///
/// The pointer returned must not be freed by the caller.
char const* solidity_license() SOLC_NOEXCEPT;

/// Returns the compiler version.
///
/// The pointer returned must not be freed by the caller.
char const* solidity_version() SOLC_NOEXCEPT;

/// Takes a "Standard Input JSON" and an optional callback (can be set to null). Returns
/// a "Standard Output JSON". Both are to be UTF-8 encoded.
///
/// @param _input The input JSON to process.
/// @param _readCallback The optional callback pointer. Can be NULL.
/// @param _readContext An optional context pointer passed to _readCallback. Can be NULL.
///
/// @returns A pointer to the result. The pointer returned must not be freed by the caller.
char const* solidity_compile(char const* _input, CStyleReadFileCallback _readCallback, void* _readContext) SOLC_NOEXCEPT;

/// Frees up any allocated memory.
///
/// NOTE: the pointer returned by solidity_compile is invalid after calling this!
void solidity_free() SOLC_NOEXCEPT;

#ifdef __cplusplus
}
#endif
