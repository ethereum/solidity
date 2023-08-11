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

/// Utilities shared by multiple libsolidity tests.

#include <libsolidity/interface/CompilerStack.h>

#include <string>

namespace solidity::frontend::test
{

/// @returns @p _sourceCode prefixed with the version pragma and the SPDX license identifier.
/// Can optionally also insert an abicoder pragma when missing.
std::string withPreamble(std::string const& _sourceCode, bool _addAbicoderV1Pragma = false);

/// @returns a copy of @p _sources with preamble prepended to all sources.
StringMap withPreamble(StringMap _sources, bool _addAbicoderV1Pragma = false);

std::string stripPreReleaseWarning(std::string const& _stderrContent);

} // namespace solidity::frontend::test
