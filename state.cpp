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

#include <secp256k1.h>
#include <BlockChain.h>
#include <State.h>
using namespace std;
using namespace eth;

int stateTest()
{
	KeyPair me = sha3("Gav Wood");
	KeyPair myMiner = sha3("Gav's Miner");
//	KeyPair you = sha3("123");

	Defaults::setDBPath("/tmp");

	Overlay stateDB = State::openDB();
	BlockChain bc;
	State s(myMiner.address(), stateDB);

	cout << bc;

	// Sync up - this won't do much until we use the last state.
	s.sync(bc);

	cout << s;

	// Mine to get some ether!
	s.commitToMine(bc);
	while (s.mine(100).completed()) {}
	bc.attemptImport(s.blockData(), stateDB);

	cout << bc;

	s.sync(bc);

	cout << s;

	// Inject a transaction to transfer funds from miner to me.
	bytes tx;
	{
		Transaction t;
		t.nonce = s.transactionsFrom(myMiner.address());
		t.fee = 0;
		t.value = 1000;			// 1e3 wei.
		t.receiveAddress = me.address();
		t.sign(myMiner.secret());
		assert(t.sender() == myMiner.address());
		tx = t.rlp();
	}
	s.execute(tx);

	cout << s;

	// Mine to get some ether and set in stone.
	s.commitToMine(bc);
	while (s.mine(100).completed()) {}
	bc.attemptImport(s.blockData(), stateDB);

	cout << bc;

	s.sync(bc);

	cout << s;

	return 0;
}

