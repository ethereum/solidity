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
/** @file state.cpp
 * @author Gav Wood <i@gavwood.com>
 * @date 2014
 * State test functions.
 */

#include <ExtVMFace.h>
#include <Transaction.h>
#include <VM.h>
#include <Instruction.h>
using namespace std;
using namespace eth;

namespace eth
{

class FakeExtVM: public ExtVMFace
{
public:
	FakeExtVM(Address _myAddress, u256 _myBalance, u256 _myNonce, u256s _myData, Address _txSender, u256 _txValue, u256s const& _txData, FeeStructure const& _fees, BlockInfo const& _previousBlock, BlockInfo const& _currentBlock, uint _currentNumber):
		ExtVMFace(_myAddress, _txSender, _txValue, _txData, _fees, _previousBlock, _currentBlock, _currentNumber)
	{
		reset(_myBalance, _myNonce, _myData);
	}

	u256 store(u256 _n) { return get<3>(addresses[myAddress])[_n]; }
	void setStore(u256 _n, u256 _v) { get<3>(addresses[myAddress])[_n] = _v; }
	void mktx(Transaction& _t) { txs.push_back(_t); }
	u256 balance(Address _a) { return get<0>(addresses[_a]); }
	void payFee(bigint _fee) { get<0>(addresses[myAddress]) = (u256)(get<0>(addresses[myAddress]) - _fee); }
	u256 txCount(Address _a) { return get<1>(addresses[_a]); }
	u256 extro(Address _a, u256 _pos) { return get<3>(addresses[_a])[_pos]; }
	u256 extroPrice(Address _a) { return get<2>(addresses[_a]); }
	void suicide(Address _a) { dead = _a; }

	void reset(u256 _myBalance, u256 _myNonce, u256s _myData)
	{
		txs.clear();
		addresses.clear();
		get<0>(addresses[myAddress]) = _myBalance;
		get<1>(addresses[myAddress]) = _myNonce;
		get<2>(addresses[myAddress]) = 0;
		for (unsigned i = 0; i < _myData.size(); ++i)
			get<3>(addresses[myAddress])[i] = _myData[i];
		dead = Address();
	}

	map<Address, tuple<u256, u256, u256, map<u256, u256>>> addresses;
	Transactions txs;
	Address dead;
};

template <> class UnitTest<1>
{
public:
	int operator()()
	{
		VM vm;
		BlockInfo pb;
		pb.hash = sha3("previousHash");
		pb.nonce = sha3("previousNonce");
		BlockInfo cb = pb;
		cb.difficulty = 256;
		cb.timestamp = 1;
		cb.coinbaseAddress = toAddress(sha3("coinbase"));
		FeeStructure fees;
		fees.setMultiplier(1);

		string code = "(suicide (txsender))";

		FakeExtVM fev(toAddress(sha3("contract")), ether, 0, compileLisp(code), toAddress(sha3("sender")), ether, u256s(), fees, pb, cb, 0);

		vm.go(fev);
		cnote << fev.dead << formatBalance(fev.balance(toAddress(sha3("contract"))));

		return 0;
	}
};

}

int vmTest()
{
	return UnitTest<1>()();
}

