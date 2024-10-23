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

#include <boost/test/unit_test.hpp>

#include <libyul/Dialect.h>
#include <libyul/YulName.h>
#include <libyul/backends/evm/EVMDialect.h>

using namespace solidity;
using namespace solidity::yul;

namespace solidity::yul::test
{

BOOST_AUTO_TEST_SUITE(YulName)

BOOST_AUTO_TEST_CASE(repository_generate_labels_for_derived_types)
{
	Dialect dialect{};
	YulNameRepository nameRepository;
	auto const test1 = nameRepository.defineName("test1");
	BOOST_CHECK(!nameRepository.isDerivedName(test1));
	auto const test1_1 = nameRepository.deriveName(test1);
	BOOST_CHECK(nameRepository.isDerivedName(test1_1));
	auto const test1_2 = nameRepository.deriveName(test1);
	BOOST_CHECK(nameRepository.isDerivedName(test1_2));
	auto const test1_2_1 = nameRepository.deriveName(test1_2);
	BOOST_CHECK(nameRepository.isDerivedName(test1_2_1));
	auto const test2 = nameRepository.defineName("test2_1");
	auto const test2_1 = nameRepository.deriveName(test2);
	auto const test3 = nameRepository.defineName("test3");
	auto const test3_1 = nameRepository.deriveName(test3);
	BOOST_CHECK(test1 != test1_1);
	BOOST_CHECK(test1 < test1_1);
	nameRepository.generateLabels({test1, test1_1, test1_2, test1_2_1, test2_1, test3, test3_1}, {"test1"});

	// marking test1 as invalid means that all labels get bumped up by one
	BOOST_CHECK(nameRepository.labelOf(test1) == "test1_1");
	BOOST_CHECK(nameRepository.labelOf(test1_1) == "test1_2");
	BOOST_CHECK(nameRepository.labelOf(test1_2) == "test1_3");
	BOOST_CHECK(nameRepository.labelOf(test1_2_1) == "test1_4");

	BOOST_CHECK(nameRepository.labelOf(test2) == "test2_1");
	// the label of test2 is reused as it's not in the used names when generating labels
	BOOST_CHECK(nameRepository.labelOf(test2_1) == "test2_1");

	BOOST_CHECK(nameRepository.labelOf(test3) == "test3");
	BOOST_CHECK(nameRepository.labelOf(test3_1) == "test3_1");

	// derive a name from the (now labelled) test2_1 name
	auto const test2_1_1 = nameRepository.deriveName(test2_1);
	// but we have a conflict with an already defined/labelled name, expectation is that we get test2_1_2
	auto const conflict = nameRepository.defineName("test2_1_1");
	nameRepository.generateLabels({test1, test1_1, test1_2, test1_2_1, test2_1, test2_1_1, test3, test3_1, conflict});
	// test2_1 is in the list, so produce a new name
	BOOST_CHECK(nameRepository.labelOf(test2_1_1) == "test2_1_2");
	BOOST_CHECK(nameRepository.labelOf(conflict) == "test2_1_1");

	nameRepository.generateLabels({test2, test2_1, test2_1_1, conflict});
	BOOST_CHECK(nameRepository.labelOf(test2) == "test2_1");
	// this label gets reassigned, as test2_1 is back in the game
	BOOST_CHECK(nameRepository.labelOf(test2_1) == "test2_1_3");
	BOOST_CHECK(nameRepository.labelOf(test2_1_1) == "test2_1_2");
	BOOST_CHECK(nameRepository.labelOf(conflict) == "test2_1_1");

}

BOOST_AUTO_TEST_SUITE_END()

}
