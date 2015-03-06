/*
	This file is part of cpp-ethereum.

	cpp-ethereum is free software: you can redistribute it and/or modify
	it under the terms of the GNU General Public License as published by
	the Free Software Foundation, either version 3 of the License, or
	(at your option) any later version.

	cpp-ethereum is distributed in the hope that it will be useful,
	but WITHOUT ANY WARRANTY; without even the implied warranty of
	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
	GNU General Public License for more details.

	You should have received a copy of the GNU General Public License
	along with cpp-ethereum.  If not, see <http://www.gnu.org/licenses/>.
*/
/** @file trie.cpp
 * @author Gav Wood <i@gavwood.com>
 * @date 2014
 * Trie test functions.
 */

#include <fstream>
#include <random>

#include <boost/test/unit_test.hpp>

#include "JsonSpiritHeaders.h"
#include <libdevcore/CommonIO.h>
#include <libdevcrypto/TrieDB.h>
#include "TrieHash.h"
#include "MemTrie.h"
#include "TestHelper.h"

using namespace std;
using namespace dev;

namespace js = json_spirit;

namespace dev
{
namespace test
{

static unsigned fac(unsigned _i)
{
	return _i > 2 ? _i * fac(_i - 1) : _i;
}

}
}

using dev::operator <<;

BOOST_AUTO_TEST_SUITE(TrieTests)

BOOST_AUTO_TEST_CASE(fat_trie)
{
	h256 r;
	MemoryDB fm;
	{
		FatGenericTrieDB<MemoryDB> ft(&fm);
		ft.init();
		ft.insert(h256("69", h256::FromHex, h256::AlignRight).ref(), h256("414243", h256::FromHex, h256::AlignRight).ref());
		for (auto i: ft)
			cnote << i.first << i.second;
		r = ft.root();
	}
	{
		FatGenericTrieDB<MemoryDB> ft(&fm);
		ft.setRoot(r);
		for (auto i: ft)
			cnote << i.first << i.second;
	}
}

BOOST_AUTO_TEST_CASE(hex_encoded_securetrie_test)
{
	string testPath = test::getTestPath();

	testPath += "/TrieTests";

	cnote << "Testing Secure Trie...";
	js::mValue v;
	string s = asString(contents(testPath + "/hex_encoded_securetrie_test.json"));
	BOOST_REQUIRE_MESSAGE(s.length() > 0, "Contents of 'hex_encoded_securetrie_test.json' is empty. Have you cloned the 'tests' repo branch develop?");
	js::read_string(s, v);
	for (auto& i: v.get_obj())
	{
		cnote << i.first;
		js::mObject& o = i.second.get_obj();
		vector<pair<string, string>> ss;
		for (auto i: o["in"].get_obj())
		{
			ss.push_back(make_pair(i.first, i.second.get_str()));
			if (!ss.back().first.find("0x"))
				ss.back().first = asString(fromHex(ss.back().first.substr(2)));
			if (!ss.back().second.find("0x"))
				ss.back().second = asString(fromHex(ss.back().second.substr(2)));
		}
		for (unsigned j = 0; j < min(1000000000u, dev::test::fac((unsigned)ss.size())); ++j)
		{
			next_permutation(ss.begin(), ss.end());
			MemoryDB m;
			GenericTrieDB<MemoryDB> t(&m);
			MemoryDB hm;
			HashedGenericTrieDB<MemoryDB> ht(&hm);
			MemoryDB fm;
			FatGenericTrieDB<MemoryDB> ft(&fm);
			t.init();
			ht.init();
			ft.init();
			BOOST_REQUIRE(t.check(true));
			BOOST_REQUIRE(ht.check(true));
			BOOST_REQUIRE(ft.check(true));
			for (auto const& k: ss)
			{
				t.insert(k.first, k.second);
				ht.insert(k.first, k.second);
				ft.insert(k.first, k.second);
				BOOST_REQUIRE(t.check(true));
				BOOST_REQUIRE(ht.check(true));
				BOOST_REQUIRE(ft.check(true));
				for (auto i = ft.begin(), j = t.begin(); i != ft.end() && j != t.end(); ++i, ++j)
				{
					BOOST_CHECK_EQUAL(i == ft.end(), j == t.end());
					BOOST_REQUIRE((*i).first.toBytes() == (*j).first.toBytes());
					BOOST_REQUIRE((*i).second.toBytes() == (*j).second.toBytes());
				}
				BOOST_CHECK_EQUAL(ht.root(), ft.root());
			}
			BOOST_REQUIRE(!o["root"].is_null());
			BOOST_CHECK_EQUAL(o["root"].get_str(), "0x" + toHex(ht.root().asArray()));
			BOOST_CHECK_EQUAL(o["root"].get_str(), "0x" + toHex(ft.root().asArray()));
		}
	}
}

