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

#pragma once

#include <libsolutil/FixedHash.h>

#include <json/json.h>

#include <functional>
#include <map>
#include <optional>
#include <random>
#include <sstream>
#include <string>

class ValueGenerator
{
public:
	enum class Type: size_t
	{
		Boolean = 0,
		Integer,
		UInteger,
		FixedBytes,
		Bytes,
		String,
		Address,
		Function,
		Tuple
	};
	struct ArrayInfo
	{
		bool staticSize;
		size_t numElements;
	};
	enum class FixedBytesWidth: size_t
	{
		Bytes1 = 1,
		Bytes2,
		Bytes3,
		Bytes4,
		Bytes5,
		Bytes6,
		Bytes7,
		Bytes8,
		Bytes9,
		Bytes10,
		Bytes11,
		Bytes12,
		Bytes13,
		Bytes14,
		Bytes15,
		Bytes16,
		Bytes17,
		Bytes18,
		Bytes19,
		Bytes20,
		Bytes21,
		Bytes22,
		Bytes23,
		Bytes24,
		Bytes25,
		Bytes26,
		Bytes27,
		Bytes28,
		Bytes29,
		Bytes30,
		Bytes31,
		Bytes32
	};
	enum class IntegerWidth: size_t
	{
		W8 = 8,
		W16 = 16,
		W24 = 24,
		W32 = 32,
		W40 = 40,
		W48 = 48,
		W56 = 56,
		W64 = 64,
		W72 = 72,
		W80 = 80,
		W88 = 88,
		W96 = 96,
		W104 = 104,
		W112 = 112,
		W120 = 120,
		W128 = 128,
		W136 = 136,
		W144 = 144,
		W152 = 152,
		W160 = 160,
		W168 = 168,
		W176 = 176,
		W184 = 184,
		W192 = 192,
		W200 = 200,
		W208 = 208,
		W216 = 216,
		W224 = 224,
		W232 = 232,
		W240 = 240,
		W248 = 248,
		W256 = 256
	};
	struct IntegerType
	{
		bool sign;
		IntegerWidth width;
	};
	struct TypeInfo
	{
		Type type;
		FixedBytesWidth fixedByteWidth;
		IntegerType intType;
		std::vector<ArrayInfo> arrayInfo;
		std::vector<TypeInfo> tupleInfo;
		std::string name;
		std::string value;
	};
	explicit ValueGenerator(
		Json::Value const& _type,
		unsigned _seed,
		std::vector<solidity::util::h160> _addresses
	):
		m_rand(_seed),
		m_type(_type),
		m_bernoulli(0.5),
		m_addresses(std::move(_addresses))
	{}
	void boolean()
	{
		m_stream << (m_bernoulli(m_rand) ? "true" : "false");
	}
	void string()
	{
		m_stream << "hello";
	}
	void bytes()
	{
		m_stream << "0x1234";
	}
	void function();
	void fixedbytes()
	{

	}
	void address();
	void integer()
	{

	}
	void typeHelper(Json::Value const& _type, TypeInfo& _typeInfo);
	TypeInfo type(Json::Value const& _type);
	void tuple(Json::Value const& _tuple, TypeInfo& _typeInfo);
	std::pair<std::string, std::string> type();
	void initialiseTuple(TypeInfo& _tuple);
	void initialiseType(TypeInfo& _t);
	void initialiseArray(
		ArrayInfo& _arrayInfo,
	    TypeInfo& _typeInfo
    );
	void initialiseArray(
		std::vector<ArrayInfo>& _arrayInfo,
		TypeInfo& _typeInfo
	);
	std::string addressLiteral(bool _hexPrefix = true);
private:
	std::ostringstream m_stream;
	std::minstd_rand m_rand;
	Json::Value const& m_type;
	std::bernoulli_distribution m_bernoulli;
	std::vector<solidity::util::h160> m_addresses;
};
