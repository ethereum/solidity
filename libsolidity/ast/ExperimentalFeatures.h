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
	TestOnlyAnalysis,
	Solidity
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
	{ "solidity", ExperimentalFeature::Solidity }
};

}
