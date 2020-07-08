// SPDX-License-Identifier: GPL-3.0
#include <tools/solidityUpgrade/Upgrade050.h>
#include <tools/solidityUpgrade/SourceTransform.h>

#include <libsolidity/analysis/OverrideChecker.h>

#include <libyul/AsmData.h>

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
					SourceTransform::replaceFunctionName(
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
				SourceTransform::insertAfterRightParenthesis(_function.location(), "public")
		);
}
