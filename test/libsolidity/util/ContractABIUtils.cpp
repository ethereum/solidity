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

#include <test/libsolidity/util/ContractABIUtils.h>

#include <test/libsolidity/util/SoltestErrors.h>

#include <libsolidity/ast/Types.h>
#include <libsolidity/ast/TypeProvider.h>
#include <libsolutil/FunctionSelector.h>
#include <libsolutil/CommonData.h>

#include <liblangutil/Common.h>

#include <boost/algorithm/string.hpp>
#include <boost/assign/list_of.hpp>
#include <range/v3/view/zip.hpp>

#include <fstream>
#include <memory>
#include <numeric>
#include <regex>
#include <stdexcept>

using namespace solidity;
using namespace solidity::util;
using namespace solidity::langutil;
using namespace solidity::frontend::test;
using namespace std;

namespace
{

using ParameterList = solidity::frontend::test::ParameterList;

size_t arraySize(string const& _arrayType)
{
	auto leftBrack = _arrayType.find("[");
	auto rightBrack = _arrayType.rfind("]");

	soltestAssert(
		leftBrack != string::npos &&
		rightBrack != string::npos &&
		rightBrack == _arrayType.size() - 1 &&
		leftBrack < rightBrack,
		""
	);

	string size = _arrayType.substr(leftBrack + 1, rightBrack - leftBrack - 1);

	return static_cast<size_t>(stoi(size));
}

bool isBool(string const& _type)
{
	return _type == "bool";
}

bool isUint(string const& _type)
{
	return regex_match(_type, regex{"uint\\d*"});
}

bool isInt(string const& _type)
{
	return regex_match(_type, regex{"int\\d*"});
}

bool isFixedBytes(string const& _type)
{
	return regex_match(_type, regex{"bytes\\d+"});
}

bool isBytes(string const& _type)
{
	return regex_match(_type, regex{"\\bbytes\\b"});
}

bool isString(string const& _type)
{
	return _type == "string";
}

bool isFixedBoolArray(string const& _type)
{
	return regex_match(_type, regex{"bool\\[\\d+\\]"});
}

bool isFixedUintArray(string const& _type)
{
	return regex_match(_type, regex{"uint\\d*\\[\\d+\\]"});
}

bool isFixedIntArray(string const& _type)
{
	return regex_match(_type, regex{"int\\d*\\[\\d+\\]"});
}

bool isFixedStringArray(string const& _type)
{
	return regex_match(_type, regex{"string\\[\\d+\\]"});
}

bool isTuple(string const& _type)
{
	return _type == "tuple";
}

bool isFixedTupleArray(string const& _type)
{
	return regex_match(_type, regex{"tuple\\[\\d+\\]"});
}

optional<ABIType> isFixedPoint(string const& type)
{
	optional<ABIType> fixedPointType;
	smatch matches;
	if (regex_match(type, matches, regex{"(u?)fixed(\\d+)x(\\d+)"}))
	{
		ABIType abiType(ABIType::SignedFixedPoint);
		if (matches[1].str() == "u")
			abiType.type = ABIType::UnsignedFixedPoint;
		abiType.fractionalDigits = static_cast<unsigned>(std::stoi(matches[3].str()));
		fixedPointType = abiType;
	}
	return fixedPointType;
}

string functionSignatureFromABI(Json::Value const& _functionABI)
{
	auto inputs = _functionABI["inputs"];
	string signature = {_functionABI["name"].asString() + "("};
	size_t parameterCount = 0;

	for (auto const& input: inputs)
	{
		parameterCount++;
		signature += input["type"].asString();
		if (parameterCount < inputs.size())
			signature += ",";
	}

	return signature + ")";
}

}

std::optional<solidity::frontend::test::ParameterList> ContractABIUtils::parametersFromJsonOutputs(
	ErrorReporter& _errorReporter,
	Json::Value const& _contractABI,
	string const& _functionSignature
)
{
	if (!_contractABI)
		return std::nullopt;

	for (auto const& function: _contractABI)
		if (_functionSignature == functionSignatureFromABI(function))
		{
			ParameterList inplaceTypeParams;
			ParameterList dynamicTypeParams;
			ParameterList finalParams;

			for (auto const& output: function["outputs"])
			{
				string type = output["type"].asString();

				ABITypes inplaceTypes;
				ABITypes dynamicTypes;

				if (appendTypesFromName(output, inplaceTypes, dynamicTypes))
				{
					for (auto const& type: inplaceTypes)
						inplaceTypeParams.push_back(Parameter{bytes(), "", type, FormatInfo{}});
					for (auto const& type: dynamicTypes)
						dynamicTypeParams.push_back(Parameter{bytes(), "", type, FormatInfo{}});
				}
				else
				{
					_errorReporter.warning(
						"Could not convert \"" + type +
						"\" to internal ABI type representation. Falling back to default encoding."
					);
					return std::nullopt;
				}

				finalParams += inplaceTypeParams;

				inplaceTypeParams.clear();
			}
			return std::optional<ParameterList>(finalParams + dynamicTypeParams);
		}

	return std::nullopt;
}

