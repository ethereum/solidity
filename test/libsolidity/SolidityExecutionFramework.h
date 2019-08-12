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
 * @author Christian <c@ethdev.com>
 * @date 2014
 * Framework for executing Solidity contracts and testing them against C++ implementation.
 */

#pragma once

#include <functional>

#include <test/ExecutionFramework.h>

#include <libsolidity/interface/CompilerStack.h>

#include <libyul/AssemblyStack.h>

#include <liblangutil/Exceptions.h>
#include <liblangutil/SourceReferenceFormatter.h>

namespace dev
{
namespace solidity
{

namespace test
{

class SolidityExecutionFramework: public dev::test::ExecutionFramework
{

public:
	SolidityExecutionFramework() {}
	explicit SolidityExecutionFramework(langutil::EVMVersion _evmVersion):
		ExecutionFramework(_evmVersion)
	{}

	virtual bytes const& compileAndRunWithoutCheck(
		std::string const& _sourceCode,
		u256 const& _value = 0,
		std::string const& _contractName = "",
		bytes const& _arguments = bytes(),
		std::map<std::string, dev::test::Address> const& _libraryAddresses = std::map<std::string, dev::test::Address>()
	) override
	{
		bytes bytecode = compileContract(_sourceCode, _contractName, _libraryAddresses);
		sendMessage(bytecode + _arguments, true, _value);
		return m_output;
	}

	bytes compileContract(
		std::string const& _sourceCode,
		std::string const& _contractName = "",
		std::map<std::string, dev::test::Address> const& _libraryAddresses = std::map<std::string, dev::test::Address>()
	);

protected:
	dev::solidity::CompilerStack m_compiler;
	bool m_compileViaYul = false;
};

}
}
} // end namespaces