BOOST_AUTO_TEST_CASE(trie_test_anyorder)
{
	string testPath = test::getTestPath();

	testPath += "/TrieTests";

	cnote << "Testing Trie...";
	js::mValue v;
	string s = asString(contents(testPath + "/trieanyorder.json"));
	BOOST_REQUIRE_MESSAGE(s.length() > 0, "Contents of 'trieanyorder.json' is empty. Have you cloned the 'tests' repo branch develop?");
	js::read_string(s, v);
	for (auto& i: v.get_obj())
	{
		cnote << i.first;
		js::mObject& o = i.second.get_obj();
		vector<pair<string, string>> ss;
		for (auto i: o["in"].get_obj())
		{
			ss.push_back(make_pair(i.first, i.second.get_str()));
			if (!ss.back().first.find("0x"))
				ss.back().first = asString(fromHex(ss.back().first.substr(2)));
			if (!ss.back().second.find("0x"))
				ss.back().second = asString(fromHex(ss.back().second.substr(2)));
		}
		for (unsigned j = 0; j < min(1000u, dev::test::fac((unsigned)ss.size())); ++j)
		{
			next_permutation(ss.begin(), ss.end());
			MemoryDB m;
			GenericTrieDB<MemoryDB> t(&m);
			MemoryDB hm;
			HashedGenericTrieDB<MemoryDB> ht(&hm);
			MemoryDB fm;
			FatGenericTrieDB<MemoryDB> ft(&fm);
			t.init();
			ht.init();
			ft.init();
			BOOST_REQUIRE(t.check(true));
			BOOST_REQUIRE(ht.check(true));
			BOOST_REQUIRE(ft.check(true));
			for (auto const& k: ss)
			{
				t.insert(k.first, k.second);
				ht.insert(k.first, k.second);
				ft.insert(k.first, k.second);
				BOOST_REQUIRE(t.check(true));
				BOOST_REQUIRE(ht.check(true));
				BOOST_REQUIRE(ft.check(true));
				for (auto i = ft.begin(), j = t.begin(); i != ft.end() && j != t.end(); ++i, ++j)
				{
					BOOST_CHECK_EQUAL(i == ft.end(), j == t.end());
					BOOST_REQUIRE((*i).first.toBytes() == (*j).first.toBytes());
					BOOST_REQUIRE((*i).second.toBytes() == (*j).second.toBytes());
				}
				BOOST_CHECK_EQUAL(ht.root(), ft.root());
			}
			BOOST_REQUIRE(!o["root"].is_null());
			BOOST_CHECK_EQUAL(o["root"].get_str(), "0x" + toHex(t.root().asArray()));
			BOOST_CHECK_EQUAL(ht.root(), ft.root());
		}
	}
}

BOOST_AUTO_TEST_CASE(trie_tests_ordered)
{
	string testPath = test::getTestPath();

	testPath += "/TrieTests";

	cnote << "Testing Trie...";
	js::mValue v;
	string s = asString(contents(testPath + "/trietest.json"));
	BOOST_REQUIRE_MESSAGE(s.length() > 0, "Contents of 'trietest.json' is empty. Have you cloned the 'tests' repo branch develop?");
	js::read_string(s, v);

	for (auto& i: v.get_obj())
	{
		cnote << i.first;
		js::mObject& o = i.second.get_obj();
		vector<pair<string, string>> ss;
		vector<string> keysToBeDeleted;
		for (auto& i: o["in"].get_array())
		{
			vector<string> values;
			for (auto& s: i.get_array())
			{
				if (s.type() == json_spirit::str_type)
					values.push_back(s.get_str());
				else if (s.type() == json_spirit::null_type)
				{
					// mark entry for deletion
					values.push_back("");
					if (!values[0].find("0x"))
						values[0] = asString(fromHex(values[0].substr(2)));
					keysToBeDeleted.push_back(values[0]);
				}
				else
					BOOST_FAIL("Bad type (expected string)");
			}

			BOOST_REQUIRE(values.size() == 2);
			ss.push_back(make_pair(values[0], values[1]));
			if (!ss.back().first.find("0x"))
				ss.back().first = asString(fromHex(ss.back().first.substr(2)));
			if (!ss.back().second.find("0x"))
				ss.back().second = asString(fromHex(ss.back().second.substr(2)));
		}

		MemoryDB m;
		GenericTrieDB<MemoryDB> t(&m);
		MemoryDB hm;
		HashedGenericTrieDB<MemoryDB> ht(&hm);
		MemoryDB fm;
		FatGenericTrieDB<MemoryDB> ft(&fm);
		t.init();
		ht.init();
		ft.init();
		BOOST_REQUIRE(t.check(true));
		BOOST_REQUIRE(ht.check(true));
		BOOST_REQUIRE(ft.check(true));

		for (auto const& k: ss)
		{
			if (find(keysToBeDeleted.begin(), keysToBeDeleted.end(), k.first) != keysToBeDeleted.end() && k.second.empty())
				t.remove(k.first), ht.remove(k.first), ft.remove(k.first);
			else
				t.insert(k.first, k.second), ht.insert(k.first, k.second), ft.insert(k.first, k.second);
			BOOST_REQUIRE(t.check(true));
			BOOST_REQUIRE(ht.check(true));
			BOOST_REQUIRE(ft.check(true));
			for (auto i = ft.begin(), j = t.begin(); i != ft.end() && j != t.end(); ++i, ++j)
			{
				BOOST_CHECK_EQUAL(i == ft.end(), j == t.end());
				BOOST_REQUIRE((*i).first.toBytes() == (*j).first.toBytes());
				BOOST_REQUIRE((*i).second.toBytes() == (*j).second.toBytes());
			}
			BOOST_CHECK_EQUAL(ht.root(), ft.root());
		}

		BOOST_REQUIRE(!o["root"].is_null());
		BOOST_CHECK_EQUAL(o["root"].get_str(), "0x" + toHex(t.root().asArray()));
	}
}

