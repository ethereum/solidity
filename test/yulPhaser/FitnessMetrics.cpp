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

#include <boost/test/unit_test.hpp>

using namespace std;
using namespace solidity::langutil;
using namespace solidity::yul;

namespace solidity::phaser::test
{

class FitnessMetricFixture
{
protected:
	FitnessMetricFixture():
		m_sourceStream(SampleSourceCode, ""),
		m_program(Program::load(m_sourceStream)) {}

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

	CharStream m_sourceStream;
	Program m_program;
};

BOOST_AUTO_TEST_SUITE(Phaser)
BOOST_AUTO_TEST_SUITE(FitnessMetricsTest)
BOOST_AUTO_TEST_SUITE(ProgramSizeTest)

BOOST_FIXTURE_TEST_CASE(evaluate_should_compute_size_of_the_optimised_program, FitnessMetricFixture)
{
	Chromosome chromosome(vector<string>{UnusedPruner::name, EquivalentFunctionCombiner::name});

	Program optimisedProgram = m_program;
	optimisedProgram.optimise(chromosome.optimisationSteps());
	assert(m_program.codeSize() != optimisedProgram.codeSize());

	BOOST_TEST(ProgramSize(m_program).evaluate(chromosome) != m_program.codeSize());
	BOOST_TEST(ProgramSize(m_program).evaluate(chromosome) == optimisedProgram.codeSize());
}

BOOST_AUTO_TEST_SUITE_END()
BOOST_AUTO_TEST_SUITE_END()
BOOST_AUTO_TEST_SUITE_END()

}

