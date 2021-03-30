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
 * Component that verifies properties at contract level, after the type checker has run.
 */

#pragma once

#include <libsolidity/ast/ASTForward.h>

namespace solidity::langutil
{
class ErrorReporter;
}

namespace solidity::frontend
{

/**
 * Component that verifies properties at contract level, after the type checker has run.
 */
class PostTypeContractLevelChecker
{
public:
	explicit PostTypeContractLevelChecker(langutil::ErrorReporter& _errorReporter):
		m_errorReporter(_errorReporter)
	{}

	/// Performs checks on the given source ast.
	/// @returns true iff all checks passed. Note even if all checks passed, errors() can still contain warnings
	bool check(SourceUnit const& _sourceUnit);

private:
	/// Performs checks on the given contract.
	/// @returns true iff all checks passed. Note even if all checks passed, errors() can still contain warnings
	bool check(ContractDefinition const& _contract);

	langutil::ErrorReporter& m_errorReporter;
};

}
