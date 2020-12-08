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

#include <test/tools/ossfuzz/abiV2FuzzerCommon.h>

#include <test/tools/ossfuzz/protoToAbiV2.h>

#include <src/libfuzzer/libfuzzer_macro.h>
#include <abicoder.hpp>

using namespace solidity::test::abiv2fuzzer;
using namespace solidity::test;
using namespace std;

static constexpr size_t abiCoderHeapSize = 1024 * 512;

DEFINE_PROTO_FUZZER(Contract const&)
{
	abicoder::ABICoder coder(abiCoderHeapSize);
	auto [encodeStatus, encodedData] = coder.encode("bool", "true");
	solAssert(encodeStatus, "Isabelle abicoder fuzzer: Encoding failed");
	return;
}