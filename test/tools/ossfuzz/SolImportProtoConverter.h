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

#include <test/tools/ossfuzz/solImportProto.pb.h>

#include <random>

namespace solidity::test::solimportprotofuzzer
{

/// Random number generator that is seeded with a fuzzer
/// supplied unsigned integer.
struct SolRandomNumGenerator
{
	using RandomEngine = std::mt19937_64;

	explicit SolRandomNumGenerator(unsigned _seed): m_random(RandomEngine(_seed)) {}

	/// @returns a pseudo random unsigned integer
	unsigned operator()()
	{
		return m_random();
	}

	RandomEngine m_random;
};

class ProtoConverter
{
public:
	ProtoConverter() {}
	ProtoConverter(ProtoConverter const&) = delete;
	ProtoConverter(ProtoConverter&&) = delete;
	std::map<std::string, std::string> sourceCodeMap(Test const& _input);
private:
	std::string visit(Source const& _source);
	unsigned randomNum()
	{
		return (*m_randomGen)();
	}

	/// Number of source files declared in test file
	unsigned m_numSources = 0;
	/// Source code map to be passed to compiler stack
	std::map<std::string, std::string> m_sourceMap;
	/// Smart pointer to a random number generator seeded by fuzzing input
	std::unique_ptr<SolRandomNumGenerator> m_randomGen;
};
}