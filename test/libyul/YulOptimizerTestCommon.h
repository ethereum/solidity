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

#pragma once

#include <libyul/optimiser/OptimiserStep.h>
#include <libyul/optimiser/NameDispenser.h>

#include <libyul/YulString.h>

#include <set>
#include <memory>

namespace solidity::yul
{
	struct AsmAnalysisInfo;
	struct Object;
	struct Dialect;
}

namespace solidity::yul::test
{
class YulOptimizerTestCommon
{
public:
	explicit YulOptimizerTestCommon(
		std::shared_ptr<Object> _obj,
		Dialect const& _dialect
	);
	/// Sets optimiser step to be run to @param
	/// _optimiserStep.
	void setStep(std::string const& _optimizerStep);
	/// Runs chosen optimiser step returning pointer
	/// to yul AST Block post optimisation.
	std::shared_ptr<Block> run();
	/// Runs chosen optimiser step returning true if
	/// successful, false otherwise.
	bool runStep();
	/// Returns the string name of a randomly chosen
	/// optimiser step.
	/// @param _seed is an unsigned integer that
	/// seeds the random selection.
	std::string randomOptimiserStep(unsigned _seed);
private:
	void disambiguate();
	void updateContext();

	std::string m_optimizerStep;

	Dialect const* m_dialect = nullptr;
	std::set<YulString> m_reservedIdentifiers;
	std::unique_ptr<NameDispenser> m_nameDispenser;
	std::unique_ptr<OptimiserStepContext> m_context;

	std::shared_ptr<Object> m_object;
	std::shared_ptr<Block> m_ast;
	std::shared_ptr<AsmAnalysisInfo> m_analysisInfo;
	std::map<std::string, std::function<void(void)>> m_namedSteps;
};

}
