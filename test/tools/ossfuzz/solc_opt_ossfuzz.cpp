// SPDX-License-Identifier: GPL-3.0

#include <test/tools/fuzzer_common.h>

using namespace std;

extern "C" int LLVMFuzzerTestOneInput(uint8_t const* _data, size_t _size)
{
	if (_size <= 600)
	{
		string input(reinterpret_cast<char const *>(_data), _size);
		FuzzerUtil::testCompiler(input, /*optimize=*/true);
	}
	return 0;
}
