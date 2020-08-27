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

#include <test/tools/ossfuzz/SolidityCustomMutatorInterface.h>

#include <test/tools/ossfuzz/SolidityLexer.h>
#include <test/tools/ossfuzz/SolidityMutator.h>
#include <test/tools/ossfuzz/SolidityParser.h>

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
	return SolidityCustomMutatorInterface{_data, _size, _maxSize, _seed}.mutate();
}
}

SolidityCustomMutatorInterface::SolidityCustomMutatorInterface(uint8_t* _data, size_t _size, size_t _maxSize, unsigned int _seed):
    data(_data),
    size(_size),
    maxMutantSize(_maxSize),
    seed(_seed)
{}

size_t SolidityCustomMutatorInterface::mutate()
{
	antlr4::ANTLRInputStream aStream;
	try
	{
		aStream = antlr4::ANTLRInputStream(string(data, data + size));
	}
	// Range error is thrown by antlr4 runtime's ANTLRInputStream
	// See https://github.com/antlr/antlr4/issues/2036
	catch (range_error const&)
	{
		// Default to libFuzzer's mutator
		return LLVMFuzzerMutate(data, size, maxMutantSize);
	}
	SolidityLexer lexer(&aStream);
	lexer.removeErrorListeners();
	antlr4::CommonTokenStream tokens(&lexer);
	tokens.fill();
	SolidityParser parser(&tokens);
	parser.removeErrorListeners();
	SolidityMutator mutator(seed);
	parser.sourceUnit()->accept(&mutator);
	out = mutator.toString();
	solAssert(
		!out.empty() && data,
		"Solc custom mutator: Invalid mutant or memory pointer"
	);
	size_t mutantSize = min(out.size(), maxMutantSize - 1);
	mempcpy(data, out.data(), mutantSize);
	return mutantSize;
}