inline h256 stringMapHash256(StringMap const& _s)
{
	return hash256(_s);
}

BOOST_AUTO_TEST_CASE(moreTrieTests)
{
	cnote << "Testing Trie more...";
#if 0
	// More tests...
	{
		MemoryDB m;
		GenericTrieDB<MemoryDB> t(&m);
		t.init();	// initialise as empty tree.
		cout << t;
		cout << m;
		cout << t.root() << endl;
		cout << hash256(StringMap()) << endl;

		t.insert(string("tesz"), string("test"));
		cout << t;
		cout << m;
		cout << t.root() << endl;
		cout << stringMapHash256({{"test", "test"}}) << endl;

		t.insert(string("tesa"), string("testy"));
		cout << t;
		cout << m;
		cout << t.root() << endl;
		cout << stringMapHash256({{"test", "test"}, {"te", "testy"}}) << endl;
		cout << t.at(string("test")) << endl;
		cout << t.at(string("te")) << endl;
		cout << t.at(string("t")) << endl;

		t.remove(string("te"));
		cout << m;
		cout << t.root() << endl;
		cout << stringMapHash256({{"test", "test"}}) << endl;

		t.remove(string("test"));
		cout << m;
		cout << t.root() << endl;
		cout << hash256(StringMap()) << endl;
	}
	{
		MemoryDB m;
		GenericTrieDB<MemoryDB> t(&m);
		t.init();	// initialise as empty tree.
		t.insert(string("a"), string("A"));
		t.insert(string("b"), string("B"));
		cout << t;
		cout << m;
		cout << t.root() << endl;
		cout << stringMapHash256({{"b", "B"}, {"a", "A"}}) << endl;
		cout << RLP(rlp256({{"b", "B"}, {"a", "A"}})) << endl;
	}
	{
		MemTrie t;
		t.insert("dog", "puppy");
		cout << hex << t.hash256() << endl;
		cout << RLP(t.rlp()) << endl;
	}
	{
		MemTrie t;
		t.insert("bed", "d");
		t.insert("be", "e");
		cout << hex << t.hash256() << endl;
		cout << RLP(t.rlp()) << endl;
	}
	{
		cout << hex << stringMapHash256({{"dog", "puppy"}, {"doe", "reindeer"}}) << endl;
		MemTrie t;
		t.insert("dog", "puppy");
		t.insert("doe", "reindeer");
		cout << hex << t.hash256() << endl;
		cout << RLP(t.rlp()) << endl;
		cout << toHex(t.rlp()) << endl;
	}
#endif
	{
		MemoryDB m;
		GenericTrieDB<MemoryDB> d(&m);
		d.init();	// initialise as empty tree.
		MemTrie t;
		StringMap s;

		auto add = [&](char const* a, char const* b)
		{
			d.insert(string(a), string(b));
			t.insert(a, b);
			s[a] = b;

			/*cout << endl << "-------------------------------" << endl;
			cout << a << " -> " << b << endl;
			cout << d;
			cout << m;
			cout << d.root() << endl;
			cout << hash256(s) << endl;*/

			BOOST_REQUIRE(d.check(true));
			BOOST_REQUIRE_EQUAL(t.hash256(), hash256(s));
			BOOST_REQUIRE_EQUAL(d.root(), hash256(s));
			for (auto const& i: s)
			{
				(void)i;
				BOOST_REQUIRE_EQUAL(t.at(i.first), i.second);
				BOOST_REQUIRE_EQUAL(d.at(i.first), i.second);
			}
		};

		auto remove = [&](char const* a)
		{
			s.erase(a);
			t.remove(a);
			d.remove(string(a));

			/*cout << endl << "-------------------------------" << endl;
			cout << "X " << a << endl;
			cout << d;
			cout << m;
			cout << d.root() << endl;
			cout << hash256(s) << endl;*/

			BOOST_REQUIRE(d.check(true));
			BOOST_REQUIRE(t.at(a).empty());
			BOOST_REQUIRE(d.at(string(a)).empty());
			BOOST_REQUIRE_EQUAL(t.hash256(), hash256(s));
			BOOST_REQUIRE_EQUAL(d.root(), hash256(s));
			for (auto const& i: s)
			{
				(void)i;
				BOOST_REQUIRE_EQUAL(t.at(i.first), i.second);
				BOOST_REQUIRE_EQUAL(d.at(i.first), i.second);
			}
		};

		add("dogglesworth", "cat");
		add("doe", "reindeer");
		remove("dogglesworth");
		add("horse", "stallion");
		add("do", "verb");
		add("doge", "coin");
		remove("horse");
		remove("do");
		remove("doge");
		remove("doe");
	}
}

