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

#include <libsolidity/interface/OptimiserSettings.h>

#include <fstream>
#include <iterator>
#include <sstream>

using namespace solidity::frontend::test;
using namespace solidity::frontend;
using namespace std;

// Prototype as we can't use the FuzzerInterface.h header.
extern "C" int LLVMFuzzerTestOneInput(uint8_t const* _data, size_t _size);

extern "C" int LLVMFuzzerTestOneInput(uint8_t const* _data, size_t _size)
{
	if (_size <= 6000)
	{
		string optimizerSequence;
		string testProgram;
		// Fuzz sequence
		if (const char* crashFileEnv = std::getenv("CRASH_FILE"))
		{
			std::ifstream ifs(crashFileEnv);
			testProgram = std::string(std::istreambuf_iterator<char>{ifs}, {});
			auto fuzzedSequence = string(reinterpret_cast<char const*>(_data), _size);
			auto cleanedSequence = OptimiserSettings::removeInvalidCharacters(fuzzedSequence);
			optimizerSequence = OptimiserSettings::createValidSequence(cleanedSequence);
		}
		// Fuzz program
		else if (const char* optSeqEnv = std::getenv("OPT_SEQ"))
		{
			optimizerSequence = optSeqEnv;
			testProgram = string(reinterpret_cast<char const*>(_data), _size);
		}
		// Fuzz program and sequence
		else
		{
			testProgram = string(reinterpret_cast<char const*>(_data), _size);
			optimizerSequence = OptimiserSettings::randomYulOptimiserSequence(_size);
		}

		map<string, string> sourceCode;
		try
		{
			TestCaseReader t = TestCaseReader(std::istringstream(testProgram));
			sourceCode = t.sources().sources;
			cout << optimizerSequence << endl;
			FuzzerUtil::testCompiler(
				sourceCode,
				/*optimize=*/true,
				/*forceSMT=*/false,
				/*compileViaYul=*/_size % 2 == 0,
				/*yulOptimizerSteps=*/optimizerSequence
			);
		}
		catch (runtime_error const&)
		{
			return 0;
		}
	}
	return 0;
}
