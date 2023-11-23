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

#include <libyul/AsmAnalysisInfo.h>
#include <libyul/AsmAnalysis.h>
#include <libyul/AsmPrinter.h>
#include <libyul/Dialect.h>
#include <libyul/backends/evm/EVMDialect.h>
#include <libyul/YulStack.h>

#include <liblangutil/DebugInfoSelection.h>
#include <liblangutil/Exceptions.h>
#include <liblangutil/EVMVersion.h>

#include <libsolutil/CommonIO.h>
#include <libsolutil/CommonData.h>
#include <libsolutil/StringUtils.h>

#include <test/tools/ossfuzz/yulFuzzerCommon.h>

#include <string>
#include <memory>
#include <iostream>
#include <fstream>
#include <iterator>

using namespace std;
using namespace solidity;
using namespace solidity::yul;
using namespace solidity::util;
using namespace solidity::langutil;
using namespace solidity::yul::test::yul_fuzzer;
using namespace solidity::frontend;

// Prototype as we can't use the FuzzerInterface.h header.
extern "C" int LLVMFuzzerTestOneInput(uint8_t const* _data, size_t _size);

extern "C" int LLVMFuzzerTestOneInput(uint8_t const* _data, size_t _size)
{
	if (_size > 6000)
		return 0;

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

	if (std::any_of(testProgram.begin(), testProgram.end(), [](char c) {
		return ((static_cast<unsigned char>(c) > 127) || !(isPrint(c) || (c == '\n') || (c == '\t')));
	}))
		return 0;

	YulStringRepository::reset();

	cout << "Optimizer sequence: " << optimizerSequence << endl;

	auto settings = solidity::frontend::OptimiserSettings::fuzz(optimizerSequence);
	YulStack stack(
		langutil::EVMVersion(),
		nullopt,
		YulStack::Language::StrictAssembly,
		settings,
		DebugInfoSelection::All()
	);
	try
	{
		if (
			!stack.parseAndAnalyze("source", testProgram) ||
			!stack.parserResult()->code ||
			!stack.parserResult()->analysisInfo
		)
			return 0;
	}
	catch (Exception const&)
	{
		return 0;
	}

	ostringstream os1;
	ostringstream os2;
	// Disable memory tracing to avoid false positive reports
	// such as unused write to memory e.g.,
	// { mstore(0, 1) }
	// that would be removed by the redundant store eliminator.
	yulFuzzerUtil::TerminationReason termReason = yulFuzzerUtil::interpret(
		os1,
		stack.parserResult()->code,
		EVMDialect::strictAssemblyForEVMObjects(langutil::EVMVersion()),
		/*disableMemoryTracing=*/true
	);
	if (yulFuzzerUtil::resourceLimitsExceeded(termReason))
		return 0;

	stack.optimize();
	cout << "------ Optmized Yul code ------" << endl;
	cout << AsmPrinter{}(*stack.parserResult()->code) << endl; 
	termReason = yulFuzzerUtil::interpret(
		os2,
		stack.parserResult()->code,
		EVMDialect::strictAssemblyForEVMObjects(langutil::EVMVersion()),
		/*disableMemoryTracing=*/true
	);

	if (yulFuzzerUtil::resourceLimitsExceeded(termReason))
		return 0;

	bool isTraceEq = (os1.str() == os2.str());
	if (!isTraceEq)
	{
		cout << "------ Unoptimized trace ------" << endl;
		cout << os1.str() << endl;
		cout << "------ Optimized trace ------" << endl;
		cout << os2.str() << endl;
		yulAssert(false, "Interpreted traces for optimized and unoptimized code differ.");
	}
	return 0;
}
