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

using namespace std;
using namespace solidity;
using namespace solidity::yul;
using namespace solidity::util;
using namespace solidity::langutil;
using namespace solidity::yul::test::yul_fuzzer;

// Prototype as we can't use the FuzzerInterface.h header.
extern "C" int LLVMFuzzerTestOneInput(uint8_t const* _data, size_t _size);

extern "C" int LLVMFuzzerTestOneInput(uint8_t const* _data, size_t _size)
{
	if (_size > 600)
		return 0;

	string input(reinterpret_cast<char const*>(_data), _size);

	if (std::any_of(input.begin(), input.end(), [](char c) {
		return ((static_cast<unsigned char>(c) > 127) || !(isPrint(c) || (c == '\n') || (c == '\t')));
	}))
		return 0;

	YulStringRepository::reset();

	YulStack stack(
		langutil::EVMVersion(),
		nullopt,
		YulStack::Language::StrictAssembly,
		solidity::frontend::OptimiserSettings::full(),
		DebugInfoSelection::All()
	);
	try
	{
		if (
			!stack.parseAndAnalyze("source", input) ||
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
	termReason = yulFuzzerUtil::interpret(
		os2,
		stack.parserResult()->code,
		EVMDialect::strictAssemblyForEVMObjects(langutil::EVMVersion()),
		/*disableMemoryTracing=*/true
	);

	if (yulFuzzerUtil::resourceLimitsExceeded(termReason))
		return 0;

	bool isTraceEq = (os1.str() == os2.str());
	yulAssert(isTraceEq, "Interpreted traces for optimized and unoptimized code differ.");
	return 0;
}
