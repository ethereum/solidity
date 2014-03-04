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

// TODO: utilise the shared testdata.

int trieTest();
int rlpTest();
int daggerTest();
int cryptoTest();
int stateTest();
int vmTest();
int hexPrefixTest();
int peerTest(int argc, char** argv);

#include <BlockInfo.h>
using namespace eth;

int main(int, char**)
{
/*	RLPStream s;
	BlockInfo::genesis().fillStream(s, false);
	std::cout << RLP(s.out()) << std::endl;
	std::cout << toHex(s.out()) << std::endl;
	std::cout << sha3(s.out()) << std::endl;*/

	int r = 0;
	r += hexPrefixTest();
	r += rlpTest();
	r += trieTest();
	r += vmTest();
	r += cryptoTest();	// TODO: Put in tests repo.
//	r += daggerTest();
//	r += stateTest();
//	r += peerTest(argc, argv);
	assert(!r);
	return 0;
}

