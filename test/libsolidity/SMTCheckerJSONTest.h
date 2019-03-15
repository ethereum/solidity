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

#include <test/libsolidity/SyntaxTest.h>

#include <libdevcore/JSON.h>

#include <string>

namespace dev
{
namespace solidity
{
namespace test
{

class SMTCheckerTest: public SyntaxTest
{
public:
	static std::unique_ptr<TestCase> create(Config const& _config)
	{
		return std::unique_ptr<TestCase>(new SMTCheckerTest(_config.filename, _config.evmVersion));
	}
	SMTCheckerTest(std::string const& _filename, langutil::EVMVersion _evmVersion);

	bool run(std::ostream& _stream, std::string const& _linePrefix = "", bool const _formatted = false) override;

private:
	std::vector<std::string> hashesFromJson(Json::Value const& _jsonObj, std::string const& _auxInput, std::string const& _smtlib);
	Json::Value buildJson(std::string const& _extra);

	Json::Value m_smtResponses;
};

}
}
}
