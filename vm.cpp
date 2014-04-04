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

#include <fstream>
#include <ExtVMFace.h>
#include <Transaction.h>
#include <VM.h>
#include <Log.h>
#include <Instruction.h>
#include "JsonSpiritHeaders.h"
using namespace std;
using namespace json_spirit;
using namespace eth;

namespace eth
{

class FakeExtVM: public ExtVMFace
{
public:
	FakeExtVM()
	{}
	FakeExtVM(BlockInfo const& _previousBlock, BlockInfo const& _currentBlock, uint _currentNumber):
		ExtVMFace(Address(), Address(), Address(), 0, 1, bytesConstRef(), bytesConstRef(), _previousBlock, _currentBlock, _currentNumber)
	{}

	u256 store(u256 _n)
	{
		return get<3>(addresses[myAddress])[_n];
	}
	void setStore(u256 _n, u256 _v)
	{
		get<3>(addresses[myAddress])[_n] = _v;
	}
	u256 balance(Address _a) { return get<0>(addresses[_a]); }
	void subBalance(u256 _a) { get<0>(addresses[myAddress]) -= _a; }
	u256 txCount(Address _a) { return get<1>(addresses[_a]); }
	void suicide(Address _a)
	{
		get<0>(addresses[_a]) += get<0>(addresses[myAddress]);
		addresses.erase(myAddress);
	}
	void transact(Transaction& _t)
	{
		if (get<0>(addresses[myAddress]) >= _t.value)
		{
			get<0>(addresses[myAddress]) -= _t.value;
			get<1>(addresses[myAddress])++;
//			get<0>(addresses[_t.receiveAddress]) += _t.value;
			txs.push_back(_t);
		}
	}
	h160 create(u256 _endowment, u256* _gas, bytesConstRef _code, bytesConstRef _init)
	{
		Transaction t;
		t.value = _endowment;
		t.gasPrice = gasPrice;
		t.gas = *_gas;
		t.data = _code.toBytes();
		t.init = _init.toBytes();
		txs.push_back(t);
		return right160(t.sha3(false));
	}

	bool call(Address _receiveAddress, u256 _value, bytesConstRef _data, u256* _gas, bytesRef _out)
	{
		Transaction t;
		t.value = _value;
		t.gasPrice = gasPrice;
		t.gas = *_gas;
		t.data = _data.toVector();
		t.receiveAddress = _receiveAddress;
		txs.push_back(t);
		(void)_out;
		return true;
	}

	void setTransaction(Address _caller, u256 _value, u256 _gasPrice, bytes const& _data)
	{
		caller = origin = _caller;
		value = _value;
		data = &_data;
		gasPrice = _gasPrice;
	}
	void setContract(Address _myAddress, u256 _myBalance, u256 _myNonce, bytes const& _code, map<u256, u256> const& _storage)
	{
		myAddress = _myAddress;
		set(myAddress, _myBalance, _myNonce, _code, _storage);
	}
	void set(Address _a, u256 _myBalance, u256 _myNonce, bytes const& _code, map<u256, u256> const& _storage)
	{
		get<0>(addresses[_a]) = _myBalance;
		get<1>(addresses[_a]) = _myNonce;
		get<2>(addresses[_a]) = 0;
		get<3>(addresses[_a]) = _storage;
		get<4>(addresses[_a]) = _code;
	}

	void reset(u256 _myBalance, u256 _myNonce, map<u256, u256> const& _storage)
	{
		txs.clear();
		addresses.clear();
		set(myAddress, _myBalance, _myNonce, get<4>(addresses[myAddress]), _storage);
	}

	mObject exportEnv()
	{
		mObject ret;
		ret["previousHash"] = toString(previousBlock.hash);
		ret["previousNonce"] = toString(previousBlock.nonce);
		push(ret, "currentDifficulty", currentBlock.difficulty);
		push(ret, "currentTimestamp", currentBlock.timestamp);
		ret["currentCoinbase"] = toString(currentBlock.coinbaseAddress);
		return ret;
	}

