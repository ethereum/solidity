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

#include <test/libsolidity/util/TestFunctionCall.h>

#include <test/libsolidity/util/BytesUtils.h>
#include <test/libsolidity/util/ContractABIUtils.h>

#include <libsolutil/AnsiColorized.h>

#include <boost/algorithm/string.hpp>

#include <optional>
#include <stdexcept>
#include <string>

using namespace solidity;
using namespace solidity::util;
using namespace solidity::frontend::test;

using Token = soltest::Token;

std::string TestFunctionCall::format(
	ErrorReporter& _errorReporter,
	std::string const& _linePrefix,
	RenderMode _renderMode,
	bool const _highlight,
	bool const _interactivePrint
) const
{
	std::stringstream stream;

	bool highlight = !matchesExpectation() && _highlight;

	auto formatOutput = [&](bool const _singleLine)
	{
		std::string ws = " ";
		std::string arrow = formatToken(Token::Arrow);
		std::string colon = formatToken(Token::Colon);
		std::string comma = formatToken(Token::Comma);
		std::string comment = formatToken(Token::Comment);
		std::string ether = formatToken(Token::Ether);
		std::string wei = formatToken(Token::Wei);
		std::string newline = formatToken(Token::Newline);
		std::string failure = formatToken(Token::Failure);

		if (m_call.kind == FunctionCall::Kind::Library)
		{
			stream << _linePrefix << newline << ws << "library:" << ws;
			if (!m_call.libraryFile.empty())
				stream << "\"" << m_call.libraryFile << "\":";
			stream << m_call.signature;
			return;
		}

		/// Formats the function signature. This is the same independent from the display-mode.
		stream << _linePrefix << newline << ws << m_call.signature;
		if (m_call.value.value > u256(0))
		{
			switch (m_call.value.unit)
			{
			case FunctionValueUnit::Ether:
				stream << comma << ws << (m_call.value.value / exp256(10, 18)) << ws << ether;
				break;
			case FunctionValueUnit::Wei:
				stream << comma << ws << m_call.value.value << ws << wei;
				break;
			default:
				soltestAssert(false, "");
			}
		}
		if (!m_call.arguments.rawBytes().empty())
		{
			std::string output = formatRawParameters(m_call.arguments.parameters, _linePrefix);
			stream << colon;
			if (!m_call.arguments.parameters.at(0).format.newline)
				stream << ws;
			stream << output;
		}

		/// Formats comments on the function parameters and the arrow taking
		/// the display-mode into account.
		if (_singleLine)
		{
			if (!m_call.arguments.comment.empty())
				stream << ws << comment << m_call.arguments.comment << comment;

			if (m_call.omitsArrow)
			{
				if (_renderMode == RenderMode::ActualValuesExpectedGas && (m_failure || !matchesExpectation()))
					stream << ws << arrow;
			}
			else
				stream << ws << arrow;
		}
		else
		{
			stream << std::endl << _linePrefix << newline << ws;
			if (!m_call.arguments.comment.empty())
			{
				 stream << comment << m_call.arguments.comment << comment;
				 stream << std::endl << _linePrefix << newline << ws;
			}
			stream << arrow;
		}

		/// Format either the expected output or the actual result output
		std::string result;
		if (_renderMode != RenderMode::ActualValuesExpectedGas)
		{
			bool const isFailure = m_call.expectations.failure;
			result = isFailure ?
				formatFailure(_errorReporter, m_call, m_rawBytes, /* _renderResult */ false, highlight) :
				formatRawParameters(m_call.expectations.result);
			if (!result.empty())
				AnsiColorized(stream, highlight, {util::formatting::RED_BACKGROUND}) << ws << result;
		}
		else
		{
			if (m_calledNonExistingFunction)
				_errorReporter.warning("The function \"" + m_call.signature + "\" is not known to the compiler.");

			bytes output = m_rawBytes;
			bool const isFailure = m_failure;
			result = isFailure ?
				formatFailure(_errorReporter, m_call, output, _renderMode == RenderMode::ActualValuesExpectedGas, highlight) :
				matchesExpectation() ?
					formatRawParameters(m_call.expectations.result) :
					formatBytesParameters(
						_errorReporter,
						output,
						m_call.signature,
						m_call.expectations.result,
						highlight
					);

			if (!matchesExpectation())
			{
				std::optional<ParameterList> abiParams;

				if (isFailure)
				{
					if (!output.empty())
						abiParams = ContractABIUtils::failureParameters(output);
				}
				else
					abiParams = ContractABIUtils::parametersFromJsonOutputs(
						_errorReporter,
						m_contractABI,
						m_call.signature
					);

				std::string bytesOutput = abiParams ?
					BytesUtils::formatRawBytes(output, abiParams.value(), _linePrefix) :
					BytesUtils::formatRawBytes(
						output,
						ContractABIUtils::defaultParameters((output.size() + 31) / 32),
						_linePrefix
					);

				_errorReporter.warning(
					"The call to \"" + m_call.signature + "\" returned \n" +
					bytesOutput
				);
			}

			if (isFailure)
				AnsiColorized(stream, highlight, {util::formatting::RED_BACKGROUND}) << ws << result;
			else
				if (!result.empty())
					stream << ws << result;

		}

		/// Format comments on expectations taking the display-mode into account.
		if (_singleLine)
		{
			if (!m_call.expectations.comment.empty())
				stream << ws << comment << m_call.expectations.comment << comment;
		}
		else
		{
			if (!m_call.expectations.comment.empty())
			{
				stream << std::endl << _linePrefix << newline << ws;
				stream << comment << m_call.expectations.comment << comment;
			}
		}

		std::vector<std::string> sideEffects;
		if (_renderMode == RenderMode::ExpectedValuesExpectedGas || _renderMode == RenderMode::ExpectedValuesActualGas)
			sideEffects = m_call.expectedSideEffects;
		else
			sideEffects = m_call.actualSideEffects;

		if (!sideEffects.empty())
		{
			stream << std::endl;
			size_t i = 0;
			for (; i < sideEffects.size() - 1; ++i)
				stream << _linePrefix << "// ~ " << sideEffects[i] << std::endl;
			stream << _linePrefix << "// ~ " << sideEffects[i];
		}

		stream << formatGasExpectations(_linePrefix, _renderMode == RenderMode::ExpectedValuesActualGas, _interactivePrint);
	};

	formatOutput(m_call.displayMode == FunctionCall::DisplayMode::SingleLine);
	return stream.str();
}

