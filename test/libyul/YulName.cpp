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

#include <boost/test/data/test_case.hpp>
#include <libyul/Dialect.h>
#include <libyul/YulName.h>
#include <libyul/backends/evm/EVMDialect.h>

using namespace solidity;
using namespace solidity::yul;

namespace solidity::yul::test
{

BOOST_AUTO_TEST_SUITE(YulName)

BOOST_AUTO_TEST_CASE(repository_with_blank_dialect)
{
	Dialect dialect{};
	YulNameRepository nameRepository(dialect);
	BOOST_CHECK(!nameRepository.isEvmDialect());
	BOOST_CHECK(nameRepository.typeCount() == 1);
	BOOST_CHECK(nameRepository.isType(nameRepository.emptyName()));
	BOOST_CHECK(nameRepository.labelOf(nameRepository.predefined().empty).value().empty());
	BOOST_CHECK(nameRepository.labelOf(nameRepository.predefined().datasize) == "datasize");
	BOOST_CHECK(nameRepository.labelOf(nameRepository.predefined().dataoffset) == "dataoffset");
	BOOST_CHECK(nameRepository.labelOf(nameRepository.predefined().selfdestruct) == "selfdestruct");
	BOOST_CHECK(nameRepository.labelOf(nameRepository.predefined().memoryguard) == "memoryguard");
	BOOST_CHECK(nameRepository.labelOf(nameRepository.predefined().eq) == "eq");
	BOOST_CHECK(nameRepository.labelOf(nameRepository.predefined().add) == "add");
	BOOST_CHECK(nameRepository.labelOf(nameRepository.predefined().sub) == "sub");
	BOOST_CHECK(nameRepository.labelOf(nameRepository.predefined().tstore) == "tstore");
	BOOST_CHECK(nameRepository.labelOf(nameRepository.predefined().placeholderZero) == "@ 0");
	BOOST_CHECK(nameRepository.labelOf(nameRepository.predefined().placeholderOne) == "@ 1");
	BOOST_CHECK(nameRepository.labelOf(nameRepository.predefined().placeholderThirtyTwo) == "@ 32");
	BOOST_CHECK(nameRepository.labelOf(nameRepository.predefined().verbatim) == "@ verbatim");
	BOOST_CHECK(nameRepository.predefined().defaultType == nameRepository.emptyName());
	BOOST_CHECK(nameRepository.predefined().boolType == nameRepository.emptyName());
}

BOOST_AUTO_TEST_CASE(repository_generate_labels_for_derived_types)
{
	Dialect dialect{};
	YulNameRepository nameRepository(dialect);
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

BOOST_AUTO_TEST_CASE(repository_with_evm_dialect)
{
	auto const& dialect = EVMDialect::strictAssemblyForEVM({});
	YulNameRepository nameRepository(dialect);
	BOOST_CHECK(nameRepository.isEvmDialect());
	BOOST_CHECK(nameRepository.typeCount() == 1);
	BOOST_CHECK(nameRepository.isType(nameRepository.emptyName()));

	auto const extractEVMFunction = [](auto const* function) { return function ? function->definition : nullptr; };
	auto const type = nameRepository.emptyName();
	auto typeLabel(std::string(nameRepository.labelOf(type).value()));
	BOOST_CHECK(extractEVMFunction(nameRepository.discardFunction(type)) == dialect.discardFunction(typeLabel));
	BOOST_CHECK(extractEVMFunction(nameRepository.equalityFunction(type)) == dialect.equalityFunction(typeLabel));;
	BOOST_CHECK(extractEVMFunction(nameRepository.booleanNegationFunction()) == dialect.booleanNegationFunction());
	BOOST_CHECK(extractEVMFunction(nameRepository.memoryStoreFunction(type)) == dialect.memoryStoreFunction(typeLabel));
	BOOST_CHECK(extractEVMFunction(nameRepository.memoryLoadFunction(type)) == dialect.memoryLoadFunction(typeLabel));
	BOOST_CHECK(extractEVMFunction(nameRepository.storageStoreFunction(type)) == dialect.storageStoreFunction(typeLabel));
	BOOST_CHECK(extractEVMFunction(nameRepository.storageLoadFunction(type)) == dialect.storageLoadFunction(typeLabel));
	BOOST_CHECK(nameRepository.labelOf(nameRepository.hashFunction(type)) == dialect.hashFunction(typeLabel));
}

BOOST_AUTO_TEST_CASE(repository_with_typed_evm_dialect)
{
	auto const& dialect = EVMDialectTyped::instance({});
	YulNameRepository nameRepository(dialect);
	BOOST_CHECK(nameRepository.isEvmDialect());
	auto const wordType = nameRepository.nameOfType("u256");
	auto const boolType = nameRepository.nameOfType("bool");
	BOOST_CHECK(wordType == nameRepository.predefined().defaultType);
	BOOST_CHECK(boolType == nameRepository.predefined().boolType);
	BOOST_CHECK(nameRepository.labelOf(wordType) == "u256");
	BOOST_CHECK(nameRepository.labelOf(boolType) == "bool");
	BOOST_CHECK(nameRepository.isType(wordType));
	BOOST_CHECK(nameRepository.isType(boolType));
	BOOST_CHECK(!nameRepository.isType(nameRepository.emptyName()));
	BOOST_CHECK(nameRepository.typeCount() == 2);

	auto const extractEVMFunction = [](auto const* function) { return function ? function->definition : nullptr; };

	for (auto const type : {wordType, boolType})
	{
		auto typeLabel(std::string(nameRepository.labelOf(type).value()));
		BOOST_CHECK(extractEVMFunction(nameRepository.discardFunction(type)) == dialect.discardFunction(typeLabel));
		BOOST_CHECK(extractEVMFunction(nameRepository.equalityFunction(type)) == dialect.equalityFunction(typeLabel));
		BOOST_CHECK(extractEVMFunction(nameRepository.booleanNegationFunction()) == dialect.booleanNegationFunction());
		BOOST_CHECK(extractEVMFunction(nameRepository.memoryStoreFunction(type)) == dialect.memoryStoreFunction(typeLabel));
		BOOST_CHECK(extractEVMFunction(nameRepository.memoryLoadFunction(type)) == dialect.memoryLoadFunction(typeLabel));
		BOOST_CHECK(extractEVMFunction(nameRepository.storageStoreFunction(type)) == dialect.storageStoreFunction(typeLabel));
		BOOST_CHECK(extractEVMFunction(nameRepository.storageLoadFunction(type)) == dialect.storageLoadFunction(typeLabel));
		BOOST_CHECK(nameRepository.labelOf(nameRepository.hashFunction(type)) == dialect.hashFunction(typeLabel));
	}
}

BOOST_AUTO_TEST_CASE(verbatim_functions)
{
	auto const& dialect = EVMDialect::strictAssemblyForEVMObjects({});
	YulNameRepository nameRepository(dialect);
	auto const verbatimName = nameRepository.defineName("verbatim_5i_3o");
	BOOST_CHECK(!verbatimName.empty());
	BOOST_CHECK(nameRepository.isBuiltinName(verbatimName));
	BOOST_CHECK(nameRepository.isDerivedName(verbatimName));
	BOOST_CHECK(nameRepository.baseNameOf(verbatimName) == nameRepository.predefined().verbatim);
	BOOST_CHECK(nameRepository.builtin(verbatimName)->definition == dialect.builtin("verbatim_5i_3o"));
}

BOOST_AUTO_TEST_SUITE_END()

}
