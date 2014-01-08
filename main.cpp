/*
	This file is part of cpp-ethereum.

	cpp-ethereum is free software: you can redistribute it and/or modify
	it under the terms of the GNU General Public License as published by
	the Free Software Foundation, either version 3 of the License, or
	(at your option) any later version.

	Foobar is distributed in the hope that it will be useful,
	but WITHOUT ANY WARRANTY; without even the implied warranty of
	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
	GNU General Public License for more details.

	You should have received a copy of the GNU General Public License
	along with Foobar.  If not, see <http://www.gnu.org/licenses/>.
*/
/** @file main.cpp
 * @author Gav Wood <i@gavwood.com>
 * @date 2014
 * Main test functions.
 */

#include <random>
#include <chrono>
#include <Common.h>
#include <secp256k1.h>
#include "Dagger.h"
#include "RLP.h"
#include "Trie.h"
#include "State.h"
using namespace std;
using namespace std::chrono;
using namespace eth;

// TODO: utilise the shared testdata.

int main()
{
	// Test dagger
	{
		Dagger d(0);
		auto s = steady_clock::now();
		cout << hex << d.eval(0);
		cout << " " << dec << duration_cast<milliseconds>(steady_clock::now() - s).count() << " ms" << endl;
	}
/*
	// Test transaction.
	bytes tx = fromUserHex("88005401010101010101010101010101010101010101011f0de0b6b3a76400001ce8d4a5100080181c373130a009ba1f10285d4e659568bfcfec85067855c5a3c150100815dad4ef98fd37cf0593828c89db94bd6c64e210a32ef8956eaa81ea9307194996a3b879441f5d");
	cout << "TX: " << RLP(tx) << endl;

	Transaction t(tx);
	cout << "SENDER: " << hex << t.sender() << endl;

	bytes sig64 = toBigEndian(t.vrs.r) + toBigEndian(t.vrs.s);
	cout << "SIG: " << sig64.size() << " " << asHex(sig64) << " " << t.vrs.v << endl;

	auto msg = t.rlp(false);
	cout << "TX w/o SIG: " << RLP(msg) << endl;
	cout << "RLP(TX w/o SIG): " << asHex(t.rlpString(false)) << endl;
	std::string hmsg = sha3(t.rlpString(false), false);
	cout << "SHA256(RLP(TX w/o SIG)): 0x" << asHex(hmsg) << endl;

	bytes privkey = sha3Bytes("123");

	secp256k1_start();

	{
		bytes pubkey(65);
		int pubkeylen = 65;

		int ret = secp256k1_ecdsa_seckey_verify(privkey.data());
		cout << "SEC: " << dec << ret << " " << asHex(privkey) << endl;

		ret = secp256k1_ecdsa_pubkey_create(pubkey.data(), &pubkeylen, privkey.data(), 1);
		pubkey.resize(pubkeylen);
		int good = secp256k1_ecdsa_pubkey_verify(pubkey.data(), pubkey.size());
		cout << "PUB: " << dec << ret << " " << pubkeylen << " " << asHex(pubkey) << (good ? " GOOD" : " BAD") << endl;
	}

	// Test roundtrip...
	{
		bytes sig(64);
		u256 nonce = 0;
		int v = 0;
		int ret = secp256k1_ecdsa_sign_compact((byte const*)hmsg.data(), hmsg.size(), sig.data(), privkey.data(), (byte const*)&nonce, &v);
		cout << "MYSIG: " << dec << ret << " " << sig.size() << " " << asHex(sig) << " " << v << endl;

		bytes pubkey(65);
		int pubkeylen = 65;
		ret = secp256k1_ecdsa_recover_compact((byte const*)hmsg.data(), hmsg.size(), (byte const*)sig.data(), pubkey.data(), &pubkeylen, 0, v);
		pubkey.resize(pubkeylen);
		cout << "MYREC: " << dec << ret << " " << pubkeylen << " " << asHex(pubkey) << endl;
	}

	{
		bytes pubkey(65);
		int pubkeylen = 65;
		int ret = secp256k1_ecdsa_recover_compact((byte const*)hmsg.data(), hmsg.size(), (byte const*)sig64.data(), pubkey.data(), &pubkeylen, 0, (int)t.vrs.v - 27);
		pubkey.resize(pubkeylen);
		cout << "RECPUB: " << dec << ret << " " << pubkeylen << " " << asHex(pubkey) << endl;
		cout << "SENDER: " << hex << low160(eth::sha3(bytesConstRef(&pubkey).cropped(1))) << endl;
	}
*/
	{
		Trie t;
		t.insert("dog", "puppy");
		cout << hex << t.hash256() << endl;
		cout << RLP(t.rlp()) << endl;
	}
	{
		Trie t;
		t.insert("bed", "d");
		t.insert("be", "e");
		cout << hex << t.hash256() << endl;
		cout << RLP(t.rlp()) << endl;
	}
	{
		cout << hex << hash256({{"dog", "puppy"}, {"doe", "reindeer"}}) << endl;
		Trie t;
		t.insert("dog", "puppy");
		t.insert("doe", "reindeer");
		cout << hex << t.hash256() << endl;
		cout << RLP(t.rlp()) << endl;
		cout << asHex(t.rlp()) << endl;
	}
	{
		Trie t;

		t.insert("dog", "puppy");
		assert(t.hash256() == hash256({{"dog", "puppy"}}));
		assert(t.at("dog") == "puppy");
		t.insert("doe", "reindeer");
		assert(t.hash256() == hash256({{"dog", "puppy"}, {"doe", "reindeer"}}));
		assert(t.at("doe") == "reindeer");
		assert(t.at("dog") == "puppy");
		t.insert("dogglesworth", "cat");
		assert(t.hash256() == hash256({{"doe", "reindeer"}, {"dog", "puppy"}, {"dogglesworth", "cat"}}));
		assert(t.at("doe") == "reindeer");
		assert(t.at("dog") == "puppy");
		assert(t.at("dogglesworth") == "cat");
		t.remove("dogglesworth");
		t.remove("doe");
		assert(t.at("doe").empty());
		assert(t.at("dogglesworth").empty());
		assert(t.at("dog") == "puppy");
		assert(t.hash256() == hash256({{"dog", "puppy"}}));
		t.insert("horse", "stallion");
		t.insert("do", "verb");
		t.insert("doge", "coin");
		assert(t.hash256() == hash256({{"dog", "puppy"}, {"horse", "stallion"}, {"do", "verb"}, {"doge", "coin"}}));
		assert(t.at("doge") == "coin");
		assert(t.at("do") == "verb");
		assert(t.at("horse") == "stallion");
		assert(t.at("dog") == "puppy");
		t.remove("horse");
		t.remove("do");
		t.remove("doge");
		assert(t.hash256() == hash256({{"dog", "puppy"}}));
		assert(t.at("dog") == "puppy");
		t.remove("dog");

		for (int a = 0; a < 20; ++a)
		{
			StringMap m;
			for (int i = 0; i < 20; ++i)
			{
				auto k = randomWord();
				auto v = toString(i);
				m.insert(make_pair(k, v));
				t.insert(k, v);
				assert(hash256(m) == t.hash256());
			}
			while (!m.empty())
			{
				auto k = m.begin()->first;
				t.remove(k);
				m.erase(k);
				assert(hash256(m) == t.hash256());
			}
		}
	}

	// int of value 15
	assert(RLP("\x0f") == 15);
	assert(asString(rlp(15)) == "\x0f");

	// 3-character string
	assert(RLP("\x43""dog") == "dog");
	assert(asString(rlp("dog")) == "\x43""dog");

	// 2-item list
	RLP twoItemList("\x82\x0f\x43""dog");
	assert(twoItemList.itemCount() == 2);
	assert(twoItemList[0] == 15);
	assert(twoItemList[1] == "dog");
	assert(asString(rlpList(15, "dog")) == "\x82\x0f\x43""dog");

	// 1-byte (8-bit) int
	assert(RLP("\x18\x45") == 69);
	assert(asString(rlp(69)) == "\x18\x45");

	// 2-byte (16-bit) int
	assert(RLP("\x19\x01\x01") == 257);
	assert(asString(rlp(257)) == "\x19\x01\x01");

	// 32-byte (256-bit) int
	assert(RLP("\x37\x10\x01\x02\x03\x04\x05\x06\x07\x08\x09\x0a\x0b\x0c\x0d\x0e\x0f\x10\x11\x12\x13\x14\x15\x16\x17\x18\x19\x1a\x1b\x1c\x1d\x1e\x1f") == bigint("0x100102030405060708090a0b0c0d0e0f101112131415161718191a1b1c1d1e1f"));
	assert(asString(rlp(bigint("0x100102030405060708090a0b0c0d0e0f101112131415161718191a1b1c1d1e1f"))) == "\x37\x10\x01\x02\x03\x04\x05\x06\x07\x08\x09\x0a\x0b\x0c\x0d\x0e\x0f\x10\x11\x12\x13\x14\x15\x16\x17\x18\x19\x1a\x1b\x1c\x1d\x1e\x1f");

	// 33-byte (264-bit) int
	assert(RLP("\x38\x21\x20\x10\x01\x02\x03\x04\x05\x06\x07\x08\x09\x0a\x0b\x0c\x0d\x0e\x0f\x10\x11\x12\x13\x14\x15\x16\x17\x18\x19\x1a\x1b\x1c\x1d\x1e\x1f") == bigint("0x20100102030405060708090A0B0C0D0E0F101112131415161718191A1B1C1D1E1F"));
	assert(asString(rlp(bigint("0x20100102030405060708090A0B0C0D0E0F101112131415161718191A1B1C1D1E1F"))) == "\x38\x21\x20\x10\x01\x02\x03\x04\x05\x06\x07\x08\x09\x0a\x0b\x0c\x0d\x0e\x0f\x10\x11\x12\x13\x14\x15\x16\x17\x18\x19\x1a\x1b\x1c\x1d\x1e\x1f");

	// 56-character string.
	assert(RLP("\x78\x38""Lorem ipsum dolor sit amet, consectetur adipisicing elit") == "Lorem ipsum dolor sit amet, consectetur adipisicing elit");
	assert(asString(rlp("Lorem ipsum dolor sit amet, consectetur adipisicing elit")) == "\x78\x38""Lorem ipsum dolor sit amet, consectetur adipisicing elit");

	/*
	 * Hex-prefix Notation. First nibble has flags: oddness = 2^0 & termination = 2^1
	 * [0,0,1,2,3,4,5]   0x10012345
	 * [0,1,2,3,4,5]     0x00012345
	 * [1,2,3,4,5]       0x112345
	 * [0,0,1,2,3,4]     0x00001234
	 * [0,1,2,3,4]       0x101234
	 * [1,2,3,4]         0x001234
	 * [0,0,1,2,3,4,5,T] 0x30012345
	 * [0,0,1,2,3,4,T]   0x20001234
	 * [0,1,2,3,4,5,T]   0x20012345
	 * [1,2,3,4,5,T]     0x312345
	 * [1,2,3,4,T]       0x201234
	 */
	assert(asHex(hexPrefixEncode({0, 0, 1, 2, 3, 4, 5}, false)) == "10012345");
	assert(asHex(hexPrefixEncode({0, 1, 2, 3, 4, 5}, false)) == "00012345");
	assert(asHex(hexPrefixEncode({1, 2, 3, 4, 5}, false)) == "112345");
	assert(asHex(hexPrefixEncode({0, 0, 1, 2, 3, 4}, false)) == "00001234");
	assert(asHex(hexPrefixEncode({0, 1, 2, 3, 4}, false)) == "101234");
	assert(asHex(hexPrefixEncode({1, 2, 3, 4}, false)) == "001234");
	assert(asHex(hexPrefixEncode({0, 0, 1, 2, 3, 4, 5}, true)) == "30012345");
	assert(asHex(hexPrefixEncode({0, 0, 1, 2, 3, 4}, true)) == "20001234");
	assert(asHex(hexPrefixEncode({0, 1, 2, 3, 4, 5}, true)) == "20012345");
	assert(asHex(hexPrefixEncode({1, 2, 3, 4, 5}, true)) == "312345");
	assert(asHex(hexPrefixEncode({1, 2, 3, 4}, true)) == "201234");

	return 0;
}

