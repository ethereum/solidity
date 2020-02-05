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

#include <test/CommonSyntaxTest.h>
#include <libyul/Dialect.h>

namespace solidity::yul::test
{

using solidity::test::SyntaxTestError;

class SyntaxTest: public solidity::test::CommonSyntaxTest
{
public:
	static std::unique_ptr<TestCase> create(Config const& _config)
	{
		return std::make_unique<SyntaxTest>(_config.filename, _config.evmVersion);
	}
	static std::unique_ptr<TestCase> createErrorRecovery(Config const& _config)
	{
		return std::make_unique<SyntaxTest>(_config.filename, _config.evmVersion);
	}
	SyntaxTest(std::string const& _filename, langutil::EVMVersion _evmVersion):
		CommonSyntaxTest(_filename, _evmVersion) {}
	virtual ~SyntaxTest() {}

	/// Validates the settings, i.e. moves them from m_settings to m_validatedSettings.
	/// Throws a runtime exception if any setting is left at this class (i.e. unknown setting).
	/// Returns true, if the test case is supported in the current environment and false
	/// otherwise which causes this test to be skipped.
	/// This might check e.g. for restrictions on the EVM version.
	bool validateSettings(langutil::EVMVersion _evmVersion) override;
protected:
	void parseAndAnalyze() override;
};

}
