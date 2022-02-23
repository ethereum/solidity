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

#include <test/EVMHost.h>

#include <libyul/AssemblyStack.h>

#include <libsolidity/interface/OptimiserSettings.h>

#include <liblangutil/DebugInfoSelection.h>

namespace solidity::test::fuzzer
{
class YulAssembler
{
public:
	YulAssembler(
		langutil::EVMVersion _version,
		solidity::frontend::OptimiserSettings _optSettings,
		std::string const& _yulSource
	):
		m_stack(
			_version,
			solidity::yul::AssemblyStack::Language::StrictAssembly,
			_optSettings,
			langutil::DebugInfoSelection::All()
		),
		m_yulProgram(_yulSource),
		m_optimiseYul(_optSettings.runYulOptimiser)
	{}
	solidity::bytes assemble();
private:
	solidity::yul::AssemblyStack m_stack;
	std::string m_yulProgram;
	bool m_optimiseYul;
};

struct YulEvmoneUtility
{
	/// @returns the result of deploying bytecode @param _input on @param _host.
	static evmc::result deployCode(solidity::bytes const& _input, EVMHost& _host);
	/// @returns call message to be sent to @param _address.
	static evmc_message callMessage(evmc_address _address);
	/// @returns true if call result indicates a serious error, false otherwise.
	static bool seriousCallError(evmc_status_code _code);
};
}
