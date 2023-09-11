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
 * Unit tests for the Natspec userdoc and devdoc JSON output.
 */

#pragma once

#include <test/libsolidity/SyntaxTest.h>

#include <libsolutil/JSON.h>

#include <istream>
#include <map>
#include <memory>
#include <ostream>
#include <string>
#include <string_view>
#include <tuple>

namespace solidity::frontend::test
{

enum class NatspecJSONKind
{
	Devdoc,
	Userdoc,
};

std::ostream& operator<<(std::ostream& _output, NatspecJSONKind _kind);

using NatspecMap = std::map<std::string, std::map<NatspecJSONKind, Json::Value>>;
using SerializedNatspecMap = std::map<std::string, std::map<NatspecJSONKind, std::string>>;

class NatspecJSONTest: public SyntaxTest
{
public:

	static std::unique_ptr<TestCase> create(Config const& _config);

	NatspecJSONTest(std::string const& _filename, langutil::EVMVersion _evmVersion):
		SyntaxTest(
			_filename,
			_evmVersion,
			langutil::Error::Severity::Error // _minSeverity
		)
	{}

protected:
	void parseCustomExpectations(std::istream& _stream) override;
	bool expectationsMatch() override;
	void printExpectedResult(std::ostream& _stream, std::string const& _linePrefix, bool _formatted) const override;
	void printObtainedResult(std::ostream& _stream, std::string const& _linePrefix, bool _formatted) const override;

	NatspecMap m_expectedNatspecJSON;

private:
	static std::tuple<std::string_view, NatspecJSONKind> parseExpectationHeader(std::string_view _line);
	static std::string extractExpectationJSON(std::istream& _stream);
	static std::string_view expectLinePrefix(std::string_view _line);

	std::string formatNatspecExpectations(NatspecMap const& _expectations) const;
	SerializedNatspecMap prettyPrinted(NatspecMap const& _expectations) const;
	NatspecMap obtainedNatspec() const;
};

}
