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

#include <tools/yulPhaser/ProgramCache.h>
#include <tools/yulPhaser/Chromosome.h>

#include <libyul/optimiser/Metrics.h>

#include <liblangutil/CharStream.h>

#include <libsolutil/CommonIO.h>

#include <boost/test/unit_test.hpp>

#include <string>
#include <set>

using namespace std;
using namespace solidity::util;
using namespace solidity::langutil;
using namespace solidity::yul;

namespace solidity::phaser::test
{

class ProgramCacheFixture
{
protected:
	static constexpr char SampleSourceCode[] =
		"{\n"
		"    for { let i := 0 } not(eq(i, 15)) { i := add(i, 1) }\n"
		"    {\n"
		"        let x := 1\n"
		"        mstore(i, 2)\n"
		"    }\n"
		"}\n";

	Program optimisedProgram(Program _program, string _abbreviatedOptimisationSteps) const
	{
		Program result = std::move(_program);
		result.optimise(Chromosome::genesToSteps(_abbreviatedOptimisationSteps));
		return result;
	}

	static set<string> cachedKeys(ProgramCache const& _programCache)
	{
		 set<string> keys;
		for (auto pair = _programCache.entries().begin(); pair != _programCache.entries().end(); ++pair)
			keys.insert(pair->first);

		return keys;
	}

