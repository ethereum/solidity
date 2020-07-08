// SPDX-License-Identifier: GPL-3.0
/**
 * List of experimental features.
 */

#pragma once

#include <map>
#include <set>

namespace solidity::frontend
{

enum class ExperimentalFeature
{
	ABIEncoderV2, // new ABI encoder that makes use of Yul
	SMTChecker,
	Test,
	TestOnlyAnalysis
};

static std::set<ExperimentalFeature> const ExperimentalFeatureWithoutWarning =
{
	ExperimentalFeature::ABIEncoderV2,
	ExperimentalFeature::SMTChecker,
	ExperimentalFeature::TestOnlyAnalysis,
};

static std::map<std::string, ExperimentalFeature> const ExperimentalFeatureNames =
{
	{ "ABIEncoderV2", ExperimentalFeature::ABIEncoderV2 },
	{ "SMTChecker", ExperimentalFeature::SMTChecker },
	{ "__test", ExperimentalFeature::Test },
	{ "__testOnlyAnalysis", ExperimentalFeature::TestOnlyAnalysis },
};

}
