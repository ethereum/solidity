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
/** @file Common.h
 * @author Gav Wood <i@gavwood.com>
 * @date 2014
 *
 * Very common stuff (i.e. that every other header needs except vector_ref.h).
 */

#pragma once

// way too many unsigned to size_t warnings in 32 bit build
#ifdef _M_IX86
#pragma warning(disable:4244)
#endif

#if _MSC_VER && _MSC_VER < 1900
#define _ALLOW_KEYWORD_MACROS
#define noexcept throw()
#endif

#ifdef __INTEL_COMPILER
#pragma warning(disable:3682) //call through incomplete class
#endif

#include <libsolutil/vector_ref.h>

#include <boost/version.hpp>

#include <cstdint>
#include <functional>
#include <map>
#include <string>
#include <utility>
#include <vector>

namespace solidity
{

// Binary data types.
using bytes = std::vector<uint8_t>;
using bytesRef = util::vector_ref<uint8_t>;
using bytesConstRef = util::vector_ref<uint8_t const>;

// Map types.
using StringMap = std::map<std::string, std::string>;

// String types.
using strings = std::vector<std::string>;

/// RAII utility class whose destructor calls a given function.
class ScopeGuard
{
public:
	explicit ScopeGuard(std::function<void(void)> _f): m_f(std::move(_f)) {}
	~ScopeGuard() { m_f(); }

private:
	std::function<void(void)> m_f;
};

/// RAII utility class that sets the value of a variable for the current scope and restores it to its old value
/// during its destructor.
template<typename V>
class ScopedSaveAndRestore
{
public:
	explicit ScopedSaveAndRestore(V& _variable, V&& _value): m_variable(_variable), m_oldValue(std::move(_value))
	{
		std::swap(m_variable, m_oldValue);
	}
	ScopedSaveAndRestore(ScopedSaveAndRestore const&) = delete;
	~ScopedSaveAndRestore() { std::swap(m_variable, m_oldValue); }
	ScopedSaveAndRestore& operator=(ScopedSaveAndRestore const&) = delete;

private:
	V& m_variable;
	V m_oldValue;
};

template<typename V, typename... Args>
ScopedSaveAndRestore(V, Args...) -> ScopedSaveAndRestore<V>;

}
