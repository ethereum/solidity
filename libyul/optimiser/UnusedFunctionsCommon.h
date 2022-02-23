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

#include <libyul/optimiser/Metrics.h>
#include <libyul/optimiser/NameDispenser.h>

#include <libyul/AST.h>

namespace solidity::yul::unusedFunctionsCommon
{

/// Returns true if applying UnusedFunctionParameterPruner is not helpful or redundant because the
/// inliner will be able to handle it anyway.
inline bool tooSimpleToBePruned(FunctionDefinition const& _f)
{
	return _f.body.statements.size() <= 1 && CodeSize::codeSize(_f.body) <= 1;
}

/// Given a function definition `_original`, this function returns a 'linking' function that calls
/// `_originalFunctionName` (with reduced parameters and return values).
///
/// The parameter `_usedParametersAndReturnVariables` is a pair of boolean-vectors. Its `.first`
/// corresponds to function parameters and its `.second` corresponds to function return-variables. A
/// false value at index `i` means that the corresponding function parameter / return-variable at
/// index `i` is unused.
///
/// Example:
///
/// Let `_original` be the function `function f_1() -> y { }`. (In practice, this function usually cannot
/// be inlined and has parameters / return-variables that are unused.)
/// Let `_usedParametersAndReturnVariables` be `({}, {false})`
/// Let `_originalFunctionName` be `f`.
/// Let `_linkingFunctionName` be `f_1`.
///
/// Then the returned linking function would be `function f_1() -> y_1 { f() }`
FunctionDefinition createLinkingFunction(
	FunctionDefinition const& _original,
	std::pair<std::vector<bool>, std::vector<bool>> const& _usedParametersAndReturns,
	YulString const& _originalFunctionName,
	YulString const& _linkingFunctionName,
	NameDispenser& _nameDispenser
);

}
