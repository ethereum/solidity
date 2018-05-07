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
///			return Result<bool>(false, "Error message.")
///		return true;
/// }
///
///	A result can also be instantiated using one of the factory methods it provides:
///
/// using BoolResult = Result<bool>;
/// BoolResult check()
/// {
///		if (false)
///			return BoolResult::failure("Error message");
///		return BoolResult::success(true);
/// }
///
///
struct ResultError { std::string reason; };

template <class ResultType>
class Result
{
public:
	/// @{
	/// @name Factory functions
	/// Factory functions that provide a verbose way to create a result
	static Result<ResultType> Ok(ResultType _value) { return Result(std::move(_value)); }
	static Result<ResultType> Err() { return Result(ResultError{std::string()}); }
	static Result<ResultType> Err(std::string _error) { return Result(ResultError{std::move(_error)}); }
	/// @}

	Result(ResultType _value): m_value(std::move(_value)) {}
	Result(ResultError _error): m_error(std::move(_error.reason)) {}
	Result(ResultType _value, ResultError _error): m_value(std::move(_value)), m_error(std::move(_error.reason)) {}

	/// @{
	/// @name Wrapper functions
	/// Wrapper functions that provide implicit conversions to and explicit retrieval of
	/// the value this result holds.
	/// If the result is an error, accessing the value results in undefined behaviour.
	operator ResultType const&() const { return m_value; }
	ResultType& operator*() const { return m_value; }
	ResultType const& get() const { return m_value; }
	ResultType& get() { return m_value; }
	/// @}

	/// @returns the error message (can be empty).
	std::string const& err() const { return m_error; }

private:
	ResultType m_value;
	std::string m_error;
};

}
