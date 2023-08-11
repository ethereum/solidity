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

#include <test/libsolidity/util/Common.h>

using namespace std;
using namespace solidity;
using namespace solidity::frontend;

string test::withPreamble(string const& _sourceCode, bool _addAbicoderV1Pragma)
{
	static string const versionPragma = "pragma solidity >=0.0;\n";
	static string const licenseComment = "// SPDX-License-Identifier: GPL-3.0\n";
	static string const abicoderPragma = "pragma abicoder v1;\n";

	// NOTE: These checks are intentionally loose to match weird cases.
	// We can manually adjust a test case where this causes problem.
	bool licenseMissing = _sourceCode.find("SPDX-License-Identifier:") == string::npos;
	bool abicoderMissing =
		_sourceCode.find("pragma experimental ABIEncoderV2;") == string::npos &&
		_sourceCode.find("pragma abicoder") == string::npos;

	return
		versionPragma +
		(licenseMissing ? licenseComment : "") +
		(abicoderMissing && _addAbicoderV1Pragma ? abicoderPragma : "") +
		_sourceCode;
}

StringMap test::withPreamble(StringMap _sources, bool _addAbicoderV1Pragma)
{
	for (auto&& [sourceName, source]: _sources)
		source = withPreamble(source, _addAbicoderV1Pragma);

	return _sources;
}
