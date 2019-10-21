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

#include <test/libsolidity/util/TestFileParser.h>
#include <test/libsolidity/util/SoltestErrors.h>

#include <libsolidity/ast/Types.h>
#include <liblangutil/Exceptions.h>
#include <libdevcore/AnsiColorized.h>
#include <libdevcore/CommonData.h>

#include <json/json.h>

#include <iosfwd>
#include <numeric>
#include <stdexcept>
#include <string>
#include <vector>

namespace dev
{
namespace solidity
{
namespace test
{

/**
 * Represents a function call and the result it returned. It stores the call
 * representation itself, the actual byte result (if any) and a string representation
 * used for the interactive update routine provided by isoltest. It also provides
 * functionality to compare the actual result with the expectations attached to the
 * call object, as well as a way to reset the result if executed multiple times.
 */
class TestFunctionCall
{
public:
	TestFunctionCall(FunctionCall _call): m_call(std::move(_call)) {}

	/// Formats this function call test and applies the format that was detected during parsing.
	/// If _renderResult is false, the expected result of the call will be used, if it's true
	/// the actual result is used.
	/// If _highlight is false, it's formatted without colorized highlighting. If it's true, AnsiColorized is
	/// used to apply a colorized highlighting.
	/// If test expectations do not match, the contract ABI is consulted in order to get the
	/// right encoding for returned bytes, based on the parsed return types.
	/// Reports warnings and errors to the error reporter.
	std::string format(
		ErrorReporter& _errorReporter,
		std::string const& _linePrefix = "",
		bool const _renderResult = false,
		bool const _highlight = false
	) const;

	/// Overloaded version that passes an error reporter which is never used outside
	/// of this function.
	std::string format(
		std::string const& _linePrefix = "",
		bool const _renderResult = false,
		bool const _highlight = false
	) const
	{
		ErrorReporter reporter;
		return format(reporter, _linePrefix, _renderResult, _highlight);
	}

	/// Resets current results in case the function was called and the result
	/// stored already (e.g. if test case was updated via isoltest).
	void reset();

	FunctionCall const& call() const { return m_call; }
	void setFailure(const bool _failure) { m_failure = _failure; }
	void setRawBytes(const bytes _rawBytes) { m_rawBytes = _rawBytes; }
	void setContractABI(Json::Value _contractABI) { m_contractABI = std::move(_contractABI); }

private:
	/// Tries to format the given `bytes`, applying the detected ABI types that have be set for each parameter.
	/// Throws if there's a mismatch in the size of `bytes` and the desired formats that are specified
	/// in the ABI type.
	/// Reports warnings and errors to the error reporter.
	std::string formatBytesParameters(
		ErrorReporter& _errorReporter,
		bytes const& _bytes,
		std::string const& _signature,
		ParameterList const& _params,
		bool highlight = false,
		bool failure = false
	) const;

	/// Formats a given _bytes applying the _abiType.
	std::string formatBytesRange(
		bytes const& _bytes,
		ABIType const& _abiType
	) const;

	/// Formats a FAILURE plus additional parameters, if e.g. a revert message was returned.
	std::string formatFailure(
		ErrorReporter& _errorReporter,
		FunctionCall const& _call,
		bytes const& _output,
		bool _renderResult,
		bool _highlight
	) const;

	/// Formats the given parameters using their raw string representation.
	std::string formatRawParameters(
		ParameterList const& _params,
		std::string const& _linePrefix = ""
	) const;

	/// Compares raw expectations (which are converted to a byte representation before),
	/// and also the expected transaction status of the function call to the actual test results.
	bool matchesExpectation() const;

	/// Function call that has been parsed and which holds all parameters / expectations.
	FunctionCall m_call;
	/// Result of the actual call been made.
	bytes m_rawBytes = bytes{};
	/// Transaction status of the actual call. False in case of a REVERT or any other failure.
	bool m_failure = true;
	/// JSON object which holds the contract ABI and that is used to set the output formatting
	/// in the interactive update routine.
	Json::Value m_contractABI;
};

}
}
}
