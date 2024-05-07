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


namespace solidity::util::test
{

BOOST_AUTO_TEST_SUITE(JsonTest, *boost::unit_test::label("nooptions"))

BOOST_AUTO_TEST_CASE(json_types)
{
	auto check = [](Json value, std::string const& expectation) {
		BOOST_CHECK(jsonCompactPrint(value) == expectation);
	};

	Json value;
	BOOST_CHECK(value.empty());
	value = {};
	BOOST_CHECK(value.empty());
	value = Json();
	BOOST_CHECK(value.empty());
	value = Json();
	BOOST_CHECK(value.empty());

	check(value, "null");
	check({}, "null");
	check(Json(), "null");
	check(Json(), "null");
	check(Json::object(), "{}");
	check(Json::array(), "[]");
	check(1, "1");
	check(static_cast<uint32_t>(-1), "4294967295");
	check(1, "1");
	check(static_cast<uint64_t>(-1), "18446744073709551615");
	check(1, "1");
	check(static_cast<uint64_t>(-1), "18446744073709551615");
	check(0xffffffff, "4294967295");
	check(Json("test"), "\"test\"");
	check("test", "\"test\"");
	check(true, "true");

	value = Json::object();
	value["key"] = "value";
	check(value, "{\"key\":\"value\"}");

	value = Json::array();
	value.push_back(1);
	value.push_back(2);
	check(value, "[1,2]");
}

BOOST_AUTO_TEST_CASE(json_pretty_print)
{
	Json json;
	Json jsonChild;

	jsonChild["3.1"] = "3.1";
	jsonChild["3.2"] = 2;
	json["1"] = 1;
	json["2"] = "2";
	json["3"] = jsonChild;
	json["4"] = "ऑ ऒ ओ औ क ख";
	json["5"] = "\\xff\\xfe";

	BOOST_CHECK(
	"{\n"
	"  \"1\": 1,\n"
	"  \"2\": \"2\",\n"
	"  \"3\": {\n"
	"    \"3.1\": \"3.1\",\n"
	"    \"3.2\": 2\n"
	"  },\n"
	"  \"4\": \"\\u0911 \\u0912 \\u0913 \\u0914 \\u0915 \\u0916\",\n"
	"  \"5\": \"\\\\xff\\\\xfe\"\n"
	"}" == jsonPrettyPrint(json));
}

BOOST_AUTO_TEST_CASE(json_compact_print)
{
	Json json;
	Json jsonChild;

	jsonChild["3.1"] = "3.1";
	jsonChild["3.2"] = 2;
	json["1"] = 1;
	json["2"] = "2";
	json["3"] = jsonChild;
	json["4"] = "ऑ ऒ ओ औ क ख";
	json["5"] = "\x10";
	json["6"] = "\u4e2d";

	BOOST_CHECK(R"({"1":1,"2":"2","3":{"3.1":"3.1","3.2":2},"4":"\u0911 \u0912 \u0913 \u0914 \u0915 \u0916","5":"\u0010","6":"\u4e2d"})" == jsonCompactPrint(json));
}

BOOST_AUTO_TEST_CASE(parse_json_strict)
{
	// In this test we check conformance against JSON.parse (https://tc39.es/ecma262/multipage/structured-data.html#sec-json.parse)
	// and ECMA-404 (https://www.ecma-international.org/publications-and-standards/standards/ecma-404/)

	Json json;
	std::string errors;

	// Just parse a valid json input
	BOOST_CHECK(jsonParseStrict("{\"1\":1,\"2\":\"2\",\"3\":{\"3.1\":\"3.1\",\"3.2\":2}}", json, &errors));
	BOOST_CHECK(json["1"] == 1);
	BOOST_CHECK(json["2"] == "2");
	BOOST_CHECK(json["3"]["3.1"] == "3.1");
	BOOST_CHECK(json["3"]["3.2"] == 2);

	// Trailing garbage is not allowed in ECMA-262
	BOOST_CHECK(!jsonParseStrict("{\"1\":2,\"2\":\"2\",\"3\":{\"3.1\":\"3.1\",\"3.2\":3}}}}}}}}}}", json, &errors));

	// According to ECMA-404 object, array, number, string, true, false, null are allowed
	// ... but JSONCPP disallows value types
	BOOST_CHECK(jsonParseStrict("[]", json, &errors));
	BOOST_CHECK(json.is_array());
	BOOST_CHECK(jsonParseStrict("{}", json, &errors));
	BOOST_CHECK(json.is_object());
	BOOST_CHECK(jsonParseStrict("1", json, &errors));
	BOOST_CHECK(json.is_number());
	BOOST_CHECK(jsonParseStrict("\"hello\"", json, &errors));
	BOOST_CHECK(json.is_string());
	BOOST_CHECK(jsonParseStrict("true", json, &errors));
	BOOST_CHECK(json.is_boolean());
	BOOST_CHECK(jsonParseStrict("null", json, &errors));
	BOOST_CHECK(json.is_null());

	// Single quotes are also disallowed by ECMA-404
	BOOST_CHECK(!jsonParseStrict("'hello'", json, &errors));
	// BOOST_CHECK(json.is_string());

	// Only string keys in objects are allowed in ECMA-404
	BOOST_CHECK(!jsonParseStrict("{ 42: \"hello\" }", json, &errors));

	// According to ECMA-404 hex escape sequences are not allowed, only unicode (\uNNNN) and
	// a few control characters (\b, \f, \n, \r, \t)

	BOOST_CHECK(jsonParseStrict("[ \"\xF0\x9F\x98\x8A\" ]", json, &errors));
	BOOST_CHECK(json.is_array());
	BOOST_CHECK(json[0] == "😊");
	BOOST_CHECK(json[0] == "\xF0\x9F\x98\x8A");
}

BOOST_AUTO_TEST_CASE(json_isOfType)
{
	Json json;

	json["float"] = 3.1f;
	json["double"] = 3.1;
	json["int"] = 2;
	json["int64"] = 0x4000000000000000;
	json["string"] = "Hello World!";

	BOOST_CHECK(isOfType<float>(json["float"]));
	BOOST_CHECK(isOfType<double>(json["double"]));
	BOOST_CHECK(isOfType<int>(json["int"]));
	BOOST_CHECK(isOfType<int>(json["int"]));
	BOOST_CHECK(isOfType<uint64_t>(json["int"]));
	BOOST_CHECK(isOfType<uint64_t>(json["int"]));
	BOOST_CHECK(isOfType<int>(json["int"]));
	BOOST_CHECK(isOfType<int64_t>(json["int64"]));
	BOOST_CHECK(isOfType<uint64_t>(json["int64"]));
	BOOST_CHECK(isOfType<uint64_t >(json["int64"]));
	BOOST_CHECK(isOfType<std::string>(json["string"]));
	BOOST_CHECK(!isOfType<int>(json["int64"]));
	BOOST_CHECK(!isOfType<int>(json["double"]));
	BOOST_CHECK(!isOfType<float>(json["string"]));
	BOOST_CHECK(!isOfType<double>(json["string"]));
	BOOST_CHECK(!isOfType<int>(json["string"]));
	BOOST_CHECK(!isOfType<int64_t>(json["string"]));
	BOOST_CHECK(!isOfType<unsigned >(json["string"]));
	BOOST_CHECK(!isOfType<uint64_t >(json["string"]));
}

BOOST_AUTO_TEST_CASE(json_isisOfTypeIfExists)
{
	Json json;

	json["float"] = 3.1f;
	json["double"] = 3.1;
	json["int"] = 2;
	json["uint"] = 2u;
	json["int64"] = 0x4000000000000000;
	json["uint64"] = 0x4000000000000000u;
	json["string"] = "Hello World!";

	BOOST_CHECK(isOfTypeIfExists<float>(json, "float"));
	BOOST_CHECK(isOfTypeIfExists<double>(json, "double"));
	BOOST_CHECK(isOfTypeIfExists<int>(json, "int"));
	BOOST_CHECK(isOfTypeIfExists<int>(json, "int"));
	BOOST_CHECK(isOfTypeIfExists<unsigned>(json, "uint"));
	BOOST_CHECK(isOfTypeIfExists<int64_t>(json, "int"));
	BOOST_CHECK(isOfTypeIfExists<int64_t >(json, "int64"));
	BOOST_CHECK(isOfTypeIfExists<uint64_t>(json, "uint64"));
	BOOST_CHECK(isOfTypeIfExists<std::string>(json, "string"));
	BOOST_CHECK(!isOfTypeIfExists<int>(json, "int64"));
	BOOST_CHECK(!isOfTypeIfExists<int>(json, "double"));
	BOOST_CHECK(!isOfTypeIfExists<float>(json, "string"));
	BOOST_CHECK(!isOfTypeIfExists<double>(json, "string"));
	BOOST_CHECK(!isOfTypeIfExists<int>(json, "string"));
	BOOST_CHECK(!isOfTypeIfExists<int64_t>(json, "string"));
	BOOST_CHECK(!isOfTypeIfExists<unsigned>(json, "string"));
	BOOST_CHECK(!isOfTypeIfExists<uint64_t>(json, "string"));
	BOOST_CHECK(isOfTypeIfExists<uint64_t>(json, "NOT_EXISTING"));
}

BOOST_AUTO_TEST_CASE(json_getOrDefault)
{
	Json json;

	json["float"] = 3.1f;
	json["double"] = 3.1;
	json["int"] = 2;
	json["int64"] = 0x4000000000000000;
	json["uint64"] = 0x5000000000000000;
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
	BOOST_CHECK(getOrDefault<int>(json, "int") == 2);
	BOOST_CHECK(getOrDefault<int>(json, "int", -1) == 2);
	BOOST_CHECK(getOrDefault<int>(json, "no_int", -1) == -1);
	BOOST_CHECK(getOrDefault<unsigned>(json, "int") == 2);
	BOOST_CHECK(getOrDefault<unsigned>(json, "int", 1) == 2);
	BOOST_CHECK(getOrDefault<unsigned>(json, "no_int", 1) == 1);
	BOOST_CHECK(getOrDefault<int64_t>(json, "int") == 2);
	BOOST_CHECK(getOrDefault<int64_t>(json, "int", -1) == 2);
	BOOST_CHECK(getOrDefault<int64_t>(json, "no_int", -1) == -1);
	BOOST_CHECK(getOrDefault<int64_t>(json, "int64") == 0x4000000000000000);
	BOOST_CHECK(getOrDefault<int64_t>(json, "int64", -1) == 0x4000000000000000);
	BOOST_CHECK(getOrDefault<int64_t>(json, "no_int64", -1) == -1);
	BOOST_CHECK(getOrDefault<uint64_t>(json, "int64") == 0x4000000000000000);
	BOOST_CHECK(getOrDefault<uint64_t>(json, "int64", 1) == 0x4000000000000000);
	BOOST_CHECK(getOrDefault<uint64_t>(json, "no_int64", 1) == 1);
	BOOST_CHECK(getOrDefault<uint64_t>(json, "uint64") == 0x5000000000000000);
	BOOST_CHECK(getOrDefault<uint64_t>(json, "uint64", 1) == 0x5000000000000000);
	BOOST_CHECK(getOrDefault<uint64_t>(json, "no_uint64", 1) == 1);
	BOOST_CHECK(getOrDefault<std::string>(json, "string", "ERROR") == "Hello World!");
	BOOST_CHECK(getOrDefault<std::string>(json, "no_string").empty());
	BOOST_CHECK(getOrDefault<std::string>(json, "no_string", "ERROR") == "ERROR");
}

BOOST_AUTO_TEST_CASE(json_get)
{
	Json json;

	json["float"] = 3.1f;
	json["double"] = 3.1;
	json["int"] = 2;
	json["int64"] = 0x4000000000000000;
	json["uint64"] = 0x5000000000000000;
	json["string"] = "Hello World!";

	BOOST_CHECK(get<float>(json["float"]) == 3.1f);
	BOOST_CHECK(get<double>(json["double"]) == 3.1);
	BOOST_CHECK(get<int>(json["int"]) == 2);
	BOOST_CHECK(get<int>(json["int"]) == 2);
	BOOST_CHECK(get<unsigned>(json["int"]) == 2);
	BOOST_CHECK(get<int64_t>(json["int"]) == 2);
	BOOST_CHECK(get<int64_t>(json["int64"]) == 0x4000000000000000);
	BOOST_CHECK(get<uint64_t>(json["int64"]) == 0x4000000000000000);
	BOOST_CHECK(get<uint64_t>(json["uint64"]) == 0x5000000000000000);
	BOOST_CHECK(get<std::string>(json["string"]) == "Hello World!");
}

template<typename T>
void json_test_int_range()
{
	Json max = {{"v", std::numeric_limits<T>::max()}};
	Json min = {{"v", std::numeric_limits<T>::min()}};
	Json overflow = {{"v", static_cast<int64_t>(std::numeric_limits<T>::max()) + 1}};
	Json underflow = {{"v", static_cast<int64_t>(std::numeric_limits<T>::min()) - 1}};
	BOOST_CHECK(getOrDefault<T>(max, "v", 0) == std::numeric_limits<T>::max());
	BOOST_CHECK(getOrDefault<T>(min, "v", 0) == std::numeric_limits<T>::min());
	BOOST_CHECK(getOrDefault<T>(overflow, "v", 0) == 0);
	BOOST_CHECK(getOrDefault<T>(underflow, "v", 0) == 0);
	BOOST_CHECK(get<T>(max["v"]) == std::numeric_limits<T>::max());
	BOOST_CHECK(get<T>(min["v"]) == std::numeric_limits<T>::min());
	BOOST_CHECK_THROW(get<T>(overflow["v"]), InvalidType);
	BOOST_CHECK_THROW(get<T>(underflow["v"]), InvalidType);
}

BOOST_AUTO_TEST_CASE(json_range_checks)
{
	json_test_int_range<int32_t>();
	json_test_int_range<uint32_t>();
	json_test_int_range<int16_t>();
	json_test_int_range<uint16_t>();
	json_test_int_range<int8_t>();
	json_test_int_range<uint8_t>();

	Json overflow = {{"v", static_cast<double>(std::numeric_limits<float>::max()) * 2}};
	Json underflow = {{"v", static_cast<double>(std::numeric_limits<float>::min()) / 2}};
	BOOST_CHECK(getOrDefault<float>({{"v", std::numeric_limits<float>::max()}}, "v", 0) == std::numeric_limits<float>::max());
	BOOST_CHECK(getOrDefault<float>({{"v", std::numeric_limits<float>::min()}}, "v", 0) == std::numeric_limits<float>::min());
	BOOST_CHECK(getOrDefault<float>(overflow, "v", 0) == 0);
	BOOST_CHECK(getOrDefault<float>(underflow, "v", 0) == 0);
	BOOST_CHECK_THROW(get<float>(overflow["v"]), InvalidType);
	BOOST_CHECK_THROW(get<float>(underflow["v"]), InvalidType);
}

BOOST_AUTO_TEST_SUITE_END()

}
