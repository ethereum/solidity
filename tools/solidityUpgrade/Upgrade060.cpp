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
#include <tools/solidityUpgrade/Upgrade060.h>
#include <tools/solidityUpgrade/SourceTransform.h>

#include <libsolidity/analysis/OverrideChecker.h>

#include <libyul/AsmData.h>

#include <regex>

using namespace std;
using namespace solidity;
using namespace solidity::frontend;
using namespace solidity::tools;

using Contracts = set<ContractDefinition const*, OverrideChecker::CompareByID>;

namespace
{

inline string appendOverride(
	FunctionDefinition const& _function,
	Contracts const& _expectedContracts
)
{
	auto location = _function.location();
	string upgradedCode;
	string overrideExpression = SourceGeneration::functionOverride(_expectedContracts);

	if (SourceAnalysis::hasVirtualKeyword(location))
		upgradedCode = SourceTransform::insertAfterKeyword(
			location,
			"virtual",
			overrideExpression
		);
	else if (SourceAnalysis::hasMutabilityKeyword(location))
		upgradedCode = SourceTransform::insertAfterKeyword(
			location,
			stateMutabilityToString(_function.stateMutability()),
			overrideExpression
		);
	else if (SourceAnalysis::hasVisibilityKeyword(location))
		upgradedCode = SourceTransform::insertAfterKeyword(
			location,
			Declaration::visibilityToString(_function.visibility()),
			overrideExpression
		);
	else
		upgradedCode = SourceTransform::insertAfterRightParenthesis(
			location,
			overrideExpression
		);

	return upgradedCode;
}

inline string appendVirtual(FunctionDefinition const& _function)
{
	auto location = _function.location();
	string upgradedCode;

	if (SourceAnalysis::hasMutabilityKeyword(location))
		upgradedCode = SourceTransform::insertAfterKeyword(
			location,
			stateMutabilityToString(_function.stateMutability()),
			"virtual"
		);
	else if (SourceAnalysis::hasVisibilityKeyword(location))
		upgradedCode = SourceTransform::insertAfterKeyword(
			location,
			Declaration::visibilityToString(_function.visibility()),
			"virtual"
		);
	else
		upgradedCode = SourceTransform::insertAfterRightParenthesis(
			_function.location(),
			"virtual"
		);

	return upgradedCode;
}

}

void AbstractContract::endVisit(ContractDefinition const& _contract)
{
	bool isFullyImplemented = _contract.annotation().unimplementedDeclarations.empty();

	if (
		!isFullyImplemented &&
		!_contract.abstract() &&
		!_contract.isInterface()
	)
		m_changes.emplace_back(
				UpgradeChange::Level::Safe,
				_contract.location(),
				SourceTransform::insertBeforeKeyword(_contract.location(), "contract", "abstract")
		);
}

void OverridingFunction::endVisit(ContractDefinition const& _contract)
{
	auto const& inheritedFunctions = m_overrideChecker.inheritedFunctions(_contract);

	for (auto const* function: _contract.definedFunctions())
	{
		Contracts expectedContracts;
		OverrideProxy proxy{function};

		if (!function->isConstructor())
		{
			/// Build list of contracts expected to be mentioned in the override list (if any).
			for (auto [begin, end] = inheritedFunctions.equal_range(proxy); begin != end; begin++)
				expectedContracts.insert(&begin->contract());

			/// Add override with contract list, if needed.
			if (!function->overrides() && expectedContracts.size() > 1)
				m_changes.emplace_back(
						UpgradeChange::Level::Safe,
						function->location(),
						appendOverride(*function, expectedContracts)
				);

			for (auto [begin, end] = inheritedFunctions.equal_range(proxy); begin != end; begin++)
			{
				auto& super = (*begin);
				auto functionType = FunctionType(*function).asExternallyCallableFunction(false);
				auto superType = super.functionType()->asExternallyCallableFunction(false);

				if (functionType && functionType->hasEqualParameterTypes(*superType))
				{
					/// If function does not specify override and no override with
					/// contract list was added before.
					if (!function->overrides() && expectedContracts.size() <= 1)
						m_changes.emplace_back(
								UpgradeChange::Level::Safe,
								function->location(),
								appendOverride(*function, expectedContracts)
						);
				}
			}
		}
	}
}

void VirtualFunction::endVisit(ContractDefinition const& _contract)
{
	auto const& inheritedFunctions = m_overrideChecker.inheritedFunctions(_contract);

	for (FunctionDefinition const* function: _contract.definedFunctions())
	{
		OverrideProxy proxy{function};

		if (!function->isConstructor())
		{
			if (
				!function->markedVirtual() &&
				!function->isImplemented() &&
				!function->virtualSemantics() &&
				function->visibility() > Visibility::Private
			)
			{
				m_changes.emplace_back(
						UpgradeChange::Level::Safe,
						function->location(),
						appendVirtual(*function)
				);
			}

			for (auto [begin, end] = inheritedFunctions.equal_range(proxy); begin != end; begin++)
			{
				auto& super = (*begin);
				if (
					!function->markedVirtual() &&
					!super.virtualSemantics()
				)
				{
					m_changes.emplace_back(
							UpgradeChange::Level::Safe,
							function->location(),
							appendVirtual(*function)
					);
				}
			}
		}
	}
}
