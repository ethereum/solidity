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
/** @file stateOriginal.cpp
 * @author Gav Wood <i@gavwood.com>
 * @date 2014
 * State test functions.
 */

#include <boost/filesystem/operations.hpp>
#include <secp256k1/secp256k1.h>
#include <libethereum/CanonBlockChain.h>
#include <libethereum/State.h>
#include <libethereum/Defaults.h>
using namespace std;
using namespace dev;
using namespace dev::eth;

int stateTest()
{
	cnote << "Testing State...";

	KeyPair me = sha3("Gav Wood");
	KeyPair myMiner = sha3("Gav's Miner");
//	KeyPair you = sha3("123");

	Defaults::setDBPath(boost::filesystem::temp_directory_path().string());

	OverlayDB stateDB = State::openDB();
	CanonBlockChain bc;
	State s(myMiner.address(), stateDB);

	cout << bc;

	// Sync up - this won't do much until we use the last state.
	s.sync(bc);

	cout << s;

	// Mine to get some ether!
	s.commitToMine(bc);
	while (!s.mine(100).completed) {}
	s.completeMine();
	bc.attemptImport(s.blockData(), stateDB);

	cout << bc;

	s.sync(bc);

	cout << s;

	// Inject a transaction to transfer funds from miner to me.
	bytes tx;
	{
		Transaction t(1000, 0, 0, me.address(), bytes(), s.transactionsFrom(myMiner.address()), myMiner.secret());
		assert(t.sender() == myMiner.address());
		tx = t.rlp();
	}
	s.execute(bc, tx);

	cout << s;

	// Mine to get some ether and set in stone.
	s.commitToMine(bc);
	s.commitToMine(bc);
	while (!s.mine(50).completed) { s.commitToMine(bc); }
	s.completeMine();
	bc.attemptImport(s.blockData(), stateDB);

	cout << bc;

	s.sync(bc);

	cout << s;

	return 0;
}

