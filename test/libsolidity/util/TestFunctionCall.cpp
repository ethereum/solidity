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
#include <libsolutil/JSON.h>

#include <boost/algorithm/string/predicate.hpp>
#include <boost/algorithm/string/replace.hpp>

#include <optional>
#include <stdexcept>
#include <string>

using namespace solidity;
using namespace solidity::util;
using namespace solidity::frontend::test;
using namespace std;

using Token = soltest::Token;

TestFunctionCall::EventInformation
TestFunctionCall::findEventSignature(solidity::test::ExecutionFramework::log_record const& log) const
{
	for (const auto& entity: m_contractABI)
	{
		EventInformation eventInfo;
		if (entity["type"].isString() && entity["type"].asString() == "event")
		{
			string name{entity["name"].asString()};
			bool anonymous{entity["anonymous"].asBool()};
			vector<string> argumentTypes;
			for (const auto& input: entity["inputs"])
			{
				string type{input["internalType"].asString()};
				argumentTypes.emplace_back(type);
				if (input["indexed"].asBool())
					eventInfo.indexedTypes.push_back(type);
				else
					eventInfo.nonIndexedTypes.push_back(type);
			}
			if (!name.empty() && !anonymous)
			{
				std::string signature{name};
				signature += "(";
				for (auto& arg: argumentTypes)
					signature += arg + ",";
				if (!argumentTypes.empty())
					signature = signature.substr(0, signature.length() - 1);
				signature += ")";
				if (!log.topics.empty() && log.topics[0] == util::keccak256(signature))
				{
					eventInfo.signature = signature;
					return eventInfo;
				}
			}
		}
	}
	return {};
}

string TestFunctionCall::generateExpectation(string _linePrefix, solidity::test::ExecutionFramework::log_record const& _log) const
{
	stringstream sstream;
	EventInformation eventInfo = findEventSignature(_log);
	string signature{eventInfo.signature};
	sstream << std::endl
			<< _linePrefix << "// logs.expectEvent(uint256,string): " << _log.index << ", "
			<< "\"" + signature + "\" -> ";

	vector<u256> topicsAndData;
	for (auto& topic: _log.topics)
		if ((topic != *_log.topics.begin() && !signature.empty()) || signature.empty())
			topicsAndData.emplace_back(u256{topic});

	assert(_log.data.size() % 32 == 0);

	if (eventInfo.signature.empty())
	{
		for (long current = 0; current < static_cast<long>(_log.data.size() / 32); ++current)
		{
			bytes parameter{_log.data.begin() + current * 32, _log.data.begin() + current * 32 + 32};
			topicsAndData.emplace_back(util::fromBigEndian<u256>(parameter));
		}
	}
	else
	{
		long current = 0;
		for (auto& type: eventInfo.nonIndexedTypes)
		{
			// todo: use type information to improve decoding.
			(void) type;
			bytes parameter{_log.data.begin() + current, _log.data.begin() + current + 32};
			current += 32;
			topicsAndData.emplace_back(util::fromBigEndian<u256>(parameter));
		}
	}

	for (auto& element: topicsAndData)
	{
		sstream << util::toCompactHexWithPrefix(element);
		if (&element != &*topicsAndData.rbegin())
			sstream << ", ";
	}

	return sstream.str();
}

string TestFunctionCall::format(
	ErrorReporter& _errorReporter,
	string const& _linePrefix,
	bool const _renderResult,
	bool const _highlight
) const
{
	stringstream stream;

	bool highlight = (!matchesExpectation() || hasUnconsumedLogs()) && _highlight;

	auto formatOutput = [&](bool const _singleLine)
	{
		string ws = " ";
		string newline = formatToken(Token::Newline);
		string colon = formatToken(Token::Colon);

		if (m_call.kind == FunctionCall::Kind::Library)
		{
			stream << _linePrefix << newline << ws << "library:" << ws << m_call.signature;
			return;
		}
		else if (m_call.kind == FunctionCall::Kind::Storage)
		{
			stream << _linePrefix << newline << ws << "storage" << colon << ws;
			soltestAssert(m_rawBytes.size() == 1, "");
			soltestAssert(m_call.expectations.rawBytes().size() == 1, "");
			bool isEmpty = _renderResult ? m_rawBytes.front() == 0 : m_call.expectations.rawBytes().front() == 0;
			string output = isEmpty ? "empty" : "nonempty";
			if (_renderResult && !matchesExpectation())
				AnsiColorized(stream, highlight, {util::formatting::RED_BACKGROUND}) << output;
			else
				stream << output;

			return;
		}
		else if (m_call.kind == FunctionCall::Kind::Builtin)
		{
			//
			return;
		}
		else if (hasUnconsumedLogs())
		{
			formatFunctionCall(_linePrefix, _renderResult, _singleLine, !matchesExpectation(), _errorReporter, stream);

			for (auto& log: consumedLogs())
				stream << generateExpectation(_linePrefix, log);

			std::stringstream sstream;
			for (auto& log: unconsumedLogs())
				sstream << generateExpectation(_linePrefix, log);

			if (highlight)
			{
				AnsiColorized(stream, highlight, {util::formatting::RED_BACKGROUND}) << sstream.str();
				AnsiColorized(stream, highlight, {util::formatting::RESET});
			} else
				stream << sstream.str();
			return;
		}
		else
			formatFunctionCall(_linePrefix, _renderResult, _singleLine, highlight, _errorReporter, stream);
	};

	formatOutput(m_call.displayMode == FunctionCall::DisplayMode::SingleLine);
	return stream.str();
}

