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

#pragma once

#include <test/tools/ossfuzz/antlr4-runtime/SolidityLexer.h>
#include <test/tools/ossfuzz/antlr4-runtime/SolidityMutator.h>
#include <test/tools/ossfuzz/antlr4-runtime/SolidityParser.h>

#include <cstddef>
#include <cstdint>
#include <ostream>
#include <random>

extern "C" size_t LLVMFuzzerMutate(uint8_t *Data, size_t Size, size_t MaxSize);

namespace solidity::test::fuzzer
{
	struct CustomMutator
	{
		using RandomEngine = std::mt19937_64;

		explicit CustomMutator(uint8_t* _data, size_t _size, size_t _maxSize, unsigned _seed = 1337):
			Rand(_seed),
			Data(_data),
			Size(_size),
			In(std::string(_data, _data + _size)),
			MaxMutantSize(_maxSize),
			AStream(In),
			Lexer(&AStream),
			Tokens(&Lexer),
			Parser(&Tokens)
		{
			Lexer.removeErrorListeners();
			Parser.removeErrorListeners();
			Parser.sourceUnit()->accept(&Visitor);
		}

		enum class Mutation
		{
			ANTLR,
			NUMMUTATIONS
		};

		size_t mutate()
		{
			return mutation(static_cast<Mutation>(Rand() % static_cast<unsigned int>(Mutation::NUMMUTATIONS)));
		}

		size_t mutation(Mutation _mut);

		RandomEngine Rand;
		uint8_t* Data;
		size_t Size;
		std::string In;
		std::string Out;
		size_t MaxMutantSize;

		antlr4::ANTLRInputStream AStream;
		SolidityLexer Lexer;
		antlr4::CommonTokenStream Tokens;
		SolidityParser Parser;
		SolidityMutator Visitor;
	};

}