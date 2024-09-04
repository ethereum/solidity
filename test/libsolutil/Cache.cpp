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
 * @date 2024
 * Unit tests for libsolutil/Cache.h.
 */

#include <libsolutil/Cache.h>
#include <libsolutil/JSON.h>

#include <test/Common.h>

#include <boost/test/unit_test.hpp>


namespace solidity::util::test
{

BOOST_AUTO_TEST_SUITE(CacheTest, *boost::unit_test::label("nooptions"))

BOOST_AUTO_TEST_CASE(smoke_test)
{
	Cache<std::size_t, int>::Entry cached_int;
	Cache<std::size_t, int> int_cache;
	cached_int = int_cache.set(45);
	BOOST_CHECK(int_cache.set(45) == cached_int);
	BOOST_CHECK(int_cache.set(46) == int_cache.set(46));

	Cache<h256, int> h256_int_cache;
	cached_int = h256_int_cache.set(45);
	BOOST_CHECK(h256_int_cache.set(45) == cached_int);
	BOOST_CHECK(h256_int_cache.set(46) == h256_int_cache.set(46));

	Cache<size_t, std::string> string_cache;
	std::shared_ptr<std::string> cached_string = string_cache.set("");
	BOOST_CHECK(string_cache.set("") == cached_string);
	BOOST_CHECK(string_cache.set("hello") == string_cache.set("hello"));

	Cache<h256, std::string> h256_string_cache;
	cached_string = h256_string_cache.set("");
	BOOST_CHECK(h256_string_cache.set("") == cached_string);
	BOOST_CHECK(h256_string_cache.set("hello") == h256_string_cache.set("hello"));

	Cache<h256, Json> h256_json_cache;
	Cache<h256, Json>::Entry cached_json = h256_json_cache.set({});
	BOOST_CHECK(h256_json_cache.set({}) == cached_json);
	BOOST_CHECK(h256_json_cache.set({{"a", "b"}}) == h256_json_cache.set({{"a", "b"}}));

	Cache<size_t, Json> json_cache;
	cached_json = json_cache.set({});
	BOOST_CHECK(json_cache.set({}) == cached_json);
	BOOST_CHECK(json_cache.set({{"a", "b"}}) == json_cache.set({{"a", "b"}}));

	Cache<size_t, Json>::Ptr json_cache_ptr = std::make_shared<Cache<size_t, Json>>();
	cached_json = json_cache_ptr->set({});
	BOOST_CHECK(json_cache_ptr->set({}) == cached_json);
	BOOST_CHECK(json_cache_ptr->set({{"a", "b"}}) == json_cache_ptr->set({{"a", "b"}}));

	Cache<h256, Json>::Ptr h256_json_cache_ptr = std::make_shared<Cache<h256, Json>>();
	cached_json = h256_json_cache_ptr->set({});
	BOOST_CHECK(h256_json_cache_ptr->set({}) == cached_json);
	BOOST_CHECK(h256_json_cache_ptr->set({{"a", "b"}}) == h256_json_cache_ptr->set({{"a", "b"}}));
}

BOOST_AUTO_TEST_CASE(cache_hash_get)
{
	auto test = [](auto _value0, auto _hash0, auto _value1, auto _hash1) -> void
	{
		typedef decltype(_hash0) Hash;
		typedef decltype(_value0) Value;
		static_assert(
			std::is_same_v<decltype(_hash0), decltype(_hash1)>, "types of _hash0 and _hash1 need to be the same"
		);
		static_assert(
			std::is_same_v<decltype(_value0), decltype(_value1)>,
			"types of _value0 and _value1 need to be the same"
		);
		typename Cache<Hash, Value>::Ptr cache = std::make_shared<Cache<Hash, Value>>();
		Hash hash0 = cache->hash(_value0);
		Hash hash1 = cache->hash(_value1);
		BOOST_CHECK_EQUAL(hash0, _hash0);
		BOOST_CHECK_EQUAL(hash1, _hash1);
		BOOST_CHECK(cache->get(_hash0) == cache->end());
		BOOST_CHECK(cache->get(_hash1) == cache->end());
		cache->set(_value0);
		BOOST_CHECK(cache->get(_hash0) != cache->end());
		BOOST_CHECK(*(cache->get(_hash0)->second) == _value0);
		BOOST_CHECK(cache->get(hash1) == cache->end());
		cache->set(_value1);
		BOOST_CHECK(cache->get(hash1) != cache->end());
		BOOST_CHECK(*(cache->get(hash1)->second) == _value1);
	};
	test(0, static_cast<size_t>(0), 1, static_cast<size_t>(1));
	test(0.1f, static_cast<size_t>(1036831949), 0.2f, static_cast<size_t>(1045220557));
	test(0.1, static_cast<size_t>(4591870180066957722), 0.2, static_cast<size_t>(4596373779694328218));
	test(
		"",
		// different boost versions seem to return different hashes for strings, so we just calculate the correct hashes here.
		Cache<size_t, const char*>::hash(""),
		"HELLO WORLD",
		Cache<size_t, const char*>::hash("HELLO WORLD")
	);
	test(
		std::string(),
		// different boost versions seem to return different hashes for strings, so we just calculate the correct hashes here.
		Cache<size_t, std::string>::hash(std::string()),
		std::string("HELLO WORLD"),
		Cache<size_t, std::string>::hash("HELLO WORLD")
	);
	test(
		Json{},
		// different boost versions seem to return different hashes for strings, so we just calculate the correct hashes here.
		Cache<size_t, Json>::hash(Json{}),
		Json{{"HELLO", "WORLD"}},
		Cache<size_t, Json>::hash(Json{{"HELLO", "WORLD"}})
	);
	test(
		0,
		static_cast<h256>("044852b2a670ade5407e78fb2863c51de9fcb96542a07186fe3aeda6bb8a116d"),
		1,
		static_cast<h256>("c89efdaa54c0f20c7adf612882df0950f5a951637e0307cdcb4c672f298b8bc6")
	);
	test(
		0.1f,
		static_cast<h256>("8cd160c72d102a6747abd189ac21d4a1f802e3fcc1bb8fc78cc4d558df0c7c21"),
		0.2f,
		static_cast<h256>("c1907d585d0b0e66920f6383717e2e9e7c44e42ba86ef49b0e19983ffd702288")
	);
	test(
		0.1,
		static_cast<h256>("8cd160c72d102a6747abd189ac21d4a1f802e3fcc1bb8fc78cc4d558df0c7c21"),
		0.2,
		static_cast<h256>("c1907d585d0b0e66920f6383717e2e9e7c44e42ba86ef49b0e19983ffd702288")
	);
	test(
		"",
		static_cast<h256>("c5d2460186f7233c927e7db2dcc703c0e500b653ca82273b7bfad8045d85a470"),
		"HELLO WORLD",
		static_cast<h256>("6c1223964c4674e3797678a0af4dee8b46a8ac1471d97cf37c136ce0937fa0df")
	);
	test(
		Json{},
		static_cast<h256>("efbde2c3aee204a69b7696d4b10ff31137fe78e3946306284f806e2dfc68b805"),
		Json{{"HELLO", "WORLD"}},
		static_cast<h256>("90e22a3e1e4d820d1a76c04d130c08dd1cabb2c1d22ad7d582a0b5415d797bde")
	);
}

BOOST_AUTO_TEST_SUITE_END()

}
