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

#include <tools/yulPhaser/FitnessMetrics.h>

#include <libyul/optimiser/EquivalentFunctionCombiner.h>
#include <libyul/optimiser/UnusedPruner.h>

#include <liblangutil/CharStream.h>

#include <libsolutil/CommonIO.h>

#include <boost/test/unit_test.hpp>

#include <cmath>

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
	size_t evaluate(Chromosome const&) override { return 0; }
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
		[[maybe_unused]] size_t originalSize = _program.codeSize(m_weights);
		Program result = move(_program);
		result.optimise(m_chromosome.optimisationSteps());

		// Make sure that the program and the chromosome we have chosen are suitable for the test
		assert(result.codeSize(m_weights) != originalSize);

		return result;
	}

	CharStream m_sourceStream = CharStream(SampleSourceCode, "");
	Chromosome m_chromosome{vector<string>{UnusedPruner::name, EquivalentFunctionCombiner::name}};
	Program m_program = get<Program>(Program::load(m_sourceStream));
	Program m_optimisedProgram = optimisedProgram(m_program);
	shared_ptr<ProgramCache> m_programCache = make_shared<ProgramCache>(m_program);
	static constexpr CodeWeights m_weights{};
};

class FitnessMetricCombinationFixture: public ProgramBasedMetricFixture
{
protected:
	vector<shared_ptr<FitnessMetric>> m_simpleMetrics = {
		make_shared<ProgramSize>(m_program, nullptr, m_weights, 1),
		make_shared<ProgramSize>(m_program, nullptr, m_weights, 2),
		make_shared<ProgramSize>(m_program, nullptr, m_weights, 3),
	};
	vector<size_t> m_fitness = {
		m_simpleMetrics[0]->evaluate(m_chromosome),
		m_simpleMetrics[1]->evaluate(m_chromosome),
		m_simpleMetrics[2]->evaluate(m_chromosome),
	};
};

BOOST_AUTO_TEST_SUITE(Phaser, *boost::unit_test::label("nooptions"))
BOOST_AUTO_TEST_SUITE(FitnessMetricsTest)
BOOST_AUTO_TEST_SUITE(ProgramBasedMetricTest)

BOOST_FIXTURE_TEST_CASE(optimisedProgram_should_return_optimised_program_even_if_cache_not_available, ProgramBasedMetricFixture)
{
	string code = toString(DummyProgramBasedMetric(m_program, nullptr, m_weights).optimisedProgram(m_chromosome));

	BOOST_TEST(code != toString(m_program));
	BOOST_TEST(code == toString(m_optimisedProgram));
}

BOOST_FIXTURE_TEST_CASE(optimisedProgram_should_use_cache_if_available, ProgramBasedMetricFixture)
{
	string code = toString(DummyProgramBasedMetric(nullopt, m_programCache, m_weights).optimisedProgram(m_chromosome));

	BOOST_TEST(code != toString(m_program));
	BOOST_TEST(code == toString(m_optimisedProgram));
	BOOST_TEST(m_programCache->size() == m_chromosome.length());
}

BOOST_FIXTURE_TEST_CASE(optimisedProgramNoCache_should_return_optimised_program_even_if_cache_not_available, ProgramBasedMetricFixture)
{
	string code = toString(DummyProgramBasedMetric(m_program, nullptr, m_weights).optimisedProgramNoCache(m_chromosome));

	BOOST_TEST(code != toString(m_program));
	BOOST_TEST(code == toString(m_optimisedProgram));
}

BOOST_FIXTURE_TEST_CASE(optimisedProgramNoCache_should_not_use_cache_even_if_available, ProgramBasedMetricFixture)
{
	string code = toString(DummyProgramBasedMetric(nullopt, m_programCache, m_weights).optimisedProgramNoCache(m_chromosome));

	BOOST_TEST(code != toString(m_program));
	BOOST_TEST(code == toString(m_optimisedProgram));
	BOOST_TEST(m_programCache->size() == 0);
}

BOOST_AUTO_TEST_SUITE_END()
BOOST_AUTO_TEST_SUITE(ProgramSizeTest)

