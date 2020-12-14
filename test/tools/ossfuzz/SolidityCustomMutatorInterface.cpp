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
#include <test/tools/ossfuzz/SolidityGenerator.h>

#include <liblangutil/Exceptions.h>

using namespace std;
using namespace solidity::test::fuzzer;

// Prototype as we can't use the FuzzerInterface.h header.
extern "C" size_t LLVMFuzzerMutate(uint8_t* _data, size_t _size, size_t _maxSize);
extern "C" size_t LLVMFuzzerCustomMutator(uint8_t* _data, size_t size, size_t _maxSize, unsigned int seed);

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
	return SolidityCustomMutatorInterface{_data, _size, _maxSize, _seed}.generate();
}

SolidityCustomMutatorInterface::SolidityCustomMutatorInterface(
	uint8_t* _data,
	size_t _size,
	size_t _maxSize,
	unsigned int _seed
	):
	data(_data),
	size(_size),
	maxMutantSize(_maxSize),
	generator(make_shared<SolidityGenerator>(_seed))
{}

size_t SolidityCustomMutatorInterface::generate()
{
	string testCase = generator->generateTestProgram();
	solAssert(
		!testCase.empty() && data,
		"Solc custom mutator: Invalid mutant or memory pointer"
	);
	size_t mutantSize = min(testCase.size(), maxMutantSize - 1);
	mempcpy(data, testCase.data(), mutantSize);
	return mutantSize;
}
