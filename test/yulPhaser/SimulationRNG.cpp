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

#include <tools/yulPhaser/SimulationRNG.h>

#include <boost/test/unit_test.hpp>

#include <cassert>

using namespace std;

namespace solidity::phaser::test
{

BOOST_AUTO_TEST_SUITE(Phaser)
BOOST_AUTO_TEST_SUITE(RandomTest)

BOOST_AUTO_TEST_CASE(uniformInt_returns_different_values_when_called_multiple_times)
{
	constexpr uint32_t numSamples = 1000;
	constexpr uint32_t numOutcomes = 100;

	vector<uint32_t> samples1;
	vector<uint32_t> samples2;
	for (uint32_t i = 0; i < numSamples; ++i)
	{
		samples1.push_back(SimulationRNG::uniformInt(0, numOutcomes - 1));
		samples2.push_back(SimulationRNG::uniformInt(0, numOutcomes - 1));
	}

	vector<uint32_t> counts1(numOutcomes, 0);
	vector<uint32_t> counts2(numOutcomes, 0);
	for (uint32_t i = 0; i < numSamples; ++i)
	{
		++counts1[samples1[i]];
		++counts2[samples2[i]];
	}

	// This test rules out not only the possibility that the two sequences are the same but also
	// that they're just different permutations of the same values. The test is probabilistic so
	// it's technically possible for it to fail even if generator is good but the probability is
	// so low that it would happen on average once very 10^125 billion years if you repeated it
	// every second. The chance is much lower than 1 in 1000^100 / 100!.
	//
	// This does not really guarantee that the generated numbers have the right distribution or
	// or that they don't come in long, repeating sequences but the implementation is very simple
	// (it just calls a generator from boost) so our goal here is just to make sure it's used
	// properly and we're not getting something totally non-random, e.g. the same number every time.
	BOOST_TEST(counts1 != counts2);
}

BOOST_AUTO_TEST_CASE(uniformInt_can_be_reset)
{
	constexpr size_t numSamples = 10;
	constexpr uint32_t minValue = 50;
	constexpr uint32_t maxValue = 80;

	SimulationRNG::reset(1);
	vector<uint32_t> samples1;
	for (uint32_t i = 0; i < numSamples; ++i)
		samples1.push_back(SimulationRNG::uniformInt(minValue, maxValue));

	vector<uint32_t> samples2;
	for (uint32_t i = 0; i < numSamples; ++i)
		samples2.push_back(SimulationRNG::uniformInt(minValue, maxValue));

	SimulationRNG::reset(1);
	vector<uint32_t> samples3;
	for (uint32_t i = 0; i < numSamples; ++i)
		samples3.push_back(SimulationRNG::uniformInt(minValue, maxValue));

	SimulationRNG::reset(2);
	vector<uint32_t> samples4;
	for (uint32_t i = 0; i < numSamples; ++i)
		samples4.push_back(SimulationRNG::uniformInt(minValue, maxValue));

	BOOST_TEST(samples1 != samples2);
	BOOST_TEST(samples1 == samples3);
	BOOST_TEST(samples1 != samples4);
	BOOST_TEST(samples2 != samples3);
	BOOST_TEST(samples2 != samples4);
	BOOST_TEST(samples3 != samples4);
}

BOOST_AUTO_TEST_CASE(binomialInt_returns_different_values_when_called_multiple_times)
{
	constexpr uint32_t numSamples = 1000;
	constexpr uint32_t numTrials = 100;
	constexpr double successProbability = 0.6;

	vector<uint32_t> samples1;
	vector<uint32_t> samples2;
	for (uint32_t i = 0; i < numSamples; ++i)
	{
		samples1.push_back(SimulationRNG::binomialInt(numTrials, successProbability));
		samples2.push_back(SimulationRNG::binomialInt(numTrials, successProbability));
	}

	vector<uint32_t> counts1(numTrials, 0);
	vector<uint32_t> counts2(numTrials, 0);
	for (uint32_t i = 0; i < numSamples; ++i)
	{
		++counts1[samples1[i]];
		++counts2[samples2[i]];
	}

	// See remark for uniformInt() above. Same applies here.
	BOOST_TEST(counts1 != counts2);
}

BOOST_AUTO_TEST_CASE(binomialInt_can_be_reset)
{
	constexpr size_t numSamples = 10;
	constexpr uint32_t numTrials = 10;
	constexpr double successProbability = 0.6;

	SimulationRNG::reset(1);
	vector<uint32_t> samples1;
	for (uint32_t i = 0; i < numSamples; ++i)
		samples1.push_back(SimulationRNG::binomialInt(numTrials, successProbability));

	vector<uint32_t> samples2;
	for (uint32_t i = 0; i < numSamples; ++i)
		samples2.push_back(SimulationRNG::binomialInt(numTrials, successProbability));

	SimulationRNG::reset(1);
	vector<uint32_t> samples3;
	for (uint32_t i = 0; i < numSamples; ++i)
		samples3.push_back(SimulationRNG::binomialInt(numTrials, successProbability));

	SimulationRNG::reset(2);
	vector<uint32_t> samples4;
	for (uint32_t i = 0; i < numSamples; ++i)
		samples4.push_back(SimulationRNG::binomialInt(numTrials, successProbability));

	BOOST_TEST(samples1 != samples2);
	BOOST_TEST(samples1 == samples3);
	BOOST_TEST(samples1 != samples4);
	BOOST_TEST(samples2 != samples3);
	BOOST_TEST(samples2 != samples4);
	BOOST_TEST(samples3 != samples4);
}

BOOST_AUTO_TEST_SUITE_END()
BOOST_AUTO_TEST_SUITE_END()

}
