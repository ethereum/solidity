// SPDX-License-Identifier: GPL-3.0

#include <libyul/AsmAnalysisInfo.h>
#include <libyul/AsmParser.h>
#include <libyul/AsmAnalysis.h>
#include <libyul/Dialect.h>
#include <libyul/backends/evm/EVMDialect.h>
#include <libyul/AssemblyStack.h>

#include <liblangutil/Exceptions.h>
#include <liblangutil/ErrorReporter.h>
#include <liblangutil/EVMVersion.h>
#include <liblangutil/SourceReferenceFormatter.h>

#include <libsolutil/CommonIO.h>
#include <libsolutil/CommonData.h>

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

extern "C" int LLVMFuzzerTestOneInput(uint8_t const* _data, size_t _size)
{
	if (_size > 600)
		return 0;

	string input(reinterpret_cast<char const*>(_data), _size);

	if (std::any_of(input.begin(), input.end(), [](char c) {
		return ((static_cast<unsigned char>(c) > 127) || !(std::isprint(c) || (c == '\n') || (c == '\t')));
	}))
		return 0;

	YulStringRepository::reset();

	AssemblyStack stack(
		langutil::EVMVersion(),
		AssemblyStack::Language::StrictAssembly,
		solidity::frontend::OptimiserSettings::full()
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
	yulFuzzerUtil::TerminationReason termReason = yulFuzzerUtil::interpret(
		os1,
		stack.parserResult()->code,
		EVMDialect::strictAssemblyForEVMObjects(langutil::EVMVersion())
	);
	if (termReason == yulFuzzerUtil::TerminationReason::StepLimitReached)
		return 0;

	stack.optimize();
	termReason = yulFuzzerUtil::interpret(
		os2,
		stack.parserResult()->code,
		EVMDialect::strictAssemblyForEVMObjects(langutil::EVMVersion()),
		(yul::test::yul_fuzzer::yulFuzzerUtil::maxSteps * 4)
	);

	bool isTraceEq = (os1.str() == os2.str());
	yulAssert(isTraceEq, "Interpreted traces for optimized and unoptimized code differ.");
	return 0;
}