	CharStream m_sourceStream = CharStream(SampleSourceCode, "program-cache-test");
	Program m_program = get<Program>(Program::load(m_sourceStream));
	ProgramCache m_programCache{m_program};
};

BOOST_AUTO_TEST_SUITE(Phaser, *boost::unit_test::label("nooptions"))
BOOST_AUTO_TEST_SUITE(ProgramCacheTest)

BOOST_AUTO_TEST_CASE(CacheStats_operator_plus_should_add_stats_together)
{
	CacheStats statsA{11, 12, 13, {{1, 14}, {2, 15}}};
	CacheStats statsB{21, 22, 23, {{2, 24}, {3, 25}}};
	CacheStats statsC{32, 34, 36, {{1, 14}, {2, 39}, {3, 25}}};

	BOOST_CHECK(statsA + statsB == statsC);
}

BOOST_FIXTURE_TEST_CASE(optimiseProgram_should_apply_optimisation_steps_to_program, ProgramCacheFixture)
{
	Program expectedProgram = optimisedProgram(m_program, "IuO");
	assert(toString(expectedProgram) != toString(m_program));

	Program cachedProgram = m_programCache.optimiseProgram("IuO");

	BOOST_TEST(toString(cachedProgram) == toString(expectedProgram));
}

BOOST_FIXTURE_TEST_CASE(optimiseProgram_should_store_programs_for_all_prefixes, ProgramCacheFixture)
{
	Program programI = optimisedProgram(m_program, "I");
	Program programIu = optimisedProgram(programI, "u");
	Program programIuO = optimisedProgram(programIu, "O");
	assert(toString(m_program) != toString(programI));
	assert(toString(m_program) != toString(programIu));
	assert(toString(m_program) != toString(programIuO));
	assert(toString(programI) != toString(programIu));
	assert(toString(programI) != toString(programIuO));
	assert(toString(programIu) != toString(programIuO));

	BOOST_REQUIRE(m_programCache.size() == 0);

	Program cachedProgram = m_programCache.optimiseProgram("IuO");

	BOOST_TEST(toString(cachedProgram) == toString(programIuO));

	BOOST_REQUIRE((cachedKeys(m_programCache) == set<string>{"I", "Iu", "IuO"}));
	BOOST_TEST(toString(*m_programCache.find("I")) == toString(programI));
	BOOST_TEST(toString(*m_programCache.find("Iu")) == toString(programIu));
	BOOST_TEST(toString(*m_programCache.find("IuO")) == toString(programIuO));
}

BOOST_FIXTURE_TEST_CASE(optimiseProgram_should_repeat_the_chromosome_requested_number_of_times, ProgramCacheFixture)
{
	string steps = "IuOIuO";

	Program cachedProgram = m_programCache.optimiseProgram("IuO", 2);

	ProgramCache cacheNoRepetitions(m_program);
	Program cachedProgramNoRepetitions = cacheNoRepetitions.optimiseProgram("IuOIuO");

	BOOST_TEST(toString(cachedProgram) == toString(cachedProgramNoRepetitions));

	for (size_t size = 1; size <= 6; ++size)
	{
		BOOST_REQUIRE(m_programCache.contains(steps.substr(0, size)));
		BOOST_REQUIRE(cacheNoRepetitions.contains(steps.substr(0, size)));
		BOOST_TEST(
			toString(*cacheNoRepetitions.find(steps.substr(0, size))) ==
			toString(*m_programCache.find(steps.substr(0, size)))
		);
	}
}

BOOST_FIXTURE_TEST_CASE(optimiseProgram_should_reuse_the_longest_prefix_and_move_it_to_the_next_round, ProgramCacheFixture)
{
	BOOST_TEST(m_programCache.currentRound() == 0);

	m_programCache.optimiseProgram("Iu");
	m_programCache.optimiseProgram("Ia");
	m_programCache.startRound(1);

	BOOST_TEST(m_programCache.currentRound() == 1);
	BOOST_REQUIRE((cachedKeys(m_programCache) == set<string>{"I", "Iu", "Ia"}));
	BOOST_TEST(m_programCache.entries().find("I")->second.roundNumber == 0);
	BOOST_TEST(m_programCache.entries().find("Iu")->second.roundNumber == 0);
	BOOST_TEST(m_programCache.entries().find("Ia")->second.roundNumber == 0);

	m_programCache.optimiseProgram("IuOI");

	BOOST_REQUIRE((cachedKeys(m_programCache) == set<string>{"I", "Iu", "Ia", "IuO", "IuOI"}));
	BOOST_TEST(m_programCache.entries().find("I")->second.roundNumber == 1);
	BOOST_TEST(m_programCache.entries().find("Iu")->second.roundNumber == 1);
	BOOST_TEST(m_programCache.entries().find("Ia")->second.roundNumber == 0);
	BOOST_TEST(m_programCache.entries().find("IuO")->second.roundNumber == 1);
	BOOST_TEST(m_programCache.entries().find("IuOI")->second.roundNumber == 1);
}

BOOST_FIXTURE_TEST_CASE(startRound_should_remove_entries_older_than_two_rounds, ProgramCacheFixture)
{
	BOOST_TEST(m_programCache.currentRound() == 0);
	BOOST_TEST(m_programCache.size() == 0);

	m_programCache.optimiseProgram("Iu");

	BOOST_TEST(m_programCache.currentRound() == 0);
	BOOST_REQUIRE((cachedKeys(m_programCache) == set<string>{"I", "Iu"}));
	BOOST_TEST(m_programCache.entries().find("I")->second.roundNumber == 0);
	BOOST_TEST(m_programCache.entries().find("Iu")->second.roundNumber == 0);

	m_programCache.optimiseProgram("a");

	BOOST_TEST(m_programCache.currentRound() == 0);
	BOOST_REQUIRE((cachedKeys(m_programCache) == set<string>{"I", "Iu", "a"}));
	BOOST_TEST(m_programCache.entries().find("I")->second.roundNumber == 0);
	BOOST_TEST(m_programCache.entries().find("Iu")->second.roundNumber == 0);
	BOOST_TEST(m_programCache.entries().find("a")->second.roundNumber == 0);

	m_programCache.startRound(1);

	BOOST_TEST(m_programCache.currentRound() == 1);
	BOOST_REQUIRE((cachedKeys(m_programCache) == set<string>{"I", "Iu", "a"}));
	BOOST_TEST(m_programCache.entries().find("I")->second.roundNumber == 0);
	BOOST_TEST(m_programCache.entries().find("Iu")->second.roundNumber == 0);
	BOOST_TEST(m_programCache.entries().find("a")->second.roundNumber == 0);

	m_programCache.optimiseProgram("af");

	BOOST_TEST(m_programCache.currentRound() == 1);
	BOOST_REQUIRE((cachedKeys(m_programCache) == set<string>{"I", "Iu", "a", "af"}));
	BOOST_TEST(m_programCache.entries().find("I")->second.roundNumber == 0);
	BOOST_TEST(m_programCache.entries().find("Iu")->second.roundNumber == 0);
	BOOST_TEST(m_programCache.entries().find("a")->second.roundNumber == 1);
	BOOST_TEST(m_programCache.entries().find("af")->second.roundNumber == 1);

	m_programCache.startRound(2);

	BOOST_TEST(m_programCache.currentRound() == 2);
	BOOST_REQUIRE((cachedKeys(m_programCache) == set<string>{"a", "af"}));
	BOOST_TEST(m_programCache.entries().find("a")->second.roundNumber == 1);
	BOOST_TEST(m_programCache.entries().find("af")->second.roundNumber == 1);

	m_programCache.startRound(3);

	BOOST_TEST(m_programCache.currentRound() == 3);
	BOOST_TEST(m_programCache.size() == 0);
}

BOOST_FIXTURE_TEST_CASE(gatherStats_should_return_cache_statistics, ProgramCacheFixture)
{
	size_t sizeI = optimisedProgram(m_program, "I").codeSize(CacheStats::StorageWeights);
	size_t sizeIu = optimisedProgram(m_program, "Iu").codeSize(CacheStats::StorageWeights);
	size_t sizeIuO = optimisedProgram(m_program, "IuO").codeSize(CacheStats::StorageWeights);
	size_t sizeL = optimisedProgram(m_program, "L").codeSize(CacheStats::StorageWeights);
	size_t sizeLT = optimisedProgram(m_program, "LT").codeSize(CacheStats::StorageWeights);

	m_programCache.optimiseProgram("L");
	m_programCache.optimiseProgram("Iu");
	BOOST_REQUIRE((cachedKeys(m_programCache) == set<string>{"L", "I", "Iu"}));
	CacheStats expectedStats1{0, 3, sizeL + sizeI + sizeIu, {{0, 3}}};
	BOOST_CHECK(m_programCache.gatherStats() == expectedStats1);

	m_programCache.optimiseProgram("IuO");
	BOOST_REQUIRE((cachedKeys(m_programCache) == set<string>{"L", "I", "Iu", "IuO"}));
	CacheStats expectedStats2{2, 4, sizeL + sizeI + sizeIu + sizeIuO, {{0, 4}}};
	BOOST_CHECK(m_programCache.gatherStats() == expectedStats2);

	m_programCache.startRound(1);
	BOOST_REQUIRE((cachedKeys(m_programCache) == set<string>{"L", "I", "Iu", "IuO"}));
	BOOST_CHECK(m_programCache.gatherStats() == expectedStats2);

	m_programCache.optimiseProgram("IuO");
	BOOST_REQUIRE((cachedKeys(m_programCache) == set<string>{"L", "I", "Iu", "IuO"}));
	CacheStats expectedStats3{5, 4, sizeL + sizeI + sizeIu + sizeIuO, {{0, 1}, {1, 3}}};
	BOOST_CHECK(m_programCache.gatherStats() == expectedStats3);

	m_programCache.startRound(2);
	BOOST_REQUIRE((cachedKeys(m_programCache) == set<string>{"I", "Iu", "IuO"}));
	CacheStats expectedStats4{5, 4, sizeI + sizeIu + sizeIuO, {{1, 3}}};
	BOOST_CHECK(m_programCache.gatherStats() == expectedStats4);

	m_programCache.optimiseProgram("LT");
	BOOST_REQUIRE((cachedKeys(m_programCache) == set<string>{"L", "LT", "I", "Iu", "IuO"}));
	CacheStats expectedStats5{5, 6, sizeL + sizeLT + sizeI + sizeIu + sizeIuO, {{1, 3}, {2, 2}}};
	BOOST_CHECK(m_programCache.gatherStats() == expectedStats5);
}

BOOST_AUTO_TEST_SUITE_END()
BOOST_AUTO_TEST_SUITE_END()

}
