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

#include <test/tools/fuzzer_common.h>

#include <test/TestCaseReader.h>

#include <sstream>

using namespace solidity::frontend::test;
using namespace std;

// Prototype as we can't use the FuzzerInterface.h header.
extern "C" int LLVMFuzzerTestOneInput(uint8_t const* _data, size_t _size);

extern "C" int LLVMFuzzerTestOneInput(uint8_t const* _data, size_t _size)
{
	if (_size <= 600)
	{
		string input(reinterpret_cast<char const*>(_data), _size);
		map<string, string> sourceCode;
		try
		{
			TestCaseReader t = TestCaseReader(std::istringstream(input));
			sourceCode = t.sources().sources;
			map<string, string> settings = t.settings();
			bool compileViaYul =
				settings.count("compileViaYul") &&
				(settings.at("compileViaYul") == "also" || settings.at("compileViaYul") == "true");
			bool optimize = settings.count("optimize") && settings.at("optimize") == "true";
			FuzzerUtil::testCompiler(
				sourceCode,
				optimize,
				/*_rand=*/static_cast<unsigned>(_size),
				/*forceSMT=*/true,
				compileViaYul
			);
		}
		catch (runtime_error const&)
		{
			return 0;
		}
	}
	return 0;
}
