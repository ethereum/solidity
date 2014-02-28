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
#include "../json_spirit/json_spirit_reader_template.h"
#include "../json_spirit/json_spirit_writer_template.h"
#include <ExtVMFace.h>
#include <Transaction.h>
#include <VM.h>
#include <Instruction.h>
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
	FakeExtVM(FeeStructure const& _fees, BlockInfo const& _previousBlock, BlockInfo const& _currentBlock, uint _currentNumber):
		ExtVMFace(Address(), Address(), 0, u256s(), _fees, _previousBlock, _currentBlock, _currentNumber)
	{}

	u256 store(u256 _n)
	{
#ifdef __clang__
		tuple<u256, u256, u256, map<u256, u256> > & address = addresses[myAddress];
		map<u256, u256> & third = get<3>(address);
		auto sFinder = third.find(_n);
		if (sFinder != third.end())
			return sFinder->second;
		else
			return 0;
#else
		return get<3>(addresses[myAddress])[_n];
#endif
	}
	void setStore(u256 _n, u256 _v)
	{
#ifdef __clang__
		tuple<u256, u256, u256, map<u256, u256> > & address = addresses[myAddress];
		map<u256, u256> & third = get<3>(address);
		auto sFinder = third.find(_n);
		if (sFinder != third.end())
			sFinder->second = _v;
		else
			third.insert(std::make_pair(_n, _v));
#else
		get<3>(addresses[myAddress])[_n] = _v;
#endif
	}
	void mktx(Transaction& _t)
	{
		if (get<0>(addresses[myAddress]) >= _t.value)
		{
			get<0>(addresses[myAddress]) -= _t.value;
			get<1>(addresses[myAddress])++;
//			get<0>(addresses[_t.receiveAddress]) += _t.value;
			txs.push_back(_t);
		}
	}
	u256 balance(Address _a) { return get<0>(addresses[_a]); }
	void payFee(bigint _fee) { get<0>(addresses[myAddress]) = (u256)(get<0>(addresses[myAddress]) - _fee); }
	u256 txCount(Address _a) { return get<1>(addresses[_a]); }
	u256 extro(Address _a, u256 _pos)
	{
#ifdef __clang__
		tuple<u256, u256, u256, map<u256, u256> > & address = addresses[_a];
		map<u256, u256> & third = get<3>(address);
		auto sFinder = third.find(_pos);
		if (sFinder != third.end())
			return sFinder->second;
		else
			return 0;
#else
		return get<3>(addresses[_a])[_pos];
#endif
	}
	u256 extroPrice(Address _a) { return get<2>(addresses[_a]); }
	void suicide(Address _a)
	{
		for (auto const& i: get<3>(addresses[myAddress]))
			if (i.second)
				get<0>(addresses[_a]) += fees.m_memoryFee;
		get<0>(addresses[_a]) += get<0>(addresses[myAddress]);
		addresses.erase(myAddress);
	}

	void setTransaction(Address _txSender, u256 _txValue, u256s const& _txData)
	{
		txSender = _txSender;
		txValue = _txValue;
		txData = _txData;
	}
	void setContract(Address _myAddress, u256 _myBalance, u256 _myNonce, u256s _myData)
	{
		myAddress = _myAddress;
		set(myAddress, _myBalance, _myNonce, _myData);
	}
	void set(Address _a, u256 _myBalance, u256 _myNonce, u256s _myData)
	{
		get<0>(addresses[_a]) = _myBalance;
		get<1>(addresses[_a]) = _myNonce;
		get<2>(addresses[_a]) = 0;
		for (unsigned i = 0; i < _myData.size(); ++i)
#ifdef __clang__
		{
			tuple<u256, u256, u256, map<u256, u256> > & address = addresses[_a];
			map<u256, u256> & third = get<3>(address);
			auto sFinder = third.find(i);
			if (sFinder != third.end())
				sFinder->second = _myData[i];
			else
				third.insert(std::make_pair(i, _myData[i]));
		}
#else
			get<3>(addresses[_a])[i] = _myData[i];
#endif
	}

	mObject exportEnv()
	{
		mObject ret;
		ret["previousHash"] = toString(previousBlock.hash);
		ret["previousNonce"] = toString(previousBlock.nonce);
		push(ret, "currentDifficulty", currentBlock.difficulty);
		push(ret, "currentTimestamp", currentBlock.timestamp);
		ret["currentCoinbase"] = toString(currentBlock.coinbaseAddress);
		push(ret, "feeMultiplier", fees.multiplier());
		return ret;
	}

	void importEnv(mObject& _o)
	{
		previousBlock.hash = h256(_o["previousHash"].get_str());
		previousBlock.nonce = h256(_o["previousNonce"].get_str());
		currentBlock.difficulty = toInt(_o["currentDifficulty"]);
		currentBlock.timestamp = toInt(_o["currentTimestamp"]);
		currentBlock.coinbaseAddress = Address(_o["currentCoinbase"].get_str());
		fees.setMultiplier(toInt(_o["feeMultiplier"]));
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
			push(o, "extroPrice", get<2>(a.second));

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
			get<2>(a) = toInt(o["extroPrice"]);
			if (o.count("store"))
				for (auto const& j: o["store"].get_obj())
				{
					u256 adr(j.first);
					for (auto const& k: j.second.get_array())
#ifdef __clang__
					{
						map<u256, u256> & third = get<3>(a);
						auto sFinder = third.find(adr);
						if (sFinder != third.end())
							sFinder->second = toInt(k);
						else
							third.insert(std::make_pair(adr, toInt(k)));
						adr++;
					}
#else
						get<3>(a)[adr++] = toInt(k);
#endif
				}
			if (o.count("code"))
			{
				u256s d = compileLisp(o["code"].get_str());
				for (unsigned i = 0; i < d.size(); ++i)
#ifdef __clang__
				{
					map<u256, u256> & third = get<3>(a);
					auto sFinder = third.find(i);
					if (sFinder != third.end())
						sFinder->second = d[i];
					else
						third.insert(std::make_pair(i, d[i]));
				}
#else
					get<3>(a)[(u256)i] = d[i];
#endif
			}
		}
	}

	mObject exportExec()
	{
		mObject ret;
		ret["address"] = toString(myAddress);
		ret["sender"] = toString(txSender);
		push(ret, "value", txValue);
		mArray d;
		for (auto const& i: txData)
			push(d, i);
		ret["data"] = d;
		return ret;
	}

	void importExec(mObject& _o)
	{
		myAddress = Address(_o["address"].get_str());
		txSender = Address(_o["sender"].get_str());
		txValue = toInt(_o["value"]);
		for (auto const& j: _o["data"].get_array())
			txData.push_back(toInt(j));
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
				t.data.push_back(toInt(j));
			txs.push_back(t);
		}
	}

	void reset(u256 _myBalance, u256 _myNonce, u256s _myData)
	{
		txs.clear();
		addresses.clear();
		set(myAddress, _myBalance, _myNonce, _myData);
	}

	map<Address, tuple<u256, u256, u256, map<u256, u256>>> addresses;
	Transactions txs;
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

			for (auto i: o["exec"].get_array())
			{
				fev.importExec(i.get_obj());
				vm.go(fev);
			}
			if (_fillin)
			{
				o["post"] = mValue(fev.exportState());
				o["txs"] = fev.exportTxs();
			}
			else
			{
				FakeExtVM test;
				test.importState(o["post"].get_obj());
				test.importTxs(o["txs"].get_array());
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
		FeeStructure fees;
		fees.setMultiplier(1);
		FakeExtVM fev(fees, pb, cb, 0);
		fev.setContract(toAddress(sha3("contract")), ether, 0, compileLisp("(suicide (txsender))"));
		o["env"] = fev.exportEnv();
		o["pre"] = fev.exportState();
		fev.setTransaction(toAddress(sha3("sender")), ether, u256s());
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

