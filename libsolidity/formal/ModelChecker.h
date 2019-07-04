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
 * Entry point to the model checking engines.
 * The goal of this class is to make different
 * engines share knowledge to boost their proving power.
 */

#pragma once

#include <libsolidity/formal/BMC.h>
#include <libsolidity/formal/CHC.h>
#include <libsolidity/formal/EncodingContext.h>

#include <libsolidity/interface/ReadFile.h>
#include <liblangutil/ErrorReporter.h>
#include <liblangutil/Scanner.h>

namespace langutil
{
class ErrorReporter;
struct SourceLocation;
}

namespace dev
{
namespace solidity
{

class ModelChecker
{
public:
	ModelChecker(langutil::ErrorReporter& _errorReporter, std::map<h256, std::string> const& _smtlib2Responses);

	void analyze(SourceUnit const& _sources, std::shared_ptr<langutil::Scanner> const& _scanner);

	/// This is used if the SMT solver is not directly linked into this binary.
	/// @returns a list of inputs to the SMT solver that were not part of the argument to
	/// the constructor.
	std::vector<std::string> unhandledQueries();

private:
	/// Bounded Model Checker engine.
	BMC m_bmc;

	/// Constrained Horn Clauses engine.
	CHC m_chc;

	/// Stores the context of the encoding.
	smt::EncodingContext m_context;
};

}
}
