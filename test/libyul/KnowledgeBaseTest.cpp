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
 * Unit tests for KnowledgeBase
 */

#include <test/Common.h>

#include <test/libyul/Common.h>

#include <libyul/Object.h>
#include <libyul/optimiser/KnowledgeBase.h>
#include <libyul/optimiser/SSAValueTracker.h>
#include <libyul/optimiser/DataFlowAnalyzer.h>
#include <libyul/backends/evm/EVMDialect.h>

#include <liblangutil/ErrorReporter.h>

#include <boost/test/unit_test.hpp>

using namespace std;
using namespace solidity::langutil;

namespace solidity::yul::test
{

class KnowledgeBaseTest
{
protected:
	KnowledgeBase constructKnowledgeBase(string const& _source)
	{
		ErrorList errorList;
		shared_ptr<Object> object;
		shared_ptr<AsmAnalysisInfo> analysisInfo;
		std::tie(object, analysisInfo) = yul::test::parse(_source, m_dialect, errorList);
		BOOST_REQUIRE(object && errorList.empty() && object->code);

		SSAValueTracker ssaValues;
		ssaValues(*object->code);
		for (auto const& [name, expression]: ssaValues.values())
			m_values[name].value = expression;

		return KnowledgeBase(m_dialect, m_values);
	}

	EVMDialect m_dialect{EVMVersion{}, true};
	map<YulString, AssignedValue> m_values;
};

BOOST_FIXTURE_TEST_SUITE(KnowledgeBase, KnowledgeBaseTest)

BOOST_AUTO_TEST_CASE(basic)
{
	yul::KnowledgeBase kb = constructKnowledgeBase(R"({
		let a := calldataload(0)
		let b := calldataload(0)
		let zero := 0
		let c := add(b, 0)
		let d := mul(b, 0)
		let e := sub(a, b)
	})");

	BOOST_CHECK(!kb.knownToBeDifferent("a"_yulstring, "b"_yulstring));
	BOOST_CHECK(kb.valueIfKnownConstant("zero"_yulstring) == u256(0));

}

BOOST_AUTO_TEST_SUITE_END()

}
