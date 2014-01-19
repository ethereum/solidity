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
/** @file state.cpp
 * @author Gav Wood <i@gavwood.com>
 * @date 2014
 * State test functions.
 */

#include <State.h>
using namespace std;
using namespace eth;

struct KeyPair
{
	KeyPair() {}
	KeyPair(PrivateKey _k): priv(_k), addr(toPublic(_k)) {}
	PrivateKey priv;
	Address addr;
};

int stateTest()
{
	KeyPair me = sha3("Gav Wood");
	KeyPair myMiner = sha3("Gav's Miner");
//	KeyPair you = sha3("123");

	State s(myMiner.addr);

	// Mine to get some ether!
	s.mine();

	bytes tx;
	{
		Transaction t;
		t.nonce = s.transactionsFrom(myMiner.addr);
		t.fee = 0;
		t.value = 1;			// 1 wei.
		t.receiveAddress = me.addr;
		t.sign(myMiner.priv);
		tx = t.rlp();
	}
	cout << RLP(tx) << endl;
	s.execute(tx);

	// TODO: Mine to set in stone.

	return 0;
}

