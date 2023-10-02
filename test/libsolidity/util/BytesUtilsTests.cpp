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

#include <test/libsolidity/util/BytesUtils.h>

#include <libsolutil/CommonData.h>

using namespace solidity::util;
using namespace solidity::test;

namespace solidity::frontend::test
{

BOOST_AUTO_TEST_SUITE(BytesUtilsTest)

BOOST_AUTO_TEST_CASE(format_fixed)
{
	BOOST_CHECK_EQUAL(
		BytesUtils::formatFixedPoint(toBigEndian(u256{0}), true, 2),
		"0.00"
	);
	BOOST_CHECK_EQUAL(
		BytesUtils::formatFixedPoint(toBigEndian(u256{1}), true, 2),
		"0.01"
	);
	BOOST_CHECK_EQUAL(
		BytesUtils::formatFixedPoint(toBigEndian(u256{123}), true, 2),
		"1.23"
	);
	BOOST_CHECK_EQUAL(
		BytesUtils::formatFixedPoint(toBigEndian(u256{-1}), true, 2),
		"-0.01"
	);
	BOOST_CHECK_EQUAL(
		BytesUtils::formatFixedPoint(toBigEndian(u256{-12}), true, 2),
		"-0.12"
	);
	BOOST_CHECK_EQUAL(
		BytesUtils::formatFixedPoint(toBigEndian(u256{-123}), true, 2),
		"-1.23"
	);
	BOOST_CHECK_EQUAL(
		BytesUtils::formatFixedPoint(toBigEndian(u256{-1234}), true, 2),
		"-12.34"
	);
	BOOST_CHECK_EQUAL(
		BytesUtils::formatFixedPoint(toBigEndian(u256{-12345}), true, 2),
		"-123.45"
	);
	BOOST_CHECK_EQUAL(
		BytesUtils::formatFixedPoint(toBigEndian(u256{-123456}), true, 2),
		"-1234.56"
	);
	BOOST_CHECK_EQUAL(
		BytesUtils::formatFixedPoint(toBigEndian(u256{-1234567}), true, 2),
		"-12345.67"
	);
}



BOOST_AUTO_TEST_SUITE_END()

}