std::string TestFunctionCall::formatBytesParameters(
	ErrorReporter& _errorReporter,
	bytes const& _bytes,
	std::string const& _signature,
	solidity::frontend::test::ParameterList const& _parameters,
	bool _highlight,
	bool _failure
) const
{
	using ParameterList = solidity::frontend::test::ParameterList;

	std::stringstream os;

	if (_bytes.empty())
		return {};

	if (_failure)
	{
		os << BytesUtils::formatBytesRange(
			_bytes,
			ContractABIUtils::failureParameters(_bytes),
			_highlight
		);

		return os.str();
	}
	else
	{
		std::optional<ParameterList> abiParams = ContractABIUtils::parametersFromJsonOutputs(
			_errorReporter,
			m_contractABI,
			_signature
		);

		if (abiParams)
		{
			std::optional<ParameterList> preferredParams = ContractABIUtils::preferredParameters(
				_errorReporter,
				_parameters,
				abiParams.value(),
				_bytes
			);

			if (preferredParams)
			{
				ContractABIUtils::overwriteParameters(_errorReporter, preferredParams.value(), abiParams.value());
				os << BytesUtils::formatBytesRange(_bytes, preferredParams.value(), _highlight);
			}
		}
		else
		{
			ParameterList defaultParameters = ContractABIUtils::defaultParameters((_bytes.size() + 31) / 32);

			ContractABIUtils::overwriteParameters(_errorReporter, defaultParameters, _parameters);
			os << BytesUtils::formatBytesRange(_bytes, defaultParameters, _highlight);
		}
		return os.str();
	}
}

std::string TestFunctionCall::formatFailure(
	ErrorReporter& _errorReporter,
	solidity::frontend::test::FunctionCall const& _call,
	bytes const& _output,
	bool _renderResult,
	bool _highlight
) const
{
	std::stringstream os;

	os << formatToken(Token::Failure);

	if (!_output.empty())
		os << ", ";

	if (_renderResult)
		os << formatBytesParameters(
			_errorReporter,
			_output,
			_call.signature,
			_call.expectations.result,
			_highlight,
			true
		);
	else
		os << formatRawParameters(_call.expectations.result);

	return os.str();
}

std::string TestFunctionCall::formatRawParameters(
	solidity::frontend::test::ParameterList const& _params,
	std::string const& _linePrefix
) const
{
	std::stringstream os;
	for (auto const& param: _params)
		if (!param.rawString.empty())
		{
			if (param.format.newline)
				os << std::endl << _linePrefix << "// ";
			for (auto const c: param.rawString)
				// NOTE: Even though we have a toHex() overload specifically for uint8_t, the compiler
				// chooses the one for bytes if the second argument is omitted.
				os << (c >= ' ' ? std::string(1, c) : "\\x" + util::toHex(static_cast<uint8_t>(c), HexCase::Lower));
			if (&param != &_params.back())
				os << ", ";
		}
	return os.str();
}

std::string TestFunctionCall::formatGasExpectations(
	std::string const& _linePrefix,
	bool _useActualCost,
	bool _showDifference
) const
{
	std::stringstream os;
	for (auto const& [runType, gasUsed]: (_useActualCost ? m_gasCosts : m_call.expectations.gasUsed))
		if (!runType.empty())
		{
			bool differentResults =
				m_gasCosts.count(runType) > 0 &&
				m_call.expectations.gasUsed.count(runType) > 0 &&
				m_gasCosts.at(runType) != m_call.expectations.gasUsed.at(runType);

			s256 difference = 0;
			if (differentResults)
				difference =
					static_cast<s256>(m_gasCosts.at(runType)) -
					static_cast<s256>(m_call.expectations.gasUsed.at(runType));
			int percent = 0;
			if (differentResults)
				percent = static_cast<int>(
					100.0 * (static_cast<double>(difference) / static_cast<double>(m_call.expectations.gasUsed.at(runType)))
				);
			os << std::endl << _linePrefix << "// gas " << runType << ": " << (gasUsed.str());
			if (_showDifference && differentResults && _useActualCost)
				os << " [" << std::showpos << difference << " (" << percent << "%)]";
		}
	return os.str();
}

void TestFunctionCall::reset()
{
	m_rawBytes = bytes{};
	m_failure = true;
	m_contractABI = Json::Value{};
	m_calledNonExistingFunction = false;
}

bool TestFunctionCall::matchesExpectation() const
{
	return m_failure == m_call.expectations.failure && m_rawBytes == m_call.expectations.rawBytes();
}
