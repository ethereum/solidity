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
 * Full assembly stack that can support EVM-assembly and JULIA as input and EVM, EVM1.5 and
 * eWasm as output.
 */

#pragma once

#include <libsolidity/interface/Exceptions.h>
#include <libsolidity/inlineasm/AsmAnalysisInfo.h>
#include <libevmasm/LinkerObject.h>

#include <string>
#include <memory>

namespace dev
{
namespace solidity
{
class Scanner;
namespace assembly
{
struct AsmAnalysisInfo;
struct Block;
}

/*
 * Full assembly stack that can support EVM-assembly and JULIA as input and EVM, EVM1.5 and
 * eWasm as output.
 */
class MultiBackendAssemblyStack
{
public:
	enum class Input { JULIA, Assembly };
	enum class Machine { EVM, EVM15, eWasm };

	MultiBackendAssemblyStack(Input _input = Input::Assembly, Machine _targetMachine = Machine::EVM):
		m_input(_input),
		m_targetMachine(_targetMachine)
	{}

	/// @returns the scanner used during parsing
	Scanner const& scanner() const;

	/// Runs parsing and analysis steps, returns false if input cannot be assembled.
	bool parseAndAnalyze(std::string const& _sourceName, std::string const& _source);

	/// Run the assembly step (should only be called after parseAndAnalyze).
	eth::LinkerObject assemble();

	ErrorList const& errors() const { return m_errors; }

	/// Pretty-print the input after having parsed it.
	std::string print();

private:

	Input m_input = Input::Assembly;
	Machine m_targetMachine = Machine::EVM;

	std::shared_ptr<Scanner> m_scanner;

	bool m_analysisSuccessful = false;
	std::shared_ptr<assembly::Block> m_parserResult;
	std::shared_ptr<assembly::AsmAnalysisInfo> m_analysisInfo;
	ErrorList m_errors;
};

}
}