	void importEnv(mObject& _o)
	{
		previousBlock.hash = h256(_o["previousHash"].get_str());
		previousBlock.nonce = h256(_o["previousNonce"].get_str());
		currentBlock.difficulty = toInt(_o["currentDifficulty"]);
		currentBlock.timestamp = toInt(_o["currentTimestamp"]);
		currentBlock.coinbaseAddress = Address(_o["currentCoinbase"].get_str());
	}

	static u256 toInt(mValue const& _v)
	{
		switch (_v.type())
		{
		case str_type: return u256(_v.get_str());
		case int_type: return (u256)_v.get_uint64();
		case bool_type: return (u256)(uint64_t)_v.get_bool();
		case real_type: return (u256)(uint64_t)_v.get_real();
		default: cwarn << "Bad type for scalar: " << _v.type();
		}
		return 0;
	}

	static byte toByte(mValue const& _v)
	{
		switch (_v.type())
		{
		case str_type: return (byte)stoi(_v.get_str());
		case int_type: return (byte)_v.get_uint64();
		case bool_type: return (byte)_v.get_bool();
		case real_type: return (byte)_v.get_real();
		default: cwarn << "Bad type for scalar: " << _v.type();
		}
		return 0;
	}

	static void push(mObject& o, string const& _n, u256 _v)
	{
		if (_v < (u256)1 << 64)
			o[_n] = (uint64_t)_v;
		else
			o[_n] = toString(_v);
	}

	static void push(mArray& a, u256 _v)
	{
		if (_v < (u256)1 << 64)
			a.push_back((uint64_t)_v);
		else
			a.push_back(toString(_v));
	}

	mObject exportState()
	{
		mObject ret;
		for (auto const& a: addresses)
		{
			mObject o;
			push(o, "balance", get<0>(a.second));
			push(o, "nonce", get<1>(a.second));

			mObject store;
			string curKey;
			u256 li = 0;
			mArray curVal;
			for (auto const& s: get<3>(a.second))
			{
				if (!li || s.first > li + 8)
				{
					if (li)
						store[curKey] = curVal;
					li = s.first;
					curKey = toString(li);
					curVal = mArray();
				}
				else
					for (; li != s.first; ++li)
						curVal.push_back(0);
				push(curVal, s.second);
				++li;
			}
			if (li)
			{
				store[curKey] = curVal;
				o["store"] = store;
			}
			ret[toString(a.first)] = o;
		}
		return ret;
	}

	void importState(mObject& _o)
	{
		for (auto const& i: _o)
		{
			mObject o = i.second.get_obj();
			auto& a = addresses[Address(i.first)];
			get<0>(a) = toInt(o["balance"]);
			get<1>(a) = toInt(o["nonce"]);
			if (o.count("store"))
				for (auto const& j: o["store"].get_obj())
				{
					u256 adr(j.first);
					for (auto const& k: j.second.get_array())
						get<3>(a)[adr++] = toInt(k);
				}
			if (o.count("code"))
			{
				bytes e;
				bytes d = compileLisp(o["code"].get_str(), false, e);
				get<4>(a) = d;
			}
		}
	}

	mObject exportExec()
	{
		mObject ret;
		ret["address"] = toString(myAddress);
		ret["caller"] = toString(caller);
		ret["origin"] = toString(origin);
		push(ret, "value", value);
		push(ret, "gasPrice", gasPrice);
		mArray d;
		for (auto const& i: data)
			push(d, i);
		ret["data"] = d;
		return ret;
	}

	void importExec(mObject& _o)
	{
		myAddress = Address(_o["address"].get_str());
		caller = Address(_o["caller"].get_str());
		origin = Address(_o["origin"].get_str());
		value = toInt(_o["value"]);
		gasPrice = toInt(_o["gasPrice"]);
		thisTxData.clear();
		for (auto const& j: _o["data"].get_array())
			thisTxData.push_back(toByte(j));
		data = &thisTxData;
	}

