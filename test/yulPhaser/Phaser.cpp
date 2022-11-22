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

#include <test/yulPhaser/TestHelpers.h>

#include <tools/yulPhaser/Exceptions.h>
#include <tools/yulPhaser/Phaser.h>

#include <liblangutil/CharStream.h>

#include <libsolutil/CommonIO.h>
#include <libsolutil/TemporaryDirectory.h>

#include <boost/filesystem.hpp>
#include <boost/test/unit_test.hpp>

#include <algorithm>
#include <fstream>

using namespace std;
using namespace solidity::util;
using namespace solidity::langutil;
using namespace solidity::yul;

namespace fs = boost::filesystem;

namespace solidity::phaser::test
{

class GeneticAlgorithmFactoryFixture
{
protected:
	GeneticAlgorithmFactory::Options m_options = {
		/* algorithm = */ Algorithm::Random,
		/* minChromosomeLength = */ 50,
		/* maxChromosomeLength = */ 100,
		/* CrossoverChoice = */ CrossoverChoice::Uniform,
		/* uniformCrossoverSwapChance = */ 0.5,
		/* randomElitePoolSize = */ 0.5,
		/* gewepMutationPoolSize = */ 0.1,
		/* gewepCrossoverPoolSize = */ 0.1,
		/* gewepRandomisationChance = */ 0.6,
		/* gewepDeletionVsAdditionChance = */ 0.3,
		/* gewepGenesToRandomise = */ 0.4,
		/* gewepGenesToAddOrDelete = */ 0.2,
		/* classicElitePoolSize = */ 0.0,
		/* classicCrossoverChance = */ 0.75,
		/* classicMutationChance = */ 0.2,
		/* classicDeletionChance = */ 0.2,
		/* classicAdditionChance = */ 0.2,
	};
};

class FixtureWithPrograms
{
protected:
	vector<CharStream> m_sourceStreams = {
		CharStream("{}", ""),
		CharStream("{{}}", ""),
		CharStream("{{{}}}", ""),
	};
	vector<Program> m_programs = {
		get<Program>(Program::load(m_sourceStreams[0])),
		get<Program>(Program::load(m_sourceStreams[1])),
		get<Program>(Program::load(m_sourceStreams[2])),
	};
};

class FitnessMetricFactoryFixture: public FixtureWithPrograms
{
protected:
	FitnessMetricFactory::Options m_options = {
		/* metric = */ MetricChoice::CodeSize,
		/* metricAggregator = */ MetricAggregatorChoice::Average,
		/* relativeMetricScale = */ 5,
		/* chromosomeRepetitions = */ 1,
	};
	CodeWeights const m_weights{};
};

class PoulationFactoryFixture
{
protected:
	shared_ptr<FitnessMetric> m_fitnessMetric = make_shared<ChromosomeLengthMetric>();
	PopulationFactory::Options m_options = {
		/* minChromosomeLength = */ 0,
		/* maxChromosomeLength = */ 0,
		/* population = */ {},
		/* randomPopulation = */ {},
		/* populationFromFile = */ {},
	};
};

BOOST_AUTO_TEST_SUITE(Phaser, *boost::unit_test::label("nooptions"))
BOOST_AUTO_TEST_SUITE(PhaserTest)
BOOST_AUTO_TEST_SUITE(GeneticAlgorithmFactoryTest)

BOOST_FIXTURE_TEST_CASE(build_should_select_the_right_algorithm_and_pass_the_options_to_it, GeneticAlgorithmFactoryFixture)
{
	m_options.algorithm = Algorithm::Random;
	unique_ptr<GeneticAlgorithm> algorithm1 = GeneticAlgorithmFactory::build(m_options, 100);
	BOOST_REQUIRE(algorithm1 != nullptr);

	auto randomAlgorithm = dynamic_cast<RandomAlgorithm*>(algorithm1.get());
	BOOST_REQUIRE(randomAlgorithm != nullptr);
	BOOST_TEST(randomAlgorithm->options().elitePoolSize == m_options.randomElitePoolSize.value());
	BOOST_TEST(randomAlgorithm->options().minChromosomeLength == m_options.minChromosomeLength);
	BOOST_TEST(randomAlgorithm->options().maxChromosomeLength == m_options.maxChromosomeLength);

	m_options.algorithm = Algorithm::GEWEP;
	unique_ptr<GeneticAlgorithm> algorithm2 = GeneticAlgorithmFactory::build(m_options, 100);
	BOOST_REQUIRE(algorithm2 != nullptr);

	auto gewepAlgorithm = dynamic_cast<GenerationalElitistWithExclusivePools*>(algorithm2.get());
	BOOST_REQUIRE(gewepAlgorithm != nullptr);
	BOOST_TEST(gewepAlgorithm->options().crossover == m_options.crossover);
	BOOST_TEST(gewepAlgorithm->options().uniformCrossoverSwapChance.has_value());
	BOOST_TEST(gewepAlgorithm->options().uniformCrossoverSwapChance.value() == m_options.uniformCrossoverSwapChance);
	BOOST_TEST(gewepAlgorithm->options().mutationPoolSize == m_options.gewepMutationPoolSize);
	BOOST_TEST(gewepAlgorithm->options().crossoverPoolSize == m_options.gewepCrossoverPoolSize);
	BOOST_TEST(gewepAlgorithm->options().randomisationChance == m_options.gewepRandomisationChance);
	BOOST_TEST(gewepAlgorithm->options().deletionVsAdditionChance == m_options.gewepDeletionVsAdditionChance);
	BOOST_TEST(gewepAlgorithm->options().percentGenesToRandomise == m_options.gewepGenesToRandomise.value());
	BOOST_TEST(gewepAlgorithm->options().percentGenesToAddOrDelete == m_options.gewepGenesToAddOrDelete.value());

	m_options.algorithm = Algorithm::Classic;
	unique_ptr<GeneticAlgorithm> algorithm3 = GeneticAlgorithmFactory::build(m_options, 100);
	BOOST_REQUIRE(algorithm3 != nullptr);

	auto classicAlgorithm = dynamic_cast<ClassicGeneticAlgorithm*>(algorithm3.get());
	BOOST_REQUIRE(classicAlgorithm != nullptr);
	BOOST_TEST(classicAlgorithm->options().uniformCrossoverSwapChance.has_value());
	BOOST_TEST(classicAlgorithm->options().uniformCrossoverSwapChance.value() == m_options.uniformCrossoverSwapChance);
	BOOST_TEST(classicAlgorithm->options().elitePoolSize == m_options.classicElitePoolSize);
	BOOST_TEST(classicAlgorithm->options().crossoverChance == m_options.classicCrossoverChance);
	BOOST_TEST(classicAlgorithm->options().mutationChance == m_options.classicMutationChance);
	BOOST_TEST(classicAlgorithm->options().deletionChance == m_options.classicDeletionChance);
	BOOST_TEST(classicAlgorithm->options().additionChance == m_options.classicAdditionChance);
}

BOOST_FIXTURE_TEST_CASE(build_should_set_random_algorithm_elite_pool_size_based_on_population_size_if_not_specified, GeneticAlgorithmFactoryFixture)
{
	m_options.algorithm = Algorithm::Random;
	m_options.randomElitePoolSize = nullopt;
	unique_ptr<GeneticAlgorithm> algorithm = GeneticAlgorithmFactory::build(m_options, 100);
	BOOST_REQUIRE(algorithm != nullptr);

	auto randomAlgorithm = dynamic_cast<RandomAlgorithm*>(algorithm.get());
	BOOST_REQUIRE(randomAlgorithm != nullptr);
	BOOST_TEST(randomAlgorithm->options().elitePoolSize == 1.0 / 100.0);
}

BOOST_FIXTURE_TEST_CASE(build_should_set_gewep_mutation_percentages_based_on_maximum_chromosome_length_if_not_specified, GeneticAlgorithmFactoryFixture)
{
	m_options.algorithm = Algorithm::GEWEP;
	m_options.gewepGenesToRandomise = nullopt;
	m_options.gewepGenesToAddOrDelete = nullopt;
	m_options.maxChromosomeLength = 125;

	unique_ptr<GeneticAlgorithm> algorithm = GeneticAlgorithmFactory::build(m_options, 100);
	BOOST_REQUIRE(algorithm != nullptr);

	auto gewepAlgorithm = dynamic_cast<GenerationalElitistWithExclusivePools*>(algorithm.get());
	BOOST_REQUIRE(gewepAlgorithm != nullptr);
	BOOST_TEST(gewepAlgorithm->options().percentGenesToRandomise == 1.0 / 125.0);
	BOOST_TEST(gewepAlgorithm->options().percentGenesToAddOrDelete == 1.0 / 125.0);
}

BOOST_AUTO_TEST_SUITE_END()
BOOST_AUTO_TEST_SUITE(FitnessMetricFactoryTest)

BOOST_FIXTURE_TEST_CASE(build_should_create_metric_of_the_right_type, FitnessMetricFactoryFixture)
{
	m_options.metric = MetricChoice::RelativeCodeSize;
	m_options.metricAggregator = MetricAggregatorChoice::Sum;
	unique_ptr<FitnessMetric> metric = FitnessMetricFactory::build(m_options, {m_programs[0]}, {nullptr}, m_weights);
	BOOST_REQUIRE(metric != nullptr);

	auto sumMetric = dynamic_cast<FitnessMetricSum*>(metric.get());
	BOOST_REQUIRE(sumMetric != nullptr);
	BOOST_REQUIRE(sumMetric->metrics().size() == 1);
	BOOST_REQUIRE(sumMetric->metrics()[0] != nullptr);

	auto relativeProgramSizeMetric = dynamic_cast<RelativeProgramSize*>(sumMetric->metrics()[0].get());
	BOOST_REQUIRE(relativeProgramSizeMetric != nullptr);
	BOOST_TEST(toString(relativeProgramSizeMetric->program()) == toString(m_programs[0]));
}

BOOST_FIXTURE_TEST_CASE(build_should_respect_chromosome_repetitions_option, FitnessMetricFactoryFixture)
{
	m_options.metric = MetricChoice::CodeSize;
	m_options.metricAggregator = MetricAggregatorChoice::Average;
	m_options.chromosomeRepetitions = 5;
	unique_ptr<FitnessMetric> metric = FitnessMetricFactory::build(m_options, {m_programs[0]}, {nullptr}, m_weights);
	BOOST_REQUIRE(metric != nullptr);

	auto averageMetric = dynamic_cast<FitnessMetricAverage*>(metric.get());
	BOOST_REQUIRE(averageMetric != nullptr);
	BOOST_REQUIRE(averageMetric->metrics().size() == 1);
	BOOST_REQUIRE(averageMetric->metrics()[0] != nullptr);

	auto programSizeMetric = dynamic_cast<ProgramSize*>(averageMetric->metrics()[0].get());
	BOOST_REQUIRE(programSizeMetric != nullptr);
	BOOST_TEST(programSizeMetric->repetitionCount() == m_options.chromosomeRepetitions);
}

BOOST_FIXTURE_TEST_CASE(build_should_set_relative_metric_scale, FitnessMetricFactoryFixture)
{
	m_options.metric = MetricChoice::RelativeCodeSize;
	m_options.metricAggregator = MetricAggregatorChoice::Average;
	m_options.relativeMetricScale = 10;
	unique_ptr<FitnessMetric> metric = FitnessMetricFactory::build(m_options, {m_programs[0]}, {nullptr}, m_weights);
	BOOST_REQUIRE(metric != nullptr);

	auto averageMetric = dynamic_cast<FitnessMetricAverage*>(metric.get());
	BOOST_REQUIRE(averageMetric != nullptr);
	BOOST_REQUIRE(averageMetric->metrics().size() == 1);
	BOOST_REQUIRE(averageMetric->metrics()[0] != nullptr);

	auto relativeProgramSizeMetric = dynamic_cast<RelativeProgramSize*>(averageMetric->metrics()[0].get());
	BOOST_REQUIRE(relativeProgramSizeMetric != nullptr);
	BOOST_TEST(relativeProgramSizeMetric->fixedPointPrecision() == m_options.relativeMetricScale);
}

BOOST_FIXTURE_TEST_CASE(build_should_create_metric_for_each_input_program, FitnessMetricFactoryFixture)
{
	unique_ptr<FitnessMetric> metric = FitnessMetricFactory::build(
		m_options,
		m_programs,
		vector<shared_ptr<ProgramCache>>(m_programs.size(), nullptr),
		m_weights
	);
	BOOST_REQUIRE(metric != nullptr);

	auto combinedMetric = dynamic_cast<FitnessMetricCombination*>(metric.get());
	BOOST_REQUIRE(combinedMetric != nullptr);
	BOOST_REQUIRE(combinedMetric->metrics().size() == m_programs.size());
}

BOOST_FIXTURE_TEST_CASE(build_should_pass_program_caches_to_metrics, FitnessMetricFactoryFixture)
{
	assert(m_programs.size() == 3);
	vector<shared_ptr<ProgramCache>> caches = {
		make_shared<ProgramCache>(m_programs[0]),
		make_shared<ProgramCache>(m_programs[1]),
		make_shared<ProgramCache>(m_programs[2]),
	};

	m_options.metric = MetricChoice::RelativeCodeSize;
	unique_ptr<FitnessMetric> metric = FitnessMetricFactory::build(m_options, m_programs, caches, m_weights);
	BOOST_REQUIRE(metric != nullptr);

	auto combinedMetric = dynamic_cast<FitnessMetricCombination*>(metric.get());
	BOOST_REQUIRE(combinedMetric != nullptr);
	BOOST_REQUIRE(combinedMetric->metrics().size() == caches.size());

	for (size_t i = 0; i < caches.size(); ++i)
	{
		auto programBasedMetric = dynamic_cast<ProgramBasedMetric*>(combinedMetric->metrics()[i].get());
		BOOST_REQUIRE(programBasedMetric != nullptr);
		BOOST_TEST(programBasedMetric->programCache() == caches[i].get());
	}
}

BOOST_AUTO_TEST_SUITE_END()
BOOST_AUTO_TEST_SUITE(PopulationFactoryTest)

BOOST_FIXTURE_TEST_CASE(build_should_create_an_empty_population_if_no_specific_options_given, PoulationFactoryFixture)
{
	m_options.population = {};
	m_options.randomPopulation = {};
	m_options.populationFromFile = {};
	BOOST_TEST(
		PopulationFactory::build(m_options, m_fitnessMetric) ==
		Population(m_fitnessMetric, vector<Chromosome>{})
	);
}

BOOST_FIXTURE_TEST_CASE(build_should_respect_population_option, PoulationFactoryFixture)
{
	m_options.population = {"a", "afc", "xadd"};
	BOOST_TEST(
		PopulationFactory::build(m_options, m_fitnessMetric) ==
		Population(m_fitnessMetric, {Chromosome("a"), Chromosome("afc"), Chromosome("xadd")})
	);
}

BOOST_FIXTURE_TEST_CASE(build_should_respect_random_population_option, PoulationFactoryFixture)
{
	m_options.randomPopulation = {5, 3, 2};
	m_options.minChromosomeLength = 5;
	m_options.maxChromosomeLength = 10;

	auto population = PopulationFactory::build(m_options, m_fitnessMetric);

	BOOST_TEST(population.individuals().size() == 10);
	BOOST_TEST(all_of(
		population.individuals().begin(),
		population.individuals().end(),
		[](auto const& individual){ return 5 <= individual.chromosome.length() && individual.chromosome.length() <= 10; }
	));
}

BOOST_FIXTURE_TEST_CASE(build_should_respect_population_from_file_option, PoulationFactoryFixture)
{
	map<string, vector<string>> fileContent = {
		{"a.txt", {"a", "fff", "", "jxccLTa"}},
		{"b.txt", {}},
		{"c.txt", {""}},
		{"d.txt", {"c", "T"}},
	};

	TemporaryDirectory tempDir;
	for (auto const& [fileName, chromosomes]: fileContent)
	{
		ofstream tmpFile((tempDir.path() / fileName).string());
		for (auto const& chromosome: chromosomes)
			tmpFile << chromosome << endl;

		m_options.populationFromFile.push_back((tempDir.path() / fileName).string());
	}

	BOOST_TEST(
		PopulationFactory::build(m_options, m_fitnessMetric) ==
		Population(m_fitnessMetric, {
			Chromosome("a"),
			Chromosome("fff"),
			Chromosome(""),
			Chromosome("jxccLTa"),
			Chromosome(""),
			Chromosome("c"),
			Chromosome("T"),
		})
	);
}

BOOST_FIXTURE_TEST_CASE(build_should_throw_FileOpenError_if_population_file_does_not_exist, PoulationFactoryFixture)
{
	m_options.populationFromFile = {"a-file-that-does-not-exist.abcdefgh"};
	assert(!fs::exists(m_options.populationFromFile[0]));

	BOOST_CHECK_THROW(PopulationFactory::build(m_options, m_fitnessMetric), FileOpenError);
}

BOOST_FIXTURE_TEST_CASE(build_should_combine_populations_from_all_sources, PoulationFactoryFixture)
{
	TemporaryDirectory tempDir;
	{
		ofstream tmpFile((tempDir.path() / "population.txt").string());
		tmpFile << "axc" << endl << "fcL" << endl;
	}

	m_options.population = {"axc", "fcL"};
	m_options.randomPopulation = {2};
	m_options.populationFromFile = {(tempDir.path() / "population.txt").string()};
	m_options.minChromosomeLength = 3;
	m_options.maxChromosomeLength = 3;

	auto population = PopulationFactory::build(m_options, m_fitnessMetric);

	auto begin = population.individuals().begin();
	auto end = population.individuals().end();
	BOOST_TEST(population.individuals().size() == 6);
	BOOST_TEST(all_of(begin, end, [](auto const& individual){ return individual.chromosome.length() == 3; }));
	BOOST_TEST(count(begin, end, Individual(Chromosome("axc"), *m_fitnessMetric)) >= 2);
	BOOST_TEST(count(begin, end, Individual(Chromosome("fcL"), *m_fitnessMetric)) >= 2);
}


BOOST_AUTO_TEST_SUITE_END()
BOOST_AUTO_TEST_SUITE(ProgramCacheFactoryTest)

BOOST_FIXTURE_TEST_CASE(build_should_create_cache_for_each_input_program_if_cache_enabled, FixtureWithPrograms)
{
	ProgramCacheFactory::Options options{/* programCacheEnabled = */ true};
	vector<shared_ptr<ProgramCache>> caches = ProgramCacheFactory::build(options, m_programs);
	assert(m_programs.size() >= 2 && "There must be at least 2 programs for this test to be meaningful");

	BOOST_TEST(caches.size() == m_programs.size());
	for (size_t i = 0; i < m_programs.size(); ++i)
	{
		BOOST_REQUIRE(caches[i] != nullptr);
		BOOST_TEST(toString(caches[i]->program()) == toString(m_programs[i]));
	}
}

BOOST_FIXTURE_TEST_CASE(build_should_return_nullptr_for_each_input_program_if_cache_disabled, FixtureWithPrograms)
{
	ProgramCacheFactory::Options options{/* programCacheEnabled = */ false};
	vector<shared_ptr<ProgramCache>> caches = ProgramCacheFactory::build(options, m_programs);
	assert(m_programs.size() >= 2 && "There must be at least 2 programs for this test to be meaningful");

	BOOST_TEST(caches.size() == m_programs.size());
	for (size_t i = 0; i < m_programs.size(); ++i)
		BOOST_TEST(caches[i] == nullptr);
}

BOOST_AUTO_TEST_SUITE_END()
BOOST_AUTO_TEST_SUITE(ProgramFactoryTest)

BOOST_AUTO_TEST_CASE(build_should_load_programs_from_files)
{
	TemporaryDirectory tempDir;
	vector<string> sources{"{}", "{{}}", "{{{}}}"};
	ProgramFactory::Options options{
		/* inputFiles = */ {
			(tempDir.path() / "program1.yul").string(),
			(tempDir.path() / "program2.yul").string(),
			(tempDir.path() / "program3.yul").string(),
		},
		/* prefix = */ "",
	};

	for (size_t i = 0; i < sources.size(); ++i)
	{
		ofstream tmpFile(options.inputFiles[i]);
		tmpFile << sources[i] << endl;
	}

	vector<Program> programs = ProgramFactory::build(options);

	BOOST_TEST(programs.size() == sources.size());
	for (size_t i = 0; i < sources.size(); ++i)
	{
		CharStream sourceStream(sources[i], options.inputFiles[i]);
		BOOST_TEST(toString(programs[i]) == toString(get<Program>(Program::load(sourceStream))));
	}
}

BOOST_AUTO_TEST_CASE(build_should_apply_prefix)
{
	TemporaryDirectory tempDir;
	ProgramFactory::Options options{
		/* inputFiles = */ {(tempDir.path() / "program1.yul").string()},
		/* prefix = */ "f",
	};

	CharStream nestedSource("{{{let x:= 1}}}", "");
	Program nestedProgram = get<Program>(Program::load(nestedSource));
	Program flatProgram = get<Program>(Program::load(nestedSource));
	flatProgram.optimise(Chromosome::genesToSteps("f"));
	assert(toString(nestedProgram) != toString(flatProgram));

	{
		ofstream tmpFile(options.inputFiles[0]);
		tmpFile << nestedSource.source() << endl;
	}

	vector<Program> programs = ProgramFactory::build(options);

	BOOST_TEST(programs.size() == 1);
	BOOST_TEST(toString(programs[0]) == toString(flatProgram));
}

BOOST_AUTO_TEST_SUITE_END()
BOOST_AUTO_TEST_SUITE_END()
BOOST_AUTO_TEST_SUITE_END()

}