void TestFunctionCall::formatFunctionCall(
	const string& _linePrefix,
	const bool _renderResult,
	const bool _singleLine,
	const bool _highlight,
	ErrorReporter& _errorReporter,
	stringstream& stream) const
{
	string ws = " ";
	string arrow = formatToken(Token::Arrow);
	string colon = formatToken(Token::Colon);
	string comma = formatToken(Token::Comma);
	string comment = formatToken(Token::Comment);
	string ether = formatToken(Token::Ether);
	string wei = formatToken(Token::Wei);
	string newline = formatToken(Token::Newline);
	string failure = formatToken(Token::Failure);

	/// Formats the function signature. This is the same independent from the display-mode.
	stream << _linePrefix << newline << ws << m_call.signature;
	if (m_call.value.value > u256(0))
	{
		if (m_call.value.unit == FunctionValueUnit::Ether)
			stream << comma << ws << (m_call.value.value / exp256(10, 18)) << ws << ether;
		else if (m_call.value.unit == FunctionValueUnit::Wei)
			stream << comma << ws << m_call.value.value << ws << wei;
		else
			soltestAssert(false, "");
	}
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
	auto& builtin = m_call.expectations.builtin;
	if (!_renderResult)
	{
		bool const isFailure = m_call.expectations.failure;
		result = isFailure ? formatFailure(_errorReporter, m_call, m_rawBytes, _renderResult, _highlight)
						   : formatRawParameters(m_call.expectations.result);
		if (!result.empty())
		{
			AnsiColorized(stream, _highlight, {util::formatting::RESET}) << ws;
			AnsiColorized(stream, _highlight, {util::formatting::RED_BACKGROUND}) << result;
		}
	}
	else
	{
		if (m_calledNonExistingFunction)
			_errorReporter.warning("The function \"" + m_call.signature + "\" is not known to the compiler.");

		if (m_consumedLogIndexes.size() != m_rawLogs.size())
		{
			stringstream str;
			str << "The function \"" << m_call.signature << "\" produced " << m_rawLogs.size() << " log(s), but only "
				<< m_consumedLogIndexes.size() << " log(s) where consumed.";
			_errorReporter.error(str.str());
		}

		if (builtin)
			_errorReporter.warning(
				"The expectation \"" + builtin->signature + ": " + formatRawParameters(builtin->arguments.parameters)
				+ "\" will be replaced with the actual value returned by the test.");

		bytes output = m_rawBytes;
		bool const isFailure = m_failure;
		result = isFailure ? formatFailure(_errorReporter, m_call, output, _renderResult, _highlight)
				 : matchesExpectation()
					 ? formatRawParameters(m_call.expectations.result)
					 : formatBytesParameters(
						 _errorReporter, output, m_call.signature, m_call.expectations.result, _highlight);

		if (!matchesExpectation())
		{
			std::optional<ParameterList> abiParams;

			if (isFailure)
			{
				if (!output.empty())
					abiParams = ContractABIUtils::failureParameters(output);
			}
			else
				abiParams
					= ContractABIUtils::parametersFromJsonOutputs(_errorReporter, m_contractABI, m_call.signature);

			string bytesOutput
				= abiParams ? BytesUtils::formatRawBytes(output, abiParams.value(), _linePrefix)
							: BytesUtils::formatRawBytes(
								output, ContractABIUtils::defaultParameters((output.size() + 31) / 32), _linePrefix);

			_errorReporter.warning("The call to \"" + m_call.signature + "\" returned \n" + bytesOutput);
		}

		if (isFailure)
			AnsiColorized(stream, _highlight, {util::formatting::RED_BACKGROUND}) << ws << result;
		else if (!result.empty())
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
	for (auto& log: consumedLogs())
		stream << generateExpectation(_linePrefix, log);
}

string TestFunctionCall::formatBytesParameters(
	ErrorReporter& _errorReporter,
	bytes const& _bytes,
	string const& _signature,
	solidity::frontend::test::ParameterList const& _parameters,
	bool _highlight,
	bool _failure) const
{
	using ParameterList = solidity::frontend::test::ParameterList;

	stringstream os;

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

string TestFunctionCall::formatFailure(
	ErrorReporter& _errorReporter,
	solidity::frontend::test::FunctionCall const& _call,
	bytes const& _output,
	bool _renderResult,
	bool _highlight
) const
{
	stringstream os;

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

string TestFunctionCall::formatRawParameters(
	solidity::frontend::test::ParameterList const& _params,
	std::string const& _linePrefix
) const
{
	stringstream os;
	for (auto const& param: _params)
		if (!param.rawString.empty())
		{
			if (param.format.newline)
				os << endl << _linePrefix << "// ";
			for (auto const c: param.rawString)
				os << (c >= ' ' ? string(1, c) : "\\x" + toHex(static_cast<uint8_t>(c)));
			if (&param != &_params.back())
				os << ", ";
		}
	return os.str();
}

void TestFunctionCall::reset()
{
	m_rawBytes = bytes{};
	m_failure = true;
	m_calledNonExistingFunction = false;
}

bool TestFunctionCall::matchesExpectation() const
{
	return m_failure == m_call.expectations.failure && m_rawBytes == m_call.expectations.rawBytes();
}
