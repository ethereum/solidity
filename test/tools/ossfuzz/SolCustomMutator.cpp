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

#include <test/tools/ossfuzz/SolCustomMutator.h>

#include <antlr4-runtime.h>
#include <cstring>

using namespace std;
using namespace antlr4;
using namespace solidity::test::fuzzer;

namespace {
/// Forward declare libFuzzer's default mutator definition
extern "C" size_t LLVMFuzzerMutate(uint8_t* _data, size_t _size, size_t _maxSize);

/// Define Solidity's custom mutator by implementing libFuzzer's
/// custom mutator external interface.
extern "C" size_t LLVMFuzzerCustomMutator(
	uint8_t* _data,
	size_t _size,
	size_t _maxSize,
	unsigned int _seed
)
{
	if (_maxSize <= _size || _size == 0)
		return LLVMFuzzerMutate(_data, _size, _maxSize);
	return SolCustomMutator{_data, _size, _maxSize, _seed}.mutate();
}
}

SolCustomMutator::SolCustomMutator(uint8_t* _data, size_t _size, size_t _maxSize, unsigned int _seed)
{
	Data = _data;
	Size = _size;
	In = string(_data, _data + _size),
	MaxMutantSize = _maxSize;
	Seed = _seed;
}

size_t SolCustomMutator::mutate()
{
	try
	{
		antlr4::ANTLRInputStream AStream(In);
		SolidityLexer Lexer(&AStream);
		Lexer.removeErrorListeners();
		antlr4::CommonTokenStream Tokens(&Lexer);
		Tokens.fill();
		SolidityParser Parser(&Tokens);
		Parser.removeErrorListeners();
		SolidityMutator Mutator(Seed);
		Parser.sourceUnit()->accept(&Mutator);
		Out = Mutator.toString();

		// If mutant is empty, default to libFuzzer's mutator
		if (Out.empty())
			return Size;
		else
		{
			size_t mutantSize = Out.size() >= MaxMutantSize ? MaxMutantSize - 1 : Out.size();
			fill_n(Data, MaxMutantSize, 0);
			copy(Out.data(), Out.data() + mutantSize, Data);
			return mutantSize;
		}
	}
	// Range error is thrown by antlr4 runtime's ANTLRInputStream
	// See https://github.com/antlr/antlr4/issues/2036
	catch (range_error const&)
	{
		// Default to libFuzzer's mutator
		return Size;
	}
}
