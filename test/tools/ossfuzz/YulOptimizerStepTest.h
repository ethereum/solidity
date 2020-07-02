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

#pragma once

#include <libyul/optimiser/OptimiserStep.h>
#include <libyul/optimiser/NameDispenser.h>

#include <libyul/YulString.h>

#include <memory>
#include <set>

namespace solidity::langutil
{
class Scanner;
class Error;
using ErrorList = std::vector<std::shared_ptr<Error const>>;
class EVMVersion;
}

namespace solidity::yul
{
struct AsmAnalysisInfo;
struct Block;
struct Dialect;
struct Object;
}

namespace solidity::yul::test
{

class YulOptimizerStepTest
{
public:
	explicit YulOptimizerStepTest(
		std::shared_ptr<Object> _obj,
		Dialect const& _dialect,
		std::string const& _optimizerStep
	);
	std::shared_ptr<Block> run();

private:
	void disambiguate();
	void updateContext();

	std::string m_optimizerStep;

	Dialect const* m_dialect = nullptr;
	std::set<YulString> m_reservedIdentifiers;
	std::unique_ptr<NameDispenser> m_nameDispenser;
	std::unique_ptr<OptimiserStepContext> m_context;

	std::shared_ptr<Block> m_ast;
	std::shared_ptr<AsmAnalysisInfo> m_analysisInfo;
	std::shared_ptr<Object> m_object;
};

}
