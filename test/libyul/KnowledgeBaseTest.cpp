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
#include <libyul/optimiser/ASTCopier.h>
#include <libyul/optimiser/KnowledgeBase.h>
#include <libyul/optimiser/SSAValueTracker.h>
#include <libyul/optimiser/NameDispenser.h>
#include <libyul/optimiser/CommonSubexpressionEliminator.h>
#include <libyul/backends/evm/EVMDialect.h>

#include <liblangutil/ErrorReporter.h>

#include <boost/test/unit_test.hpp>

using namespace solidity::langutil;

namespace solidity::yul::test
{

class KnowledgeBaseTest
{
protected:
	KnowledgeBase constructKnowledgeBase(std::string const& _source)
	{
		ErrorList errorList;
		std::shared_ptr<AsmAnalysisInfo> analysisInfo;
		std::tie(m_object, analysisInfo) = yul::test::parse(_source, m_dialect, errorList);
		BOOST_REQUIRE(m_object && errorList.empty() && m_object->code);

		auto astRoot = std::get<Block>(yul::ASTCopier{}(m_object->code->root()));
		NameDispenser dispenser(m_dialect, astRoot);
		std::set<YulName> reserved;
		OptimiserStepContext context{m_dialect, dispenser, reserved, 0};
		CommonSubexpressionEliminator::run(context, astRoot);

		m_ssaValues(astRoot);
		for (auto const& [name, expression]: m_ssaValues.values())
			m_values[name].value = expression;

		m_object->code = std::make_shared<AST>(std::move(astRoot));
		return KnowledgeBase([this](YulName _var) { return util::valueOrNullptr(m_values, _var); });
	}

	EVMDialect m_dialect{EVMVersion{}, true};
	std::shared_ptr<Object> m_object;
	SSAValueTracker m_ssaValues;
	std::map<YulName, AssignedValue> m_values;
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

	BOOST_CHECK(!kb.knownToBeDifferent("a"_yulname, "b"_yulname));
	// This only works if the variable names are the same.
	// It assumes that SSA+CSE+Simplifier actually replaces the variables.
	BOOST_CHECK(!kb.valueIfKnownConstant("a"_yulname));
	BOOST_CHECK(kb.valueIfKnownConstant("zero"_yulname) == u256(0));
	BOOST_CHECK(kb.differenceIfKnownConstant("a"_yulname, "b"_yulname) == u256(0));
	BOOST_CHECK(kb.differenceIfKnownConstant("a"_yulname, "c"_yulname) == u256(0));
	BOOST_CHECK(kb.valueIfKnownConstant("e"_yulname) == u256(0));
}

BOOST_AUTO_TEST_CASE(difference)
{
	yul::KnowledgeBase kb = constructKnowledgeBase(R"({
		let a := calldataload(0)
		let b := add(a, 200)
		let c := add(a, 220)
		let d := add(12, c)
		let e := sub(c, 12)
	})");

	BOOST_CHECK(kb.differenceIfKnownConstant("c"_yulname, "b"_yulname) ==
		u256(20)
	);
	BOOST_CHECK(kb.differenceIfKnownConstant("b"_yulname, "c"_yulname) ==
		u256(-20)
	);
	BOOST_CHECK(!kb.knownToBeDifferentByAtLeast32("b"_yulname, "c"_yulname));
	BOOST_CHECK(kb.knownToBeDifferentByAtLeast32("b"_yulname, "d"_yulname));
	BOOST_CHECK(kb.knownToBeDifferentByAtLeast32("a"_yulname, "b"_yulname));
	BOOST_CHECK(kb.knownToBeDifferentByAtLeast32("b"_yulname, "a"_yulname));

	BOOST_CHECK(kb.differenceIfKnownConstant("e"_yulname, "a"_yulname) == u256(208));
	BOOST_CHECK(kb.differenceIfKnownConstant("e"_yulname, "b"_yulname) == u256(8));
	BOOST_CHECK(kb.differenceIfKnownConstant("a"_yulname, "e"_yulname) == u256(-208));
	BOOST_CHECK(kb.differenceIfKnownConstant("b"_yulname, "e"_yulname) == u256(-8));
}


BOOST_AUTO_TEST_SUITE_END()

}
