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

#include <test/tools/ossfuzz/SolImportProtoConverter.h>
#include <test/tools/ossfuzz/solImportProto.pb.h>

#include <test/tools/fuzzer_common.h>

#include <fstream>
#include <src/libfuzzer/libfuzzer_macro.h>

using namespace solidity::test::solimportprotofuzzer;
using namespace std;

DEFINE_PROTO_FUZZER(Test const& _input)
{
	auto sourceMap = ProtoConverter{}.sourceCodeMap(_input);
	if (char const* dump_path = getenv("PROTO_FUZZER_DUMP_PATH"))
	{
		// With libFuzzer binary run this to generate a YUL source file x.yul:
		// PROTO_FUZZER_DUMP_PATH=x.yul ./a.out proto-input
		ofstream of(dump_path);
		for (const auto &[key, value]: sourceMap)
		{
			string header = "==== Source: " + key + " ====\n";
			of.write(header.data(), header.size());
			of.write(value.data(), value.size());
			of.write("\n", 1);
		}
	}
	FuzzerUtil::testCompiler(sourceMap, false, _input.ByteSizeLong());
}
