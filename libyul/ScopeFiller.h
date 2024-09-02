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
 * Module responsible for registering identifiers inside their scopes.
 */

#pragma once

#include <libyul/ASTForward.h>

#include <functional>
#include <memory>

namespace solidity::langutil
{
class ErrorReporter;
struct SourceLocation;
}

namespace solidity::yul
{

struct NameWithDebugData;
struct Scope;
struct AsmAnalysisInfo;

/**
 * Fills scopes with identifiers and checks for name clashes.
 * Does not resolve references.
 */
class ScopeFiller
{
public:
	ScopeFiller(AsmAnalysisInfo& _info, langutil::ErrorReporter& _errorReporter);

	bool operator()(Literal const&) { return true; }
	bool operator()(Identifier const&) { return true; }
	bool operator()(ExpressionStatement const& _expr);
	bool operator()(Assignment const&) { return true; }
	bool operator()(VariableDeclaration const& _variableDeclaration);
	bool operator()(FunctionDefinition const& _functionDefinition);
	bool operator()(FunctionCall const&) { return true; }
	bool operator()(If const& _if);
	bool operator()(Switch const& _switch);
	bool operator()(ForLoop const& _forLoop);
	bool operator()(Break const&) { return true; }
	bool operator()(Continue const&) { return true; }
	bool operator()(Leave const&) { return true; }
	bool operator()(Block const& _block);

private:
	bool registerVariable(
		NameWithDebugData const& _name,
		langutil::SourceLocation const& _location,
		Scope& _scope
	);
	bool registerFunction(FunctionDefinition const& _funDef);

	Scope& scope(Block const* _block);

	Scope* m_currentScope = nullptr;
	AsmAnalysisInfo& m_info;
	langutil::ErrorReporter& m_errorReporter;
};

}
