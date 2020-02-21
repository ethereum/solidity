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

#include <test/yulPhaser/Common.h>

#include <tools/yulPhaser/AlgorithmRunner.h>

#include <libsolutil/CommonIO.h>

#include <boost/test/unit_test.hpp>
#include <boost/test/tools/output_test_stream.hpp>

using namespace std;
using namespace boost::unit_test::framework;
using namespace boost::test_tools;
using namespace solidity::util;

namespace solidity::phaser::test
{

class DummyAlgorithm: public GeneticAlgorithm
{
public:
	using GeneticAlgorithm::GeneticAlgorithm;
	Population runNextRound(Population _population) override
	{
		++m_currentRound;
		return _population;
	}

	size_t m_currentRound = 0;
};

class AlgorithmRunnerFixture
{
protected:
	shared_ptr<FitnessMetric> m_fitnessMetric = make_shared<ChromosomeLengthMetric>();
	output_test_stream m_output;
};

BOOST_AUTO_TEST_SUITE(Phaser)
BOOST_AUTO_TEST_SUITE(AlgorithmRunnerTest)

BOOST_FIXTURE_TEST_CASE(run_should_call_runNextRound_once_per_round, AlgorithmRunnerFixture)
{
	AlgorithmRunner runner(Population(m_fitnessMetric), m_output);
	DummyAlgorithm algorithm;

	BOOST_TEST(algorithm.m_currentRound == 0);
	runner.run(algorithm, 10);
	BOOST_TEST(algorithm.m_currentRound == 10);
	runner.run(algorithm, 3);
	BOOST_TEST(algorithm.m_currentRound == 13);
}

BOOST_FIXTURE_TEST_CASE(run_should_print_the_top_chromosome, AlgorithmRunnerFixture)
{
	// run() is allowed to print more but should at least print the first one

	AlgorithmRunner runner(
		// NOTE: Chromosomes chosen so that they're not substrings of each other and are not
		// words likely to appear in the output in normal circumstances.
		Population(m_fitnessMetric, {Chromosome("fcCUnDve"), Chromosome("jsxIOo"), Chromosome("ighTLM")}),
		m_output
	);

	DummyAlgorithm algorithm;

	BOOST_TEST(m_output.is_empty());
	runner.run(algorithm, 1);
	BOOST_TEST(countSubstringOccurrences(m_output.str(), toString(runner.population().individuals()[0].chromosome)) == 1);
	runner.run(algorithm, 3);
	BOOST_TEST(countSubstringOccurrences(m_output.str(), toString(runner.population().individuals()[0].chromosome)) == 4);
}

BOOST_AUTO_TEST_SUITE_END()
BOOST_AUTO_TEST_SUITE_END()

}
