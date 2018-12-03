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

#include <string>

namespace dev
{

/// Simple generic result that holds a value and an optional error message.
/// Results can be implicitly converted to and created from the type of
/// the value they hold.
///
/// Result<bool> check()
/// {
///		if (false)
///			return Result<bool>("Error message.")
///		return true;
/// }
///

template <class ResultType>
class Result
{
public:
	Result(ResultType _value): Result(_value, std::string{}) {}
	Result(std::string _error): Result(ResultType{}, _error) {}

	/// @{
	/// @name Wrapper functions
	/// Wrapper functions that provide implicit conversions to and explicit retrieval of
	/// the value this result holds.
	operator ResultType const&() const { return m_value; }
	ResultType& operator*() const { return m_value; }
	ResultType const& get() const { return m_value; }
	ResultType& get() { return m_value; }
	/// @}

	/// @returns the error message (can be empty).
	std::string const& error() const { return m_error; }

private:
	explicit Result(ResultType _value, std::string _error):
		m_value(std::move(_value)),
		m_error(std::move(_error))
	{}

	ResultType m_value;
	std::string m_error;
};

}
