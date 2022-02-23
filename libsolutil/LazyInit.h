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

#include <libsolutil/Assertions.h>
#include <libsolutil/Exceptions.h>

#include <memory>
#include <optional>
#include <type_traits>
#include <utility>

namespace solidity::util
{

DEV_SIMPLE_EXCEPTION(BadLazyInitAccess);

/**
 * A value that is initialized at some point after construction of the LazyInit. The stored value can only be accessed
 * while calling "init", which initializes the stored value (if it has not already been initialized).
 *
 * @tparam T the type of the stored value; may not be a function, reference, array, or void type; may be const-qualified.
 */
template<typename T>
class LazyInit
{
public:
	using value_type = T;

	static_assert(std::is_object_v<value_type>, "Function, reference, and void types are not supported");
	static_assert(!std::is_array_v<value_type>, "Array types are not supported.");
	static_assert(!std::is_volatile_v<value_type>, "Volatile-qualified types are not supported.");

	LazyInit() = default;

	LazyInit(LazyInit const&) = delete;
	LazyInit& operator=(LazyInit const&) = delete;

	// Move constructor must be overridden to ensure that moved-from object is left empty.
	constexpr LazyInit(LazyInit&& _other) noexcept:
		m_value(std::move(_other.m_value))
	{
		_other.m_value.reset();
	}

	LazyInit& operator=(LazyInit&& _other) noexcept
	{
		this->m_value.swap(_other.m_value);
		_other.m_value.reset();
	}

	template<typename F>
	value_type& init(F&& _fun)
	{
		doInit(std::forward<F>(_fun));
		return m_value.value();
	}

	template<typename F>
	value_type const& init(F&& _fun) const
	{
		doInit(std::forward<F>(_fun));
		return m_value.value();
	}

private:
	/// Although not quite logically const, this is marked const for pragmatic reasons. It doesn't change the platonic
	/// value of the object (which is something that is initialized to some computed value on first use).
	template<typename F>
	void doInit(F&& _fun) const
	{
		if (!m_value.has_value())
			m_value.emplace(std::forward<F>(_fun)());
	}

	mutable std::optional<value_type> m_value;
};

}
