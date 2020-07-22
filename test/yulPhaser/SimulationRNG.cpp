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

#include <tools/yulPhaser/SimulationRNG.h>

#include <boost/test/unit_test.hpp>

#include <cassert>

using namespace std;

namespace solidity::phaser::test
{

BOOST_AUTO_TEST_SUITE(Phaser, *boost::unit_test::label("nooptions"))
BOOST_AUTO_TEST_SUITE(RandomTest)

BOOST_AUTO_TEST_CASE(bernoulliTrial_should_produce_samples_with_right_expected_value_and_variance)
{
	SimulationRNG::reset(1);
	constexpr size_t numSamples = 10000;
	constexpr double successProbability = 0.4;
	constexpr double relativeTolerance = 0.05;

	// For bernoulli distribution with success probability p: EX = p, VarX = p(1 - p)
	constexpr double expectedValue = successProbability;
	constexpr double variance = successProbability * (1 - successProbability);

	vector<uint32_t> samples;
	for (uint32_t i = 0; i < numSamples; ++i)
		samples.push_back(static_cast<uint32_t>(SimulationRNG::bernoulliTrial(successProbability)));

	BOOST_TEST(abs(mean(samples) - expectedValue) < expectedValue * relativeTolerance);
	BOOST_TEST(abs(meanSquaredError(samples, expectedValue) - variance) < variance * relativeTolerance);
}

BOOST_AUTO_TEST_CASE(bernoulliTrial_can_be_reset)
{
	constexpr size_t numSamples = 10;
	constexpr double successProbability = 0.4;

	SimulationRNG::reset(1);
	vector<uint32_t> samples1;
	for (uint32_t i = 0; i < numSamples; ++i)
		samples1.push_back(static_cast<uint32_t>(SimulationRNG::bernoulliTrial(successProbability)));

	vector<uint32_t> samples2;
	for (uint32_t i = 0; i < numSamples; ++i)
		samples2.push_back(static_cast<uint32_t>(SimulationRNG::bernoulliTrial(successProbability)));

	SimulationRNG::reset(1);
	vector<uint32_t> samples3;
	for (uint32_t i = 0; i < numSamples; ++i)
		samples3.push_back(static_cast<uint32_t>(SimulationRNG::bernoulliTrial(successProbability)));

	SimulationRNG::reset(2);
	vector<uint32_t> samples4;
	for (uint32_t i = 0; i < numSamples; ++i)
		samples4.push_back(static_cast<uint32_t>(SimulationRNG::bernoulliTrial(successProbability)));

	BOOST_TEST(samples1 != samples2);
	BOOST_TEST(samples1 == samples3);
	BOOST_TEST(samples1 != samples4);
	BOOST_TEST(samples2 != samples3);
	BOOST_TEST(samples2 != samples4);
	BOOST_TEST(samples3 != samples4);
}

BOOST_AUTO_TEST_CASE(uniformInt_returns_different_values_when_called_multiple_times)
{
	SimulationRNG::reset(1);
	constexpr size_t numSamples = 1000;
	constexpr uint32_t minValue = 50;
	constexpr uint32_t maxValue = 80;
	constexpr double relativeTolerance = 0.05;

	// For uniform distribution from range a..b: EX = (a + b) / 2, VarX = ((b - a + 1)^2 - 1) / 12
	constexpr double expectedValue = (minValue + maxValue) / 2.0;
	constexpr double variance = ((maxValue - minValue + 1) * (maxValue - minValue + 1) - 1) / 12.0;

	vector<uint32_t> samples;
	for (uint32_t i = 0; i < numSamples; ++i)
		samples.push_back(SimulationRNG::uniformInt(minValue, maxValue));

	BOOST_TEST(abs(mean(samples) - expectedValue) < expectedValue * relativeTolerance);
	BOOST_TEST(abs(meanSquaredError(samples, expectedValue) - variance) < variance * relativeTolerance);
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

BOOST_AUTO_TEST_CASE(binomialInt_should_produce_samples_with_right_expected_value_and_variance)
{
	SimulationRNG::reset(1);
	constexpr size_t numSamples = 1000;
	constexpr uint32_t numTrials = 100;
	constexpr double successProbability = 0.2;
	constexpr double relativeTolerance = 0.05;

	// For binomial distribution with n trials and success probability p: EX = np, VarX = np(1 - p)
	constexpr double expectedValue = numTrials * successProbability;
	constexpr double variance = numTrials * successProbability * (1 - successProbability);

	vector<uint32_t> samples;
	for (uint32_t i = 0; i < numSamples; ++i)
		samples.push_back(SimulationRNG::binomialInt(numTrials, successProbability));

	BOOST_TEST(abs(mean(samples) - expectedValue) < expectedValue * relativeTolerance);
	BOOST_TEST(abs(meanSquaredError(samples, expectedValue) - variance) < variance * relativeTolerance);
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
