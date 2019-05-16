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

#include <libdevcore/CommonIO.h>
#include <libdevcore/CommonData.h>

#include <test/tools/ossfuzz/yulFuzzerCommon.h>

#include <string>
#include <memory>
#include <iostream>

using namespace yul;
using namespace std;

using namespace langutil;
using namespace dev;
using namespace yul::test::yul_fuzzer;

extern "C" int LLVMFuzzerTestOneInput(uint8_t const* _data, size_t _size)
{
	if (_size > 600)
		return 0;

	string input(reinterpret_cast<char const*>(_data), _size);

	if (std::any_of(input.begin(), input.end(), [](char c) {
		return ((static_cast<unsigned char>(c) > 127) || !(std::isprint(c) || (c == '\n') || (c == '\t')));
	}))
		return 0;

	AssemblyStack stack(
		langutil::EVMVersion(),
		AssemblyStack::Language::StrictAssembly,
		dev::solidity::OptimiserSettings::full()
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
	try
	{
		yulFuzzerUtil::interpret(
			os1,
			stack.parserResult()->code,
			EVMDialect::strictAssemblyForEVMObjects(langutil::EVMVersion())
		);
	}
	catch (yul::test::StepLimitReached const&)
	{
		return 0;
	}
	catch (yul::test::InterpreterTerminatedGeneric const&)
	{
	}

	stack.optimize();
	try
	{
		yulFuzzerUtil::interpret(
			os2,
			stack.parserResult()->code,
			EVMDialect::strictAssemblyForEVMObjects(langutil::EVMVersion()),
			(yul::test::yul_fuzzer::yulFuzzerUtil::maxSteps * 1.5)
		);
	}
	catch (yul::test::InterpreterTerminatedGeneric const&)
	{
	}

	bool isTraceEq = (os1.str() == os2.str());
	yulAssert(isTraceEq, "Interpreted traces for optimized and unoptimized code differ.");
	return 0;
}
