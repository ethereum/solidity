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
/** @file main.cpp
 * @author Gav Wood <i@gavwood.com>
 * @date 2014
 * Main test functions.
 */

#include <libethsupport/TrieDB.h>
#include "TrieHash.h"
#include "MemTrie.h"

#include <boost/test/unit_test.hpp>

int trieTest();
int rlpTest();
int daggerTest();
int cryptoTest();
int stateTest();
int vmTest();
int hexPrefixTest();
int peerTest(int argc, char** argv);

#include <libethsupport/Log.h>
#include <libethcore/BlockInfo.h>
using namespace std;
using namespace eth;

BOOST_AUTO_TEST_CASE(basic_tests)
{
	{
		BasicMap m;
		GenericTrieDB<BasicMap> d(&m);
		d.init();	// initialise as empty tree.
		MemTrie t;
		for (int a = 0; a < 20; ++a)
		{
			StringMap m;
			for (int i = 0; i < 20; ++i)
			{
				auto k = randomWord();
				auto v = toString(i);
				m.insert(make_pair(k, v));
				t.insert(k, v);
				d.insert(k, v);
				assert(hash256(m) == t.hash256());
				assert(hash256(m) == d.root());
			}
			while (!m.empty())
			{
				auto k = m.begin()->first;
				d.remove(k);
				t.remove(k);
				m.erase(k);
				assert(hash256(m) == t.hash256());
				assert(hash256(m) == d.root());
			}
		}
	}

/*	RLPStream s;
	BlockInfo::genesis().fillStream(s, false);
	std::cout << RLP(s.out()) << std::endl;
	std::cout << toHex(s.out()) << std::endl;
	std::cout << sha3(s.out()) << std::endl;*/

//	int r = 0;
//	r += daggerTest();
//	r += stateTest();
//	r += peerTest(argc, argv);
//	BOOST_REQUIRE(!r);
}