BOOST_FIXTURE_TEST_CASE(evaluate_should_compute_size_of_the_optimised_program, ProgramBasedMetricFixture)
{
	size_t fitness = ProgramSize(m_program, nullptr, m_weights).evaluate(m_chromosome);

	BOOST_TEST(fitness != m_program.codeSize(m_weights));
	BOOST_TEST(fitness == m_optimisedProgram.codeSize(m_weights));
}

BOOST_FIXTURE_TEST_CASE(evaluate_should_be_able_to_use_program_cache_if_available, ProgramBasedMetricFixture)
{
	size_t fitness = ProgramSize(nullopt, m_programCache, m_weights).evaluate(m_chromosome);

	BOOST_TEST(fitness != m_program.codeSize(m_weights));
	BOOST_TEST(fitness == m_optimisedProgram.codeSize(m_weights));
	BOOST_TEST(m_programCache->size() == m_chromosome.length());
}

BOOST_FIXTURE_TEST_CASE(evaluate_should_repeat_the_optimisation_specified_number_of_times, ProgramBasedMetricFixture)
{
	Program const& programOptimisedOnce = m_optimisedProgram;
	Program programOptimisedTwice = optimisedProgram(programOptimisedOnce);

	ProgramSize metric(m_program, nullptr, m_weights, 2);
	size_t fitness = metric.evaluate(m_chromosome);

	BOOST_TEST(fitness != m_program.codeSize(m_weights));
	BOOST_TEST(fitness != programOptimisedOnce.codeSize(m_weights));
	BOOST_TEST(fitness == programOptimisedTwice.codeSize(m_weights));
}

BOOST_FIXTURE_TEST_CASE(evaluate_should_not_optimise_if_number_of_repetitions_is_zero, ProgramBasedMetricFixture)
{
	ProgramSize metric(m_program, nullptr, m_weights, 0);
	size_t fitness = metric.evaluate(m_chromosome);

	BOOST_TEST(fitness == m_program.codeSize(m_weights));
	BOOST_TEST(fitness != m_optimisedProgram.codeSize(m_weights));
}

BOOST_AUTO_TEST_SUITE_END()
BOOST_AUTO_TEST_SUITE(RelativeProgramSizeTest)

BOOST_FIXTURE_TEST_CASE(evaluate_should_compute_the_size_ratio_between_optimised_program_and_original_program, ProgramBasedMetricFixture)
{
	BOOST_TEST(
		RelativeProgramSize(m_program, nullptr, 3, m_weights).evaluate(m_chromosome) ==
		round(1000.0 * m_optimisedProgram.codeSize(m_weights) / m_program.codeSize(m_weights))
	);
}

BOOST_FIXTURE_TEST_CASE(evaluate_should_be_able_to_use_program_cache_if_available, ProgramBasedMetricFixture)
{
	BOOST_TEST(
		RelativeProgramSize(nullopt, m_programCache, 3, m_weights).evaluate(m_chromosome) ==
		round(1000.0 * m_optimisedProgram.codeSize(m_weights) / m_program.codeSize(m_weights))
	);
	BOOST_TEST(m_programCache->size() == m_chromosome.length());
}

BOOST_FIXTURE_TEST_CASE(evaluate_should_repeat_the_optimisation_specified_number_of_times, ProgramBasedMetricFixture)
{
	Program const& programOptimisedOnce = m_optimisedProgram;
	Program programOptimisedTwice = optimisedProgram(programOptimisedOnce);

	RelativeProgramSize metric(m_program, nullptr, 3, m_weights, 2);
	size_t fitness = metric.evaluate(m_chromosome);

	BOOST_TEST(fitness != 1000);
	BOOST_TEST(fitness != RelativeProgramSize(programOptimisedTwice, nullptr, 3, m_weights, 1).evaluate(m_chromosome));
	BOOST_TEST(fitness == round(1000.0 * programOptimisedTwice.codeSize(m_weights) / m_program.codeSize(m_weights)));
}

BOOST_FIXTURE_TEST_CASE(evaluate_should_return_one_if_number_of_repetitions_is_zero, ProgramBasedMetricFixture)
{
	RelativeProgramSize metric(m_program, nullptr, 3, m_weights, 0);

	BOOST_TEST(metric.evaluate(m_chromosome) == 1000);
}

