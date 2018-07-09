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
 * JSON interface for the solidity compiler to be used from Javascript.
 */

#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif

/// Callback used to retrieve additional source files. "Returns" two pointers that should be
/// heap-allocated and are free'd by the caller.
typedef void (*CStyleReadFileCallback)(char const* _path, char** o_contents, char** o_error);

char const* license();
char const* version();
char const* compileJSON(char const* _input, bool _optimize);
char const* compileJSONMulti(char const* _input, bool _optimize);
char const* compileJSONCallback(char const* _input, bool _optimize, CStyleReadFileCallback _readCallback);
char const* compileStandard(char const* _input, CStyleReadFileCallback _readCallback);

char const* solidity_license();
char const* solidity_version();
char const* solidity_compile(char const* _input, CStyleReadFileCallback _readCallback);

#ifdef __cplusplus
}
#endif
