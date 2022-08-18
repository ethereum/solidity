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
/**
 * @date 2018
 * Unit tests for JSON.h.
 */

#include <libsolutil/JSON.h>

#include <test/Common.h>

#include <boost/test/unit_test.hpp>

using namespace std;

namespace solidity::util::test
{

BOOST_AUTO_TEST_SUITE(JsonTest, *boost::unit_test::label("nooptions"))

BOOST_AUTO_TEST_CASE(json_types)
{
	auto check = [](Json::Value value, string const& expectation) {
		BOOST_CHECK(jsonCompactPrint(value) == expectation);
	};

	Json::Value value;
	BOOST_CHECK(value.empty());
	value = {};
	BOOST_CHECK(value.empty());
	value = Json::Value();
	BOOST_CHECK(value.empty());
	value = Json::nullValue;
	BOOST_CHECK(value.empty());

	check(value, "null");
	check({}, "null");
	check(Json::Value(), "null");
	check(Json::nullValue, "null");
	check(Json::objectValue, "{}");
	check(Json::arrayValue, "[]");
	check(Json::UInt(1), "1");
	check(Json::UInt(-1), "4294967295");
	check(Json::UInt64(1), "1");
	check(Json::UInt64(-1), "18446744073709551615");
	check(Json::LargestUInt(1), "1");
	check(Json::LargestUInt(-1), "18446744073709551615");
	check(Json::LargestUInt(0xffffffff), "4294967295");
	check(Json::Value("test"), "\"test\"");
	check("test", "\"test\"");
	check(true, "true");

	value = Json::objectValue;
	value["key"] = "value";
	check(value, "{\"key\":\"value\"}");

	value = Json::arrayValue;
	value.append(1);
	value.append(2);
	check(value, "[1,2]");
}

BOOST_AUTO_TEST_CASE(json_pretty_print)
{
	Json::Value json;
	Json::Value jsonChild;

	jsonChild["3.1"] = "3.1";
	jsonChild["3.2"] = 2;
	json["1"] = 1;
	json["2"] = "2";
	json["3"] = jsonChild;
	json["4"] = "ऑ ऒ ओ औ क ख";
	json["5"] = "\xff";

	BOOST_CHECK(
	"{\n"
	"  \"1\": 1,\n"
	"  \"2\": \"2\",\n"
	"  \"3\":\n"
	"  {\n"
	"    \"3.1\": \"3.1\",\n"
	"    \"3.2\": 2\n"
	"  },\n"
	"  \"4\": \"\\u0911 \\u0912 \\u0913 \\u0914 \\u0915 \\u0916\",\n"
	"  \"5\": \"\\ufffd\"\n"
	"}" == jsonPrettyPrint(json));
}

BOOST_AUTO_TEST_CASE(json_compact_print)
{
	Json::Value json;
	Json::Value jsonChild;

	jsonChild["3.1"] = "3.1";
	jsonChild["3.2"] = 2;
	json["1"] = 1;
	json["2"] = "2";
	json["3"] = jsonChild;
	json["4"] = "ऑ ऒ ओ औ क ख";
	json["5"] = "\xff";

	BOOST_CHECK("{\"1\":1,\"2\":\"2\",\"3\":{\"3.1\":\"3.1\",\"3.2\":2},\"4\":\"\\u0911 \\u0912 \\u0913 \\u0914 \\u0915 \\u0916\",\"5\":\"\\ufffd\"}" == jsonCompactPrint(json));
}

BOOST_AUTO_TEST_CASE(parse_json_strict)
{
	// In this test we check conformance against JSON.parse (https://tc39.es/ecma262/multipage/structured-data.html#sec-json.parse)
	// and ECMA-404 (https://www.ecma-international.org/publications-and-standards/standards/ecma-404/)

	Json::Value json;
	std::string errors;

	// Just parse a valid json input
	BOOST_CHECK(jsonParseStrict("{\"1\":1,\"2\":\"2\",\"3\":{\"3.1\":\"3.1\",\"3.2\":2}}", json, &errors));
	BOOST_CHECK(json["1"] == 1);
	BOOST_CHECK(json["2"] == "2");
	BOOST_CHECK(json["3"]["3.1"] == "3.1");
	BOOST_CHECK(json["3"]["3.2"] == 2);

	// Trailing garbage is not allowed in ECMA-262
	BOOST_CHECK(!jsonParseStrict("{\"1\":2,\"2\":\"2\",\"3\":{\"3.1\":\"3.1\",\"3.2\":3}}}}}}}}}}", json, &errors));

	// Comments are not allowed in ECMA-262
	// ... but JSONCPP allows them
	BOOST_CHECK(jsonParseStrict(
		"{\"1\":3, // awesome comment\n\"2\":\"2\",\"3\":{\"3.1\":\"3.1\",\"3.2\":5}}", json, &errors
	));
	BOOST_CHECK(json["1"] == 3);
	BOOST_CHECK(json["2"] == "2");
	BOOST_CHECK(json["3"]["3.1"] == "3.1");
	BOOST_CHECK(json["3"]["3.2"] == 5);

	// According to ECMA-404 object, array, number, string, true, false, null are allowed
	// ... but JSONCPP disallows value types
	BOOST_CHECK(jsonParseStrict("[]", json, &errors));
	BOOST_CHECK(json.isArray());
	BOOST_CHECK(jsonParseStrict("{}", json, &errors));
	BOOST_CHECK(json.isObject());
	BOOST_CHECK(!jsonParseStrict("1", json, &errors));
	// BOOST_CHECK(json.isNumeric());
	BOOST_CHECK(!jsonParseStrict("\"hello\"", json, &errors));
	// BOOST_CHECK(json.isString());
	BOOST_CHECK(!jsonParseStrict("true", json, &errors));
	// BOOST_CHECK(json.isBool());
	BOOST_CHECK(!jsonParseStrict("null", json, &errors));
	// BOOST_CHECK(json.isNull());

	// Single quotes are also disallowed by ECMA-404
	BOOST_CHECK(!jsonParseStrict("'hello'", json, &errors));
	// BOOST_CHECK(json.isString());

	// Only string keys in objects are allowed in ECMA-404
	BOOST_CHECK(!jsonParseStrict("{ 42: \"hello\" }", json, &errors));

	// According to ECMA-404 hex escape sequences are not allowed, only unicode (\uNNNN) and
	// a few control characters (\b, \f, \n, \r, \t)
	//
	// More lenient parsers allow hex escapes as long as they translate to a valid UTF-8 encoding.
	//
	// ... but JSONCPP allows any hex escapes
	BOOST_CHECK(jsonParseStrict("[ \"\x80\xec\x80\" ]", json, &errors));
	BOOST_CHECK(json.isArray());
	BOOST_CHECK(json[0] == "\x80\xec\x80");

	// This would be valid more lenient parsers.
	BOOST_CHECK(jsonParseStrict("[ \"\xF0\x9F\x98\x8A\" ]", json, &errors));
	BOOST_CHECK(json.isArray());
	BOOST_CHECK(json[0] == "😊");
}

BOOST_AUTO_TEST_CASE(json_ofType)
{
	Json::Value json;

	json["float"] = 3.1f;
	json["double"] = 3.1;
	json["int"] = 2;
	json["int64"] = Json::Int64{0x4000000000000000};
	json["string"] = "Hello World!";

	BOOST_CHECK(ofType<float>(json, "float"));
	BOOST_CHECK(ofType<double>(json, "double"));
	BOOST_CHECK(ofType<int>(json, "int"));
	BOOST_CHECK(ofType<Json::Int>(json, "int"));
	BOOST_CHECK(ofType<Json::UInt>(json, "int"));
	BOOST_CHECK(ofType<Json::Int64>(json, "int"));
	BOOST_CHECK(ofType<Json::Int64>(json, "int64"));
	BOOST_CHECK(ofType<Json::UInt64>(json, "int64"));
	BOOST_CHECK(ofType<std::string>(json, "string"));
	BOOST_CHECK(!ofType<Json::Int>(json, "int64"));
	BOOST_CHECK(!ofType<int>(json, "double"));
	BOOST_CHECK(!ofType<float>(json, "string"));
	BOOST_CHECK(!ofType<double>(json, "string"));
	BOOST_CHECK(!ofType<Json::Int>(json, "string"));
	BOOST_CHECK(!ofType<Json::Int64>(json, "string"));
	BOOST_CHECK(!ofType<Json::UInt>(json, "string"));
	BOOST_CHECK(!ofType<Json::UInt64>(json, "string"));
}

BOOST_AUTO_TEST_CASE(json_ofTypeIfExists)
{
	Json::Value json;

	json["float"] = 3.1f;
	json["double"] = 3.1;
	json["int"] = 2;
	json["int64"] = Json::Int64{0x4000000000000000};
	json["string"] = "Hello World!";

	BOOST_CHECK(ofTypeIfExists<float>(json, "float"));
	BOOST_CHECK(ofTypeIfExists<double>(json, "double"));
	BOOST_CHECK(ofTypeIfExists<int>(json, "int"));
	BOOST_CHECK(ofTypeIfExists<Json::Int>(json, "int"));
	BOOST_CHECK(ofTypeIfExists<Json::UInt>(json, "int"));
	BOOST_CHECK(ofTypeIfExists<Json::Int64>(json, "int"));
	BOOST_CHECK(ofTypeIfExists<Json::Int64>(json, "int64"));
	BOOST_CHECK(ofTypeIfExists<Json::UInt64>(json, "int64"));
	BOOST_CHECK(ofTypeIfExists<std::string>(json, "string"));
	BOOST_CHECK(!ofTypeIfExists<Json::Int>(json, "int64"));
	BOOST_CHECK(!ofTypeIfExists<int>(json, "double"));
	BOOST_CHECK(!ofTypeIfExists<float>(json, "string"));
	BOOST_CHECK(!ofTypeIfExists<double>(json, "string"));
	BOOST_CHECK(!ofTypeIfExists<Json::Int>(json, "string"));
	BOOST_CHECK(!ofTypeIfExists<Json::Int64>(json, "string"));
	BOOST_CHECK(!ofTypeIfExists<Json::UInt>(json, "string"));
	BOOST_CHECK(!ofTypeIfExists<Json::UInt64>(json, "string"));
	BOOST_CHECK(ofTypeIfExists<Json::UInt64>(json, "NOT_EXISTING"));
}

BOOST_AUTO_TEST_CASE(json_getOrDefault)
{
	Json::Value json;

	json["float"] = 3.1f;
	json["double"] = 3.1;
	json["int"] = 2;
	json["int64"] = Json::Int64{0x4000000000000000};
	json["uint64"] = Json::UInt64{0x5000000000000000};
	json["string"] = "Hello World!";

	BOOST_CHECK(getOrDefault<float>(json, "float") == 3.1f);
	BOOST_CHECK(getOrDefault<float>(json, "float", -1.1f) == 3.1f);
	BOOST_CHECK(getOrDefault<float>(json, "no_float", -1.1f) == -1.1f);
	BOOST_CHECK(getOrDefault<double>(json, "double") == 3.1);
	BOOST_CHECK(getOrDefault<double>(json, "double", -1) == 3.1);
	BOOST_CHECK(getOrDefault<double>(json, "no_double", -1.1) == -1.1);
	BOOST_CHECK(getOrDefault<int>(json, "int") == 2);
	BOOST_CHECK(getOrDefault<int>(json, "int", -1) == 2);
	BOOST_CHECK(getOrDefault<int>(json, "no_int", -1) == -1);
	BOOST_CHECK(getOrDefault<Json::Int>(json, "int") == 2);
	BOOST_CHECK(getOrDefault<Json::Int>(json, "int", -1) == 2);
	BOOST_CHECK(getOrDefault<Json::Int>(json, "no_int", -1) == -1);
	BOOST_CHECK(getOrDefault<Json::UInt>(json, "int") == 2);
	BOOST_CHECK(getOrDefault<Json::UInt>(json, "int", 1) == 2);
	BOOST_CHECK(getOrDefault<Json::UInt>(json, "no_int", 1) == 1);
	BOOST_CHECK(getOrDefault<Json::Int64>(json, "int") == 2);
	BOOST_CHECK(getOrDefault<Json::Int64>(json, "int", -1) == 2);
	BOOST_CHECK(getOrDefault<Json::Int64>(json, "no_int", -1) == -1);
	BOOST_CHECK(getOrDefault<Json::Int64>(json, "int64") == 0x4000000000000000);
	BOOST_CHECK(getOrDefault<Json::Int64>(json, "int64", -1) == 0x4000000000000000);
	BOOST_CHECK(getOrDefault<Json::Int64>(json, "no_int64", -1) == -1);
	BOOST_CHECK(getOrDefault<Json::UInt64>(json, "int64") == 0x4000000000000000);
	BOOST_CHECK(getOrDefault<Json::UInt64>(json, "int64", 1) == 0x4000000000000000);
	BOOST_CHECK(getOrDefault<Json::UInt64>(json, "no_int64", 1) == 1);
	BOOST_CHECK(getOrDefault<Json::UInt64>(json, "uint64") == 0x5000000000000000);
	BOOST_CHECK(getOrDefault<Json::UInt64>(json, "uint64", 1) == 0x5000000000000000);
	BOOST_CHECK(getOrDefault<Json::UInt64>(json, "no_uint64", 1) == 1);
	BOOST_CHECK(getOrDefault<std::string>(json, "string", "ERROR") == "Hello World!");
	BOOST_CHECK(getOrDefault<std::string>(json, "no_string").empty());
	BOOST_CHECK(getOrDefault<std::string>(json, "no_string", "ERROR") == "ERROR");
}

BOOST_AUTO_TEST_SUITE_END()

}
