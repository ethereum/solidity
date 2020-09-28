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

#pragma once

#include <libsolutil/Assertions.h>
#include <libsolutil/Exceptions.h>

#include <memory>
#include <optional>
#include <utility>

namespace solidity::util
{

DEV_SIMPLE_EXCEPTION(BadSetOnceReassignment);
DEV_SIMPLE_EXCEPTION(BadSetOnceAccess);

/// A class that stores a value that can only be set once
/// \tparam T the type of the stored value
template<typename T>
class SetOnce
{
public:
	/// Initializes the class to have no stored value.
	SetOnce() = default;

	// Not copiable
	SetOnce(SetOnce const&) = delete;
	SetOnce(SetOnce&&) = delete;

	// Not movable
	SetOnce& operator=(SetOnce const&) = delete;
	SetOnce& operator=(SetOnce&&) = delete;

	/// @brief Sets the stored value to \p _newValue
	/// @throws BadSetOnceReassignment when the stored value has already been set
	/// @return `*this`
	constexpr SetOnce& operator=(T _newValue) &
	{
		assertThrow(
			!m_value.has_value(),
			BadSetOnceReassignment,
			"Attempt to reassign to a SetOnce that already has a value."
		);

		m_value.emplace(std::move(_newValue));
		return *this;
	}

	/// @return A reference to the stored value. The returned reference has the same lifetime as `*this`.
	/// @throws BadSetOnceAccess when the stored value has not yet been set
	T const& operator*() const
	{
		assertThrow(
			m_value.has_value(),
			BadSetOnceAccess,
			"Attempt to access the value of a SetOnce that does not have a value."
		);

		return m_value.value();
	}

	/// @return A reference to the stored value. The referent of the returned pointer has the same lifetime as `*this`.
	/// @throws BadSetOnceAccess when the stored value has not yet been set
	T const* operator->() const { return std::addressof(**this); }

	/// @return true if a value was assigned
	bool set() const { return m_value.has_value(); }
private:
	std::optional<T> m_value = std::nullopt;
};

}
