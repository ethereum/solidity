// SPDX-License-Identifier: GPL-3.0

#include <fstream>

#include <test/tools/ossfuzz/yulProto.pb.h>
#include <test/tools/fuzzer_common.h>
#include <test/tools/ossfuzz/protoToYul.h>
#include <src/libfuzzer/libfuzzer_macro.h>

#include <libyul/AssemblyStack.h>
#include <libyul/backends/evm/EVMDialect.h>
#include <libyul/Exceptions.h>

#include <liblangutil/EVMVersion.h>
#include <liblangutil/SourceReferenceFormatter.h>

#include <test/tools/ossfuzz/yulFuzzerCommon.h>

using namespace std;
using namespace solidity;
using namespace solidity::util;
using namespace solidity::langutil;
using namespace solidity::yul;
using namespace solidity::yul::test;
using namespace solidity::yul::test::yul_fuzzer;

namespace
{
void printErrors(ostream& _stream, ErrorList const& _errors)
{
	SourceReferenceFormatter formatter(_stream);

	for (auto const& error: _errors)
		formatter.printExceptionInformation(
			*error,
			(error->type() == Error::Type::Warning) ? "Warning" : "Error"
		);
}
}

DEFINE_PROTO_FUZZER(Program const& _input)
{
	ProtoConverter converter;
	string yul_source = converter.programToString(_input);
	EVMVersion version = converter.version();

	if (const char* dump_path = getenv("PROTO_FUZZER_DUMP_PATH"))
	{
		// With libFuzzer binary run this to generate a YUL source file x.yul:
		// PROTO_FUZZER_DUMP_PATH=x.yul ./a.out proto-input
		ofstream of(dump_path);
		of.write(yul_source.data(), yul_source.size());
	}

	YulStringRepository::reset();

	// AssemblyStack entry point
	AssemblyStack stack(
		version,
		AssemblyStack::Language::StrictAssembly,
		solidity::frontend::OptimiserSettings::full()
	);

	// Parse protobuf mutated YUL code
	if (
		!stack.parseAndAnalyze("source", yul_source) ||
		!stack.parserResult()->code ||
		!stack.parserResult()->analysisInfo ||
		!Error::containsOnlyWarnings(stack.errors())
	)
	{
		printErrors(std::cout, stack.errors());
		yulAssert(false, "Proto fuzzer generated malformed program");
	}

	ostringstream os1;
	ostringstream os2;
	yulFuzzerUtil::TerminationReason termReason = yulFuzzerUtil::interpret(
		os1,
		stack.parserResult()->code,
		EVMDialect::strictAssemblyForEVMObjects(version)
	);

	if (termReason == yulFuzzerUtil::TerminationReason::StepLimitReached)
		return;

	stack.optimize();
	termReason = yulFuzzerUtil::interpret(
		os2,
		stack.parserResult()->code,
		EVMDialect::strictAssemblyForEVMObjects(version),
		(yul::test::yul_fuzzer::yulFuzzerUtil::maxSteps * 4)
	);

	bool isTraceEq = (os1.str() == os2.str());
	yulAssert(isTraceEq, "Interpreted traces for optimized and unoptimized code differ.");
	return;
}
