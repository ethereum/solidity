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

#include <test/tools/fuzzer_common.h>

#include <test/tools/ossfuzz/SolidityConstMutations.h>

#include <test/TestCaseReader.h>

#include <liblangutil/Exceptions.h>

#include <boost/functional/hash.hpp>

#include <sstream>
#include <random>

using namespace solidity::frontend::test;
using namespace std;

extern "C" size_t LLVMFuzzerMutate(uint8_t *Data, size_t Size, size_t MaxSize);

struct CustomMutator {
	using RandomEngine = std::mt19937_64;

	explicit CustomMutator(unsigned _seed = 1337): Rand(_seed) {}

	enum class Mutation {
		DEFAULT,
		ADDSTMT,
		NUMMUTATIONS
	};

	size_t operator()(uint8_t* _data, size_t _size, size_t _maxSize, unsigned int _seed)
	{
		return this->operator()(
			_data,
			_size,
			_maxSize,
			static_cast<Mutation>(_seed % static_cast<unsigned int>(Mutation::NUMMUTATIONS))
		);
	}

	size_t operator()(uint8_t* _data, size_t _size, size_t _maxSize, Mutation _mut)
	{
		if (_maxSize <= _size || _size == 0)
			return LLVMFuzzerMutate(_data, _size, _maxSize);

		switch (_mut)
		{
		case Mutation::DEFAULT:
			return LLVMFuzzerMutate(_data, _size, _maxSize);
		case Mutation::ADDSTMT:
			return addStmt(_data, _size, _maxSize);
		case Mutation::NUMMUTATIONS:
			solAssert(false, "Solc fuzzer: Invalid mutation selected");
		}
	}

	size_t addStmt(uint8_t* _data, size_t _size, size_t _maxSize)
	{
		// Pseudo randomly choose statement
		unsigned idx = boost::hash_value(Rand()) % s_statements.size();
		// Pseudo randomly choose mutation position
		size_t pos = boost::hash_value(Rand()) % (_size + 1);
		string stmt = s_statements[idx];
		size_t stmtLen = stmt.size();
		// Fit length must not equal/exceed maxSize
		size_t fitLen = 0;
		if (pos + stmtLen >= _maxSize)
			fitLen = _maxSize - pos - 1;
		else
			fitLen = stmtLen;

		memcpy(
			static_cast<void *>(_data + pos),
			static_cast<void const*>(stmt.data()),
			fitLen
		);

		size_t newSize = pos + fitLen > _size ? pos + fitLen : _size;
		return newSize;
	}

	RandomEngine Rand;
};

extern "C" size_t LLVMFuzzerCustomMutator(uint8_t* _data, size_t _size, size_t _maxSize, unsigned int _seed)
{
	return CustomMutator{}(_data, _size, _maxSize, _seed);
}

extern "C" int LLVMFuzzerTestOneInput(uint8_t const* _data, size_t _size)
{
	if (_size <= 600)
	{
		string input(reinterpret_cast<char const*>(_data), _size);
		map<string, string> sourceCode;
		try
		{
			TestCaseReader t = TestCaseReader(std::istringstream(input));
			sourceCode = t.sources().sources;
		}
		catch (runtime_error const&)
		{
			return 0;
		}
		FuzzerUtil::testCompiler(sourceCode, /*optimize=*/false, /*_rand=*/_size);
	}
	return 0;
}
