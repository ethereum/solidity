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

#include <libdevcore/AnsiColorized.h>

#include <boost/algorithm/string/replace.hpp>

#include <stdexcept>
#include <string>

using namespace dev;
using namespace solidity;
using namespace dev::solidity::test;
using namespace std;

string TestFunctionCall::format(
	ErrorReporter& _errorReporter,
	string const& _linePrefix,
	bool const _renderResult,
	bool const _highlight
) const
{
	using namespace soltest;
	using Token = soltest::Token;

	stringstream stream;

	bool highlight = !matchesExpectation() && _highlight;

	auto formatOutput = [&](bool const _singleLine)
	{
		string ws = " ";
		string arrow = formatToken(Token::Arrow);
		string colon = formatToken(Token::Colon);
		string comma = formatToken(Token::Comma);
		string comment = formatToken(Token::Comment);
		string ether = formatToken(Token::Ether);
		string newline = formatToken(Token::Newline);
		string failure = formatToken(Token::Failure);

		/// Formats the function signature. This is the same independent from the display-mode.
		stream << _linePrefix << newline << ws << m_call.signature;
		if (m_call.value > u256(0))
			stream << comma << ws << m_call.value << ws << ether;
		if (!m_call.arguments.rawBytes().empty())
		{
			string output = formatRawParameters(m_call.arguments.parameters, _linePrefix);
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
				if (_renderResult && (m_failure || !matchesExpectation()))
					stream << ws << arrow;
			}
			else
				stream << ws << arrow;
		}
		else
		{
			stream << endl << _linePrefix << newline << ws;
			if (!m_call.arguments.comment.empty())
			{
				 stream << comment << m_call.arguments.comment << comment;
				 stream << endl << _linePrefix << newline << ws;
			}
			stream << arrow;
		}

		/// Format either the expected output or the actual result output
		string result;
		if (!_renderResult)
		{
			bytes output = m_call.expectations.rawBytes();
			bool const isFailure = m_call.expectations.failure;
			result = isFailure ?
				failure :
				formatRawParameters(m_call.expectations.result);
			if (!result.empty())
				AnsiColorized(stream, highlight, {dev::formatting::RED_BACKGROUND}) << ws << result;
		}
		else
		{
			bytes output = m_rawBytes;
			bool const isFailure = m_failure;
			result = isFailure ?
				failure :
				matchesExpectation() ?
					formatRawParameters(m_call.expectations.result) :
					formatBytesParameters(
						_errorReporter,
						output,
						m_call.signature,
						m_call.expectations.result,
						highlight
					);

			if (isFailure)
				AnsiColorized(stream, highlight, {dev::formatting::RED_BACKGROUND}) << ws << result;
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
				stream << endl << _linePrefix << newline << ws;
				stream << comment << m_call.expectations.comment << comment;
			}
		}
	};

	formatOutput(m_call.displayMode == FunctionCall::DisplayMode::SingleLine);
	return stream.str();
}

string TestFunctionCall::formatBytesParameters(
	ErrorReporter& _errorReporter,
	bytes const& _bytes,
	string const& _signature,
	dev::solidity::test::ParameterList const& _params,
	bool _highlight
) const
{
	using ParameterList = dev::solidity::test::ParameterList;

	stringstream os;
	string functionName{_signature.substr(0, _signature.find("("))};

	/// Create parameters from Contract ABI. Used to generate values for
	/// auto-correction during interactive update routine.
	ParameterList abiParams = ContractABIUtils().parametersFromJson(m_contractABI, functionName);

	/// If parameter count does not match, take types defined by ABI, but only
	/// if the contract ABI is defined (needed for format tests where the actual
	/// result does not matter).
	ParameterList preferredParams;
	if (m_contractABI && (_params.size() != abiParams.size()))
	{
		auto sizeFold = [](size_t const _a, Parameter const& _b) { return _a + _b.abiType.size; };
		size_t encodingSize = std::accumulate(_params.begin(), _params.end(), size_t{0}, sizeFold);

		_errorReporter.warning(
			"Encoding does not match byte range. The call returned " +
			to_string(_bytes.size()) + " bytes, but " +
			to_string(encodingSize) + " bytes were expected."
		);
		preferredParams = abiParams;
	}
	else
		preferredParams = _params;

	/// If output is empty, do not format anything.
	if (_bytes.empty())
		return {};

	/// Format output bytes with the given parameters. ABI type takes precedence if:
	/// - size of ABI type is greater
	/// - given expected type does not match and needs to be overridden in order
	///   to generate a valid output of the parameter
	auto it = _bytes.begin();
	auto abiParam = abiParams.begin();
	size_t paramIndex = 1;
	for (auto const& param: preferredParams)
	{
		size_t size = param.abiType.size;
		if (m_contractABI)
			size = std::max((*abiParam).abiType.size, param.abiType.size);

		long offset = static_cast<long>(size);
		auto offsetIter = it + offset;
		bytes byteRange{it, offsetIter};

		/// Override type with ABI type if given one does not match.
		auto type = param.abiType;
		if (m_contractABI)
			if ((*abiParam).abiType.type > param.abiType.type)
			{
				type = (*abiParam).abiType;
				_errorReporter.warning(
					"Type of parameter " + to_string(paramIndex) +
					" does not match the one inferred from ABI."
				);
			}

		/// Prints obtained result if it does not match the expectation
		/// and prints the expected result otherwise.
		/// Highlights parameter only if it does not match.
		if (byteRange != param.rawBytes)
			AnsiColorized(
				os,
				_highlight,
				{dev::formatting::RED_BACKGROUND}
			) << formatBytesRange(byteRange, type);
		else
			os << param.rawString;

		if (abiParam != abiParams.end())
			abiParam++;

		it += offset;
		paramIndex++;
		if (&param != &preferredParams.back())
			os << ", ";
	}
	return os.str();
}

string TestFunctionCall::formatBytesRange(
	bytes const& _bytes,
	ABIType const& _abiType
) const
{
	stringstream os;

	switch (_abiType.type)
	{
	case ABIType::UnsignedDec:
		// Check if the detected type was wrong and if this could
		// be signed. If an unsigned was detected in the expectations,
		// but the actual result returned a signed, it would be formatted
		// incorrectly.
		os << BytesUtils().formatUnsigned(_bytes);
		break;
	case ABIType::SignedDec:
		os << BytesUtils().formatSigned(_bytes);
		break;
	case ABIType::Boolean:
		os << BytesUtils().formatBoolean(_bytes);
		break;
	case ABIType::Hex:
		os << BytesUtils().formatHex(_bytes);
		break;
	case ABIType::HexString:
		os << BytesUtils().formatHexString(_bytes);
		break;
	case ABIType::String:
		os << BytesUtils().formatString(_bytes, _abiType.size);
		break;
	case ABIType::Failure:
		break;
	case ABIType::None:
		break;
	}
	return os.str();
}

string TestFunctionCall::formatRawParameters(
	dev::solidity::test::ParameterList const& _params,
	std::string const& _linePrefix
) const
{
	stringstream os;
	for (auto const& param: _params)
	{
		if (param.format.newline)
			os << endl << _linePrefix << "// ";
		os << param.rawString;
		if (&param != &_params.back())
		{
			if (param.format.newline)
				os << ",";
			else
				os << ", ";
		}
	}
	return os.str();
}

void TestFunctionCall::reset()
{
	m_rawBytes = bytes{};
	m_failure = true;
}

bool TestFunctionCall::matchesExpectation() const
{
	return m_failure == m_call.expectations.failure && m_rawBytes == m_call.expectations.rawBytes();
}