BOOST_AUTO_TEST_CASE(trieLowerBound)
{
	cnote << "Stress-testing Trie.lower_bound...";
	if (0)
	{
		MemoryDB dm;
		EnforceRefs e(dm, true);
		GenericTrieDB<MemoryDB> d(&dm);
		d.init();	// initialise as empty tree.
		for (int a = 0; a < 20; ++a)
		{
			StringMap m;
			for (int i = 0; i < 50; ++i)
			{
				auto k = randomWord();
				auto v = toString(i);
				m[k] = v;
				d.insert(k, v);
			}

			for (auto i: d)
			{
				auto it = d.lower_bound(i.first);
				for (auto iit = d.begin(); iit != d.end(); ++iit)
					if ((*iit).first.toString() >= i.first.toString())
					{
						BOOST_REQUIRE(it == iit);
						break;
					}
			}
			for (unsigned i = 0; i < 100; ++i)
			{
				auto k = randomWord();
				auto it = d.lower_bound(k);
				for (auto iit = d.begin(); iit != d.end(); ++iit)
					if ((*iit).first.toString() >= k)
					{
						BOOST_REQUIRE(it == iit);
						break;
					}
			}

		}
	}
}

BOOST_AUTO_TEST_CASE(trieStess)
{
	cnote << "Stress-testing Trie...";
	if (0)
	{
		MemoryDB m;
		MemoryDB dm;
		EnforceRefs e(dm, true);
		GenericTrieDB<MemoryDB> d(&dm);
		d.init();	// initialise as empty tree.
		MemTrie t;
		BOOST_REQUIRE(d.check(true));
		for (int a = 0; a < 20; ++a)
		{
			StringMap m;
			for (int i = 0; i < 50; ++i)
			{
				auto k = randomWord();
				auto v = toString(i);
				m[k] = v;
				t.insert(k, v);
				d.insert(k, v);
				BOOST_REQUIRE_EQUAL(hash256(m), t.hash256());
				BOOST_REQUIRE_EQUAL(hash256(m), d.root());
				BOOST_REQUIRE(d.check(true));
			}
			while (!m.empty())
			{
				auto k = m.begin()->first;
				auto v = m.begin()->second;
				d.remove(k);
				t.remove(k);
				m.erase(k);
				if (!d.check(true))
				{
					// cwarn << m;
					for (auto i: d)
						cwarn << i.first.toString() << i.second.toString();

					MemoryDB dm2;
					EnforceRefs e2(dm2, true);
					GenericTrieDB<MemoryDB> d2(&dm2);
					d2.init();	// initialise as empty tree.
					for (auto i: d)
						d2.insert(i.first, i.second);

					cwarn << "Good:" << d2.root();
//					for (auto i: dm2.get())
//						cwarn << i.first.abridged() << ": " << RLP(i.second);
					d2.debugStructure(cerr);
					cwarn << "Broken:" << d.root();	// Leaves an extension -> extension (3c1... -> 742...)
//					for (auto i: dm.get())
//						cwarn << i.first.abridged() << ": " << RLP(i.second);
					d.debugStructure(cerr);

					d2.insert(k, v);
					cwarn << "Pres:" << d2.root();
//					for (auto i: dm2.get())
//						cwarn << i.first.abridged() << ": " << RLP(i.second);
					d2.debugStructure(cerr);
					g_logVerbosity = 99;
					d2.remove(k);
					g_logVerbosity = 4;

					cwarn << "Good?" << d2.root();
				}
				BOOST_REQUIRE(d.check(true));
				BOOST_REQUIRE_EQUAL(hash256(m), t.hash256());
				BOOST_REQUIRE_EQUAL(hash256(m), d.root());
			}
		}
	}
}

BOOST_AUTO_TEST_SUITE_END()


