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
#include <tools/solidityUpgrade/Upgrade050.h>
#include <tools/solidityUpgrade/SourceTransform.h>

#include <libsolidity/analysis/OverrideChecker.h>

#include <libyul/AST.h>

#include <regex>

using namespace std;
using namespace solidity;
using namespace solidity::frontend;
using namespace solidity::tools;

void ConstructorKeyword::endVisit(ContractDefinition const& _contract)
{
	for (auto const* function: _contract.definedFunctions())
		if (function->name() == _contract.name())
			m_changes.emplace_back(
					UpgradeChange::Level::Safe,
					function->location(),
					SourceTransform{m_charStreamProvider}.replaceFunctionName(
						function->location(),
						function->name(),
						"constructor"
					)
			);
}

void VisibilitySpecifier::endVisit(FunctionDefinition const& _function)
{
	if (_function.noVisibilitySpecified())
		m_changes.emplace_back(
				UpgradeChange::Level::Safe,
				_function.location(),
				SourceTransform{m_charStreamProvider}.insertAfterRightParenthesis(_function.location(), "public")
		);
}