BOOST_FIXTURE_TEST_CASE(evaluate_should_return_one_if_the_original_program_size_is_zero, ProgramBasedMetricFixture)
{
	CharStream sourceStream = CharStream("{}", "");
	Program program = get<Program>(Program::load(sourceStream));

	RelativeProgramSize metric(program, nullptr, 3, m_weights);

	BOOST_TEST(metric.evaluate(m_chromosome) == 1000);
	BOOST_TEST(metric.evaluate(Chromosome("")) == 1000);
	BOOST_TEST(metric.evaluate(Chromosome("afcxjLTLTDoO")) == 1000);
}

BOOST_FIXTURE_TEST_CASE(evaluate_should_multiply_the_result_by_scaling_factor, ProgramBasedMetricFixture)
{
	double sizeRatio = static_cast<double>(m_optimisedProgram.codeSize(m_weights)) / m_program.codeSize(m_weights);
	BOOST_TEST(RelativeProgramSize(m_program, nullptr, 0, m_weights).evaluate(m_chromosome) == round(1.0 * sizeRatio));
	BOOST_TEST(RelativeProgramSize(m_program, nullptr, 1, m_weights).evaluate(m_chromosome) == round(10.0 * sizeRatio));
	BOOST_TEST(RelativeProgramSize(m_program, nullptr, 2, m_weights).evaluate(m_chromosome) == round(100.0 * sizeRatio));
	BOOST_TEST(RelativeProgramSize(m_program, nullptr, 3, m_weights).evaluate(m_chromosome) == round(1000.0 * sizeRatio));
	BOOST_TEST(RelativeProgramSize(m_program, nullptr, 4, m_weights).evaluate(m_chromosome) == round(10000.0 * sizeRatio));
}

BOOST_AUTO_TEST_SUITE_END()
BOOST_AUTO_TEST_SUITE(FitnessMetricCombinationTest)

BOOST_FIXTURE_TEST_CASE(FitnessMetricAverage_evaluate_should_compute_average_of_values_returned_by_metrics_passed_to_it, FitnessMetricCombinationFixture)
{
	FitnessMetricAverage metric(m_simpleMetrics);

	assert(m_simpleMetrics.size() == 3);
	BOOST_TEST(metric.evaluate(m_chromosome) == (m_fitness[0] + m_fitness[1] + m_fitness[2]) / 3);
	BOOST_TEST(metric.metrics() == m_simpleMetrics);
}

BOOST_FIXTURE_TEST_CASE(FitnessMetricSum_evaluate_should_compute_sum_of_values_returned_by_metrics_passed_to_it, FitnessMetricCombinationFixture)
{
	FitnessMetricSum metric(m_simpleMetrics);

	assert(m_simpleMetrics.size() == 3);
	BOOST_TEST(metric.evaluate(m_chromosome) == m_fitness[0] + m_fitness[1] + m_fitness[2]);
	BOOST_TEST(metric.metrics() == m_simpleMetrics);
}

BOOST_FIXTURE_TEST_CASE(FitnessMetricMaximum_evaluate_should_compute_maximum_of_values_returned_by_metrics_passed_to_it, FitnessMetricCombinationFixture)
{
	FitnessMetricMaximum metric(m_simpleMetrics);

	assert(m_simpleMetrics.size() == 3);
	BOOST_TEST(metric.evaluate(m_chromosome) == max(m_fitness[0], max(m_fitness[1], m_fitness[2])));
	BOOST_TEST(metric.metrics() == m_simpleMetrics);
}

BOOST_FIXTURE_TEST_CASE(FitnessMetricMinimum_evaluate_should_compute_minimum_of_values_returned_by_metrics_passed_to_it, FitnessMetricCombinationFixture)
{
	FitnessMetricMinimum metric(m_simpleMetrics);

	assert(m_simpleMetrics.size() == 3);
	BOOST_TEST(metric.evaluate(m_chromosome) == min(m_fitness[0], min(m_fitness[1], m_fitness[2])));
	BOOST_TEST(metric.metrics() == m_simpleMetrics);
}

BOOST_AUTO_TEST_SUITE_END()
BOOST_AUTO_TEST_SUITE_END()
BOOST_AUTO_TEST_SUITE_END()

}
