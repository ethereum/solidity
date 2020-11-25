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

#include <libsolutil/AnsiColorized.h>
#include <test/TestCase.h>

#include <iosfwd>
#include <string>
#include <vector>
#include <utility>

namespace solidity::frontend
{
class CompilerStack;
}

namespace solidity::frontend::test
{

class ASTJSONTest: public TestCase
{
public:
	static std::unique_ptr<TestCase> create(Config const& _config)
	{
		return std::make_unique<ASTJSONTest>(_config.filename);
	}
	ASTJSONTest(std::string const& _filename);

	TestResult run(std::ostream& _stream, std::string const& _linePrefix = "", bool const _formatted = false) override;

	void printSource(std::ostream& _stream, std::string const& _linePrefix = "", bool const _formatted = false) const override;
	void printUpdatedExpectations(std::ostream& _stream, std::string const& _linePrefix) const override;
private:
	bool runTest(
		std::string& _expectation,
		std::string& _result,
		std::map<std::string, unsigned> const& _sourceIndices,
		CompilerStack& _compiler,
		std::string const& _variation,
		std::ostream& _stream,
		std::string const& _linePrefix = "",
		bool const _formatted = false
	);
	void updateExpectation(
		std::string const& _filename,
		std::string const& _expectation,
		std::string const& _variation
	) const;

	std::vector<std::pair<std::string, std::string>> m_sources;
	std::string m_expectationParseOnly;
	std::string m_astFilename;
	std::string m_astParseOnlyFilename;
	std::string m_result;
	std::string m_resultParseOnly;
};

}
