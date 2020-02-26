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

#include <tools/yulPhaser/FitnessMetrics.h>

#include <libyul/optimiser/EquivalentFunctionCombiner.h>
#include <libyul/optimiser/UnusedPruner.h>

#include <liblangutil/CharStream.h>

#include <libsolutil/CommonIO.h>

#include <boost/test/unit_test.hpp>

using namespace std;
using namespace solidity::langutil;
using namespace solidity::util;
using namespace solidity::yul;

namespace solidity::phaser::test
{

class DummyProgramBasedMetric: public ProgramBasedMetric
{
public:
	using ProgramBasedMetric::ProgramBasedMetric;
	size_t evaluate(Chromosome const&) const override { return 0; }
};

class ProgramBasedMetricFixture
{
protected:
	static constexpr char SampleSourceCode[] =
		"{\n"
		"    function foo() -> result\n"
		"    {\n"
		"        let x := 1\n"
		"        result := 15\n"
		"    }\n"
		"    function bar() -> result\n"
		"    {\n"
		"        result := 15\n"
		"    }\n"
		"    mstore(foo(), bar())\n"
		"}\n";

	Program optimisedProgram(Program _program) const
	{
		[[maybe_unused]] size_t originalSize = _program.codeSize();
		Program result = move(_program);
		result.optimise(m_chromosome.optimisationSteps());

		// Make sure that the program and the chromosome we have chosen are suitable for the test
		assert(result.codeSize() != originalSize);

		return result;
	}

	CharStream m_sourceStream = CharStream(SampleSourceCode, "");
	Chromosome m_chromosome{vector<string>{UnusedPruner::name, EquivalentFunctionCombiner::name}};
	Program m_program = get<Program>(Program::load(m_sourceStream));
	Program m_optimisedProgram = optimisedProgram(m_program);
};

BOOST_AUTO_TEST_SUITE(Phaser)
BOOST_AUTO_TEST_SUITE(FitnessMetricsTest)
BOOST_AUTO_TEST_SUITE(ProgramBasedMetricTest)

BOOST_FIXTURE_TEST_CASE(optimisedProgram_should_return_optimised_program, ProgramBasedMetricFixture)
{
	string code = toString(DummyProgramBasedMetric(m_program).optimisedProgram(m_chromosome));

	BOOST_TEST(code != toString(m_program));
	BOOST_TEST(code == toString(m_optimisedProgram));
}

BOOST_AUTO_TEST_SUITE_END()
BOOST_AUTO_TEST_SUITE(ProgramSizeTest)

BOOST_FIXTURE_TEST_CASE(evaluate_should_compute_size_of_the_optimised_program, ProgramBasedMetricFixture)
{
	size_t fitness = ProgramSize(m_program).evaluate(m_chromosome);

	BOOST_TEST(fitness != m_program.codeSize());
	BOOST_TEST(fitness == m_optimisedProgram.codeSize());
}

BOOST_FIXTURE_TEST_CASE(evaluate_should_repeat_the_optimisation_specified_number_of_times, ProgramBasedMetricFixture)
{
	Program const& programOptimisedOnce = m_optimisedProgram;
	Program programOptimisedTwice = optimisedProgram(programOptimisedOnce);

	ProgramSize metric(m_program, 2);
	size_t fitness = metric.evaluate(m_chromosome);

	BOOST_TEST(fitness != m_program.codeSize());
	BOOST_TEST(fitness != programOptimisedOnce.codeSize());
	BOOST_TEST(fitness == programOptimisedTwice.codeSize());
}

BOOST_FIXTURE_TEST_CASE(evaluate_should_not_optimise_if_number_of_repetitions_is_zero, ProgramBasedMetricFixture)
{
	ProgramSize metric(m_program, 0);
	size_t fitness = metric.evaluate(m_chromosome);

	BOOST_TEST(fitness == m_program.codeSize());
	BOOST_TEST(fitness != m_optimisedProgram.codeSize());
}

BOOST_AUTO_TEST_SUITE_END()
BOOST_AUTO_TEST_SUITE_END()
BOOST_AUTO_TEST_SUITE_END()

}
