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
/**
 * Module responsible for registering identifiers inside their scopes.
 */

#pragma once

#include <libyul/AsmDataForward.h>

#include <boost/variant.hpp>

#include <functional>
#include <memory>

namespace langutil
{
class ErrorReporter;
struct SourceLocation;
}

namespace dev
{
namespace solidity
{
namespace assembly
{

struct TypedName;
struct Scope;
struct AsmAnalysisInfo;

/**
 * Fills scopes with identifiers and checks for name clashes.
 * Does not resolve references.
 */
class ScopeFiller: public boost::static_visitor<bool>
{
public:
	ScopeFiller(AsmAnalysisInfo& _info, langutil::ErrorReporter& _errorReporter);

	bool operator()(assembly::Instruction const&) { return true; }
	bool operator()(assembly::Literal const&) { return true; }
	bool operator()(assembly::Identifier const&) { return true; }
	bool operator()(assembly::FunctionalInstruction const&) { return true; }
	bool operator()(assembly::ExpressionStatement const& _expr);
	bool operator()(assembly::Label const& _label);
	bool operator()(assembly::StackAssignment const&) { return true; }
	bool operator()(assembly::Assignment const&) { return true; }
	bool operator()(assembly::VariableDeclaration const& _variableDeclaration);
	bool operator()(assembly::FunctionDefinition const& _functionDefinition);
	bool operator()(assembly::FunctionCall const&) { return true; }
	bool operator()(assembly::If const& _if);
	bool operator()(assembly::Switch const& _switch);
	bool operator()(assembly::ForLoop const& _forLoop);
	bool operator()(assembly::Block const& _block);

private:
	bool registerVariable(
		TypedName const& _name,
		langutil::SourceLocation const& _location,
		Scope& _scope
	);

	Scope& scope(assembly::Block const* _block);

	Scope* m_currentScope = nullptr;
	AsmAnalysisInfo& m_info;
	langutil::ErrorReporter& m_errorReporter;
};

}
}
}
