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
#include <libdevcore/AnsiColorized.h>
#include <stdexcept>
#include <string>

using namespace dev;
using namespace solidity;
using namespace dev::solidity::test;
using namespace std;

string TestFunctionCall::format(string const& _linePrefix, bool const _renderResult, bool const _highlight) const
{
	using namespace soltest;
	using Token = soltest::Token;

	stringstream _stream;
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
		_stream << _linePrefix << newline << ws << m_call.signature;
		if (m_call.value > u256(0))
			_stream << comma << ws << m_call.value << ws << ether;
		if (!m_call.arguments.rawBytes().empty())
		{
			string output = formatRawParameters(m_call.arguments.parameters, _linePrefix);
			_stream << colon;
			if (_singleLine)
				_stream << ws;
			_stream << output;

		}

		/// Formats comments on the function parameters and the arrow taking
		/// the display-mode into account.
		if (_singleLine)
		{
			if (!m_call.arguments.comment.empty())
				_stream << ws << comment << m_call.arguments.comment << comment;
			_stream << ws << arrow << ws;
		}
		else
		{
			_stream << endl << _linePrefix << newline << ws;
			if (!m_call.arguments.comment.empty())
			{
				 _stream << comment << m_call.arguments.comment << comment;
				 _stream << endl << _linePrefix << newline << ws;
			}
			_stream << arrow << ws;
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
		}
		else
		{
			bytes output = m_rawBytes;
			bool const isFailure = m_failure;
			result = isFailure ?
				failure :
				matchesExpectation() ?
					formatRawParameters(m_call.expectations.result) :
					formatBytesParameters(output, m_call.expectations.result);
		}
		AnsiColorized(_stream, highlight, {dev::formatting::RED_BACKGROUND}) << result;

		/// Format comments on expectations taking the display-mode into account.
		if (_singleLine)
		{
			if (!m_call.expectations.comment.empty())
				_stream << ws << comment << m_call.expectations.comment << comment;
		}
		else
		{
			if (!m_call.expectations.comment.empty())
			{
				_stream << endl << _linePrefix << newline << ws;
				_stream << comment << m_call.expectations.comment << comment;
			}
		}
	};

	if (m_call.displayMode == FunctionCall::DisplayMode::SingleLine)
		formatOutput(true);
	else
		formatOutput(false);
	return _stream.str();
}

string TestFunctionCall::formatBytesParameters(bytes const& _bytes, dev::solidity::test::ParameterList const& _params) const
{
	stringstream resultStream;
	if (_bytes.empty())
		return {};
	auto it = _bytes.begin();
	for (auto const& param: _params)
	{
		long offset = static_cast<long>(param.abiType.size);
		auto offsetIter = it + offset;
		soltestAssert(offsetIter <= _bytes.end(), "Byte range can not be extended past the end of given bytes.");

		bytes byteRange{it, offsetIter};
		switch (param.abiType.type)
		{
		case ABIType::UnsignedDec:
			// Check if the detected type was wrong and if this could
			// be signed. If an unsigned was detected in the expectations,
			// but the actual result returned a signed, it would be formatted
			// incorrectly.
			if (*byteRange.begin() & 0x80)
				resultStream << u2s(fromBigEndian<u256>(byteRange));
			else
				resultStream << fromBigEndian<u256>(byteRange);
			break;
		case ABIType::SignedDec:
			if (*byteRange.begin() & 0x80)
				resultStream << u2s(fromBigEndian<u256>(byteRange));
			else
				resultStream << fromBigEndian<u256>(byteRange);
			break;
		case ABIType::Boolean:
		{
			u256 result = fromBigEndian<u256>(byteRange);
			if (result == 0)
				resultStream << "false";
			else if (result == 1)
				resultStream << "true";
			else
				resultStream << result;
			break;
		}
		case ABIType::Hex:
			resultStream << toHex(byteRange, HexPrefix::Add);
			break;
		case ABIType::HexString:
			resultStream << "hex\"" << toHex(byteRange) << "\"";
			break;
		case ABIType::Failure:
			break;
		case ABIType::None:
			break;
		}
		it += offset;
		if (it != _bytes.end() && !(param.abiType.type == ABIType::None))
			resultStream << ", ";
	}
	soltestAssert(it == _bytes.end(), "Parameter encoding too short for the given byte range.");
	return resultStream.str();
}

string TestFunctionCall::formatRawParameters(dev::solidity::test::ParameterList const& _params, std::string const& _linePrefix) const
{
	stringstream resultStream;
	for (auto const& param: _params)
	{
		if (param.format.newline)
			resultStream << endl << _linePrefix << "// ";
		resultStream << param.rawString;
		if (&param != &_params.back())
			resultStream << ", ";
	}
	return resultStream.str();
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
