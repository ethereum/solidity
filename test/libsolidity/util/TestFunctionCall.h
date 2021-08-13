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
#include <test/libsolidity/util/ContractABIUtils.h>

#include <liblangutil/Exceptions.h>
#include <libsolutil/AnsiColorized.h>
#include <libsolutil/CommonData.h>
#include <libsolutil/JSON.h>

#include <json/json.h>

#include <iosfwd>
#include <numeric>
#include <stdexcept>
#include <string>
#include <vector>

namespace solidity::frontend::test
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
	enum class RenderMode
	{
		ExpectedValuesExpectedGas,
		ActualValuesExpectedGas,
		ExpectedValuesActualGas
	};

	TestFunctionCall(FunctionCall _call): m_call(std::move(_call)), m_gasCosts(m_call.expectations.gasUsed) {}

	/// Formats this function call test and applies the format that was detected during parsing.
	/// _renderMode determines the source of values to be inserted into the updated test expectations.
	///     RenderMode::ActualValuesExpectedGas: use the values produced by the test but for gas preserve the original expectations,
	///     RenderMode::ExpectedValuesExpectedGas: preserve the original expectations for both gas and other values,
	///     RenderMode::ExpectedValuesActualGas: use the original expectations but for gas use actual values,
	/// If _highlight is false, it's formatted without colorized highlighting. If it's true, AnsiColorized is
	/// used to apply a colorized highlighting.
	/// If _interactivePrint is true, we are producing output that will be interactively shown to the user
	/// in the terminal rather than used to update the expectations in the test file.
	/// If test expectations do not match, the contract ABI is consulted in order to get the
	/// right encoding for returned bytes, based on the parsed return types.
	/// Reports warnings and errors to the error reporter.
	std::string format(
		ErrorReporter& _errorReporter,
		std::string const& _linePrefix = "",
		RenderMode _renderMode = RenderMode::ExpectedValuesExpectedGas,
		bool const _highlight = false,
		bool const _interactivePrint = false
	) const;

	/// Overloaded version that passes an error reporter which is never used outside
	/// of this function.
	std::string format(
		std::string const& _linePrefix = "",
		RenderMode const _renderMode = RenderMode::ExpectedValuesExpectedGas,
		bool const _highlight = false
	) const
	{
		ErrorReporter reporter;
		return format(reporter, _linePrefix, _renderMode, _highlight);
	}

	/// Resets current results in case the function was called and the result
	/// stored already (e.g. if test case was updated via isoltest).
	void reset();

	FunctionCall const& call() const { return m_call; }
	void calledNonExistingFunction() { m_calledNonExistingFunction = true; }
	void setFailure(const bool _failure) { m_failure = _failure; }
	void setRawBytes(const bytes _rawBytes) { m_rawBytes = _rawBytes; }
	void setGasCost(std::string const& _runType, u256 const& _gasCost) { m_gasCosts[_runType] = _gasCost; }
	void setContractABI(Json::Value _contractABI) { m_contractABI = std::move(_contractABI); }
	void setSideEffects(std::vector<std::string> _sideEffects) { m_call.actualSideEffects = _sideEffects; }

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

	/// Formats gas usage expectations one per line
	std::string formatGasExpectations(
		std::string const& _linePrefix,
		bool _useActualCost,
		bool _showDifference
	) const;

	/// Compares raw expectations (which are converted to a byte representation before),
	/// and also the expected transaction status of the function call to the actual test results.
	bool matchesExpectation() const;

	/// Function call that has been parsed and which holds all parameters / expectations.
	FunctionCall m_call;
	/// Result of the actual call been made.
	bytes m_rawBytes = bytes{};
	/// Actual gas costs
	std::map<std::string, u256> m_gasCosts;
	/// Transaction status of the actual call. False in case of a REVERT or any other failure.
	bool m_failure = true;
	/// JSON object which holds the contract ABI and that is used to set the output formatting
	/// in the interactive update routine.
	Json::Value m_contractABI = Json::Value{};
	/// Flags that the test failed because the called function is not known to exist on the contract.
	bool m_calledNonExistingFunction = false;
};

}
