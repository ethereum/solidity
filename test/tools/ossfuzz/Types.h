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
 * Convenience macros for Solidity type declarations.
 */

#pragma once

#include <boost/algorithm/string.hpp>
#include <boost/preprocessor.hpp>

#define BYTES_ENUM_ELEM(z, n, unused) BOOST_PP_COMMA_IF(BOOST_PP_DEC(n)) BOOST_PP_IF(BOOST_PP_NOT(BOOST_PP_EQUAL(n, 33)), BOOST_PP_CAT(BYTES, n), BYTES)
#define INTEGER_ENUM_ELEM(z, n, typeString) BOOST_PP_COMMA_IF(n) BOOST_PP_CAT(typeString, BOOST_PP_MUL(BOOST_PP_INC(n), 8))

#define ENUM_SWITCH_CASE(r, data, elem) \
	case data::elem: \
		return boost::algorithm::to_lower_copy(std::string(BOOST_PP_STRINGIZE(elem)));

#define TYPE_ENUM_DECLS(typeName, ...) \
enum class typeName: size_t \
{ \
	__VA_ARGS__ \
}; \
static std::string toString(typeName e) \
{ \
	switch(e) \
	{ \
	BOOST_PP_SEQ_FOR_EACH( \
			ENUM_SWITCH_CASE, \
			typeName, \
			BOOST_PP_VARIADIC_TO_SEQ(__VA_ARGS__) \
	) \
	} \
} \
static std::pair<typeName, std::string> BOOST_PP_CAT(indexed, BOOST_PP_CAT(typeName, Type))(size_t _index) \
{ \
    auto t = typeName(_index); \
	return std::pair(t, toString(t)); \
} \
static std::pair<typeName, std::string> BOOST_PP_CAT(random, BOOST_PP_CAT(typeName, Type))();

