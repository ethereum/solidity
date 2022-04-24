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
#include <type_traits>

/**
 * Generates a meta function `trait` to check existence of `method` with signature $sign in the given class `U`
 *
 */
#define DEFINE_METHOD_CHECK(trait, method, signature)                                   \
	template<typename U>                                                                \
	class trait                                                                         \
	{                                                                                   \
	private:                                                                            \
		template<typename T, T>                                                         \
		struct helper;                                                                  \
                                                                                        \
		template<typename T>                                                            \
		static std::true_type check(helper<signature, &T::method>*);                    \
                                                                                        \
		template<typename T>                                                            \
		static std::false_type check(...);                                              \
                                                                                        \
	public:                                                                             \
		static constexpr bool value = sizeof(check<U>(0)) == sizeof(std::true_type);    \
	}
