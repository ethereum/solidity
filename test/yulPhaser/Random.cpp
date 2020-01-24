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

#include <tools/yulPhaser/Random.h>

#include <boost/test/unit_test.hpp>

#include <cassert>

using namespace std;

namespace solidity::phaser::test
{

BOOST_AUTO_TEST_SUITE(Phaser)
BOOST_AUTO_TEST_SUITE(RandomTest)

BOOST_AUTO_TEST_CASE(uniformRandomInt_returns_different_values_when_called_multiple_times)
{
	constexpr uint32_t numSamples = 1000;
	constexpr uint32_t numOutcomes = 100;

	vector<uint32_t> samples1;
	vector<uint32_t> samples2;
	for (uint32_t i = 0; i < numSamples; ++i)
	{
		samples1.push_back(uniformRandomInt(0, numOutcomes - 1));
		samples2.push_back(uniformRandomInt(0, numOutcomes - 1));
	}

	vector<uint32_t> counts1(numSamples, 0);
	vector<uint32_t> counts2(numSamples, 0);
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

BOOST_AUTO_TEST_CASE(binomialRandomInt_returns_different_values_when_called_multiple_times)
{
	constexpr uint32_t numSamples = 1000;
	constexpr uint32_t numTrials = 100;
	constexpr double successProbability = 0.6;

	vector<uint32_t> samples1;
	vector<uint32_t> samples2;
	for (uint32_t i = 0; i < numSamples; ++i)
	{
		samples1.push_back(binomialRandomInt(numTrials, successProbability));
		samples2.push_back(binomialRandomInt(numTrials, successProbability));
	}

	vector<uint32_t> counts1(numSamples, 0);
	vector<uint32_t> counts2(numSamples, 0);
	for (uint32_t i = 0; i < numSamples; ++i)
	{
		++counts1[samples1[i]];
		++counts2[samples2[i]];
	}

	// See remark for uniformRandomInt() above. Same applies here.
	BOOST_TEST(counts1 != counts2);
}

BOOST_AUTO_TEST_SUITE_END()
BOOST_AUTO_TEST_SUITE_END()

}
