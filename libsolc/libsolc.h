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
/**
 * @author Christian <c@ethdev.com>
 * @date 2014
 * Public compiler API.
 */

#pragma once

#include <stdbool.h>
#include <stddef.h>

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
/// @param _data The data for the callback (a string).
/// @param o_contents A pointer to the contents of the file, if found. Allocated via solidity_alloc().
/// @param o_error A pointer to an error message, if there is one.
///
/// The file (as well as error) contents that is to be allocated by the callback
/// implementor must use the solidity_alloc() API to allocate its underlying
/// storage. Ownership is then transferred to the compiler which will take care
/// of the deallocation.
///
/// If the callback is not supported, *o_contents and *o_error must be set to NULL.
typedef void (*CStyleReadFileCallback)(void* _context, char const* _kind, char const* _data, char** o_contents, char** o_error);

/// Returns the complete license document.
///
/// The pointer returned must NOT be freed by the caller.
char const* solidity_license() SOLC_NOEXCEPT;

/// Returns the compiler version.
///
/// The pointer returned must NOT be freed by the caller.
char const* solidity_version() SOLC_NOEXCEPT;

/// Allocates a chunk of memory of @p _size bytes.
///
/// Use this function inside callbacks to allocate data that is to be passed to
/// the compiler. You may use solidity_free() or solidity_reset() to free this
/// memory again but it is not required as the compiler takes ownership for any
/// data passed to it via callbacks.
///
/// This function will return NULL if the requested memory region could not be allocated.
char* solidity_alloc(size_t _size) SOLC_NOEXCEPT;

/// Explicitly frees the memory (@p _data) that was being allocated with solidity_alloc()
/// or returned by a call to solidity_compile().
///
/// Important, this call will abort() in case of any invalid argument being passed to this call.
void solidity_free(char* _data) SOLC_NOEXCEPT;

/// Takes a "Standard Input JSON" and an optional callback (can be set to null). Returns
/// a "Standard Output JSON". Both are to be UTF-8 encoded.
///
/// @param _input The input JSON to process.
/// @param _readCallback The optional callback pointer. Can be NULL, but if not NULL,
///                      it can be called by the compiler to request additional input.
///                      Please see the documentation of the type for details.
/// @param _readContext An optional context pointer passed to _readCallback. Can be NULL.
///
/// @returns A pointer to the result. The pointer returned must be freed by the caller using solidity_free() or solidity_reset().
char* solidity_compile(char const* _input, CStyleReadFileCallback _readCallback, void* _readContext) SOLC_NOEXCEPT;

/// @TODO the biggest problem here is that the callback has to be synchronous.
/// With "compile" we would just record requested files and trigger a recompile later,
/// so we might need a way to trigger a recompile.

/// Switch into LSP mode. Can be undone using solidity_reset().
///
/// @returns 0 on success and -1 on failure. A failure can only happen due to mis-use of the API,
/// such as, the LSP mode has been already initiated.
int solidity_lsp_start(CStyleReadFileCallback _readCallback, void* _readContext) SOLC_NOEXCEPT;

/// Sends a single JSON-RPC message to the LSP server.
/// This message must not include any HTTP headers but only hold the payload.
///
/// @retval 0 Success.
/// @retval -1 Server not initialized.
/// @retval -2 Server not running (e.g. termination initiated).
/// @retval -3 Could not parse JSON RPC message.
int solidity_lsp_send(char const* _input) SOLC_NOEXCEPT;

/// Tries to pop a pending message from the LSP server.
/// @returns either an empty string if not possible
/// (e.g. no message available or server shut down) or a stringified JSON response message.
char const* solidity_try_receive() SOLC_NOEXCEPT;

/// Send one or more JSON-RPC messages to the LSP (including the HTTP headers),
/// expecting a response.
/// If the input is empty, just checks for a pending response.
/// @returns JSON-RPC message (inculding HTTP headers), can be empty (or nullptr).
/// If the result is not null, it has to be freed by the caller using solidity_free.
///
/// This can cause the callback provided in solidity_lsp to be invoked.
/// Should only be called after having called solidity_lsp.
char const *solidity_lsp_send_receive(char const* _input) SOLC_NOEXCEPT;

/// Frees up any allocated memory.
///
/// NOTE: the pointer returned by solidity_compile as well as any other pointer retrieved via solidity_alloc()
/// is invalid after calling this!
void solidity_reset() SOLC_NOEXCEPT;

#ifdef __cplusplus
}
#endif