bool ContractABIUtils::appendTypesFromName(
	Json::Value const& _functionOutput,
	ABITypes& _inplaceTypes,
	ABITypes& _dynamicTypes,
	bool _isCompoundType
)
{
	string type = _functionOutput["type"].asString();
	if (isBool(type))
		_inplaceTypes.push_back(ABIType{ABIType::Boolean});
	else if (isUint(type))
		_inplaceTypes.push_back(ABIType{ABIType::UnsignedDec});
	else if (isInt(type))
		_inplaceTypes.push_back(ABIType{ABIType::SignedDec});
	else if (isFixedBytes(type))
		_inplaceTypes.push_back(ABIType{ABIType::Hex});
	else if (isString(type))
	{
		_inplaceTypes.push_back(ABIType{ABIType::Hex});

		if (_isCompoundType)
			_dynamicTypes.push_back(ABIType{ABIType::Hex});

		_dynamicTypes.push_back(ABIType{ABIType::UnsignedDec});
		_dynamicTypes.push_back(ABIType{ABIType::String, ABIType::AlignLeft});
	}
	else if (isTuple(type))
	{
		ABITypes inplaceTypes;
		ABITypes dynamicTypes;

		for (auto const& component: _functionOutput["components"])
			appendTypesFromName(component, inplaceTypes, dynamicTypes, true);
		_dynamicTypes += inplaceTypes + dynamicTypes;
	}
	else if (isFixedBoolArray(type))
		_inplaceTypes += vector<ABIType>(arraySize(type), ABIType{ABIType::Boolean});
	else if (isFixedUintArray(type))
		_inplaceTypes += vector<ABIType>(arraySize(type), ABIType{ABIType::UnsignedDec});
	else if (isFixedIntArray(type))
		_inplaceTypes += vector<ABIType>(arraySize(type), ABIType{ABIType::SignedDec});
	else if (isFixedStringArray(type))
	{
		_inplaceTypes.push_back(ABIType{ABIType::Hex});

		_dynamicTypes += vector<ABIType>(arraySize(type), ABIType{ABIType::Hex});

		for (size_t i = 0; i < arraySize(type); i++)
		{
			_dynamicTypes.push_back(ABIType{ABIType::UnsignedDec});
			_dynamicTypes.push_back(ABIType{ABIType::String, ABIType::AlignLeft});
		}
	}
	else if (optional<ABIType> fixedPointType = isFixedPoint(type))
		_inplaceTypes.push_back(*fixedPointType);
	else if (isBytes(type))
		return false;
	else if (isFixedTupleArray(type))
		return false;
	else
		return false;

	return true;
}

void ContractABIUtils::overwriteParameters(
	ErrorReporter& _errorReporter,
	solidity::frontend::test::ParameterList& _targetParameters,
	solidity::frontend::test::ParameterList const& _sourceParameters
)
{
	using namespace placeholders;
	for (auto&& [source, target]: ranges::views::zip(_sourceParameters, _targetParameters))
		if (
				source.abiType.size != target.abiType.size ||
				source.abiType.type != target.abiType.type ||
				source.abiType.fractionalDigits != target.abiType.fractionalDigits
			)
			{
				_errorReporter.warning("Type or size of parameter(s) does not match.");
				target = source;
			}
}

solidity::frontend::test::ParameterList ContractABIUtils::preferredParameters(
	ErrorReporter& _errorReporter,
	solidity::frontend::test::ParameterList const& _targetParameters,
	solidity::frontend::test::ParameterList const& _sourceParameters,
	bytes const& _bytes
)
{
	if (_targetParameters.size() != _sourceParameters.size())
	{
		_errorReporter.warning(
			"Encoding does not match byte range. The call returned " +
			to_string(_bytes.size()) + " bytes, but " +
			to_string(encodingSize(_targetParameters)) + " bytes were expected."
		);
		return _sourceParameters;
	}
	else
		return _targetParameters;
}

solidity::frontend::test::ParameterList ContractABIUtils::defaultParameters(size_t count)
{
	ParameterList parameters;

	fill_n(
		back_inserter(parameters),
		count,
		Parameter{bytes(), "", ABIType{ABIType::UnsignedDec}, FormatInfo{}}
	);

	return parameters;
}

solidity::frontend::test::ParameterList ContractABIUtils::failureParameters(bytes const& _bytes)
{
	if (_bytes.empty())
		return {};
	else if (_bytes.size() < 4)
		return {Parameter{bytes(), "", ABIType{ABIType::HexString, ABIType::AlignNone, _bytes.size()}, FormatInfo{}}};
	else
	{
		ParameterList parameters;
		parameters.push_back(Parameter{bytes(), "", ABIType{ABIType::HexString, ABIType::AlignNone, 4}, FormatInfo{}});

		uint64_t selector = fromBigEndian<uint64_t>(bytes{_bytes.begin(), _bytes.begin() + 4});
		if (selector == selectorFromSignature32("Panic(uint256)"))
			parameters.push_back(Parameter{bytes(), "", ABIType{ABIType::Hex}, FormatInfo{}});
		else if (selector == selectorFromSignature32("Error(string)"))
		{
			parameters.push_back(Parameter{bytes(), "", ABIType{ABIType::Hex}, FormatInfo{}});
			parameters.push_back(Parameter{bytes(), "", ABIType{ABIType::UnsignedDec}, FormatInfo{}});
			/// If _bytes contains at least a 1 byte message (function selector + tail pointer + message length + message)
			/// append an additional string parameter to represent that message.
			if (_bytes.size() > 68)
				parameters.push_back(Parameter{bytes(), "", ABIType{ABIType::String}, FormatInfo{}});
		}
		else
			for (size_t i = 4; i < _bytes.size(); i += 32)
				parameters.push_back(Parameter{bytes(), "", ABIType{ABIType::HexString, ABIType::AlignNone, 32}, FormatInfo{}});
		return parameters;
	}
}

size_t ContractABIUtils::encodingSize(
	solidity::frontend::test::ParameterList const& _parameters
)
{
	auto sizeFold = [](size_t const _a, Parameter const& _b) { return _a + _b.abiType.size; };

	return accumulate(_parameters.begin(), _parameters.end(), size_t{0}, sizeFold);
}
