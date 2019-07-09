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

#include <test/libsolidity/util/ContractABIUtils.h>

#include <liblangutil/Common.h>

#include <boost/algorithm/string.hpp>

#include <fstream>
#include <memory>
#include <regex>
#include <stdexcept>

using namespace dev;
using namespace langutil;
using namespace solidity;
using namespace dev::solidity::test;
using namespace std;
using namespace soltest;

dev::solidity::test::ParameterList ContractABIUtils::parametersFromJson(
	Json::Value const& _contractABI,
	string const& _functionName
) const
{
	ParameterList abiParams;
	for (auto const& function: _contractABI)
		if (function["name"] == _functionName)
			for (auto const& output: function["outputs"])
			{
				auto types = fromTypeName(output["type"].asString());
				for (auto const& type: types)
					abiParams.push_back(Parameter{bytes(), "", type, FormatInfo{}});
			}

	return abiParams;
}

std::vector<ABIType> ContractABIUtils::fromTypeName(string const& _type) const
{
	static regex s_boolType{"(bool)"};
	static regex s_uintType{"(uint\\d*)"};
	static regex s_intType{"(int\\d*)"};
	static regex s_bytesType{"(bytes\\d+)"};
	static regex s_dynBytesType{"(\\bbytes\\b)"};
	static regex s_stringType{"(string)"};

	vector<ABIType> abiTypes;
	if (regex_match(_type, s_boolType))
		abiTypes.push_back(ABIType{ABIType::Boolean, ABIType::AlignRight, 32});
	else if (regex_match(_type, s_uintType))
		abiTypes.push_back(ABIType{ABIType::UnsignedDec, ABIType::AlignRight, 32});
	else if (regex_match(_type, s_intType))
		abiTypes.push_back(ABIType{ABIType::SignedDec, ABIType::AlignRight, 32});
	else if (regex_match(_type, s_bytesType))
		abiTypes.push_back(ABIType{ABIType::Hex, ABIType::AlignRight, 32});
	else if (regex_match(_type, s_dynBytesType))
	{
		abiTypes.push_back(ABIType{ABIType::UnsignedDec, ABIType::AlignRight, 32});
		abiTypes.push_back(ABIType{ABIType::UnsignedDec, ABIType::AlignRight, 32});
		abiTypes.push_back(ABIType{ABIType::HexString, ABIType::AlignLeft, 32});
	}
	else if (regex_match(_type, s_stringType))
	{
		abiTypes.push_back(ABIType{ABIType::UnsignedDec, ABIType::AlignRight, 32});
		abiTypes.push_back(ABIType{ABIType::UnsignedDec, ABIType::AlignRight, 32});
		abiTypes.push_back(ABIType{ABIType::String, ABIType::AlignLeft, 32});
	}
	else
		abiTypes.push_back(ABIType{ABIType::None, ABIType::AlignRight, 0});
	return abiTypes;
}
