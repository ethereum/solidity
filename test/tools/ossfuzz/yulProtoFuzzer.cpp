// SPDX-License-Identifier: GPL-3.0

#include <fstream>

#include <test/tools/ossfuzz/yulProto.pb.h>
#include <test/tools/fuzzer_common.h>
#include <test/tools/ossfuzz/protoToYul.h>
#include <src/libfuzzer/libfuzzer_macro.h>

#include <libyul/AssemblyStack.h>
#include <liblangutil/EVMVersion.h>
#include <libyul/Exceptions.h>

using namespace solidity;
using namespace solidity::yul;
using namespace solidity::yul::test::yul_fuzzer;
using namespace solidity::langutil;
using namespace std;

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

	if (yul_source.size() > 1200)
		return;

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
		yulAssert(false, "Proto fuzzer generated malformed program");

	// Optimize
	stack.optimize();
}