	mArray exportTxs()
	{
		mArray ret;
		for (Transaction const& tx: txs)
		{
			mObject o;
			o["destination"] = toString(tx.receiveAddress);
			push(o, "value", tx.value);
			mArray d;
			for (auto const& i: tx.data)
				push(d, i);
			o["data"] = d;
			ret.push_back(o);
		}
		return ret;
	}

	void importTxs(mArray& _txs)
	{
		for (mValue& v: _txs)
		{
			auto tx = v.get_obj();
			Transaction t;
			t.receiveAddress = Address(tx["destination"].get_str());
			t.value = toInt(tx["value"]);
			for (auto const& j: tx["data"].get_array())
				t.data.push_back(toByte(j));
			txs.push_back(t);
		}
	}

	map<Address, tuple<u256, u256, u256, map<u256, u256>, bytes>> addresses;
	Transactions txs;
	bytes thisTxData;
};

#define CREATE_TESTS 0

template <> class UnitTest<1>
{
public:
	int operator()()
	{
		json_spirit::mValue v;
#if CREATE_TESTS
		string s = asString(contents("../../cpp-ethereum/test/vmtests.json"));
		json_spirit::read_string(s, v);
		bool passed = doTests(v, true);
		cout << json_spirit::write_string(v, true) << endl;
#else
		string s = asString(contents("../../tests/vmtests.json"));
		json_spirit::read_string(s, v);
		bool passed = doTests(v, false);
#endif
		return passed ? 0 : 1;
	}

	bool doTests(json_spirit::mValue& v, bool _fillin)
	{
		bool passed = true;
		for (auto& i: v.get_obj())
		{
			cnote << i.first;
			mObject& o = i.second.get_obj();

			VM vm;
			FakeExtVM fev;
			fev.importEnv(o["env"].get_obj());
			fev.importState(o["pre"].get_obj());

			if (_fillin)
				o["pre"] = mValue(fev.exportState());

			bytes output;
			for (auto i: o["exec"].get_array())
			{
				fev.importExec(i.get_obj());
				output = vm.go(fev).toBytes();
			}
			if (_fillin)
			{
				o["post"] = mValue(fev.exportState());
				o["txs"] = fev.exportTxs();
				mArray df;
				for (auto const& i: output)
					FakeExtVM::push(df, i);
				o["out"] = df;
			}
			else
			{
				FakeExtVM test;
				test.importState(o["post"].get_obj());
				test.importTxs(o["txs"].get_array());
				int i = 0;
				for (auto const& d: o["out"].get_array())
				{
					if (output[i] != FakeExtVM::toInt(d))
					{
						cwarn << "Test failed: output byte" << i << "different.";
						passed = false;
					}
					++i;
				}

				if (test.addresses != fev.addresses)
				{
					cwarn << "Test failed: state different.";
					passed = false;
				}
				if (test.txs != fev.txs)
				{
					cwarn << "Test failed: tx list different:";
					cwarn << test.txs;
					cwarn << fev.txs;
					passed = false;
				}
			}
		}
		return passed;
	}

	string makeTestCase()
	{
		json_spirit::mObject o;

		VM vm;
		BlockInfo pb;
		pb.hash = sha3("previousHash");
		pb.nonce = sha3("previousNonce");
		BlockInfo cb = pb;
		cb.difficulty = 256;
		cb.timestamp = 1;
		cb.coinbaseAddress = toAddress(sha3("coinbase"));
		FakeExtVM fev(pb, cb, 0);
		bytes init;
		fev.setContract(toAddress(sha3("contract")), ether, 0, compileLisp("(suicide (txsender))", false, init), map<u256, u256>());
		o["env"] = fev.exportEnv();
		o["pre"] = fev.exportState();
		fev.setTransaction(toAddress(sha3("sender")), ether, finney, bytes());
		mArray execs;
		execs.push_back(fev.exportExec());
		o["exec"] = execs;
		vm.go(fev);
		o["post"] = fev.exportState();
		o["txs"] = fev.exportTxs();

		return json_spirit::write_string(json_spirit::mValue(o), true);
	}
};

}

int vmTest()
{
	cnote << "Testing VM...";
	return UnitTest<1>()();
}

