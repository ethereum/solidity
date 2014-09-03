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
#include <cstdint>
#include <libethential/Log.h>
#include <libevmface/Instruction.h>
#include <libevm/ExtVMFace.h>
#include <libevm/VM.h>
#include <liblll/Compiler.h>
#include <libethereum/Transaction.h>
#include "JsonSpiritHeaders.h"
#include <boost/test/unit_test.hpp>

using namespace std;
using namespace json_spirit;
using namespace eth;

namespace eth { namespace test {

class FakeExtVM: public ExtVMFace
{
public:
	FakeExtVM()
	{}
	FakeExtVM(BlockInfo const& _previousBlock, BlockInfo const& _currentBlock):
		ExtVMFace(Address(), Address(), Address(), 0, 1, bytesConstRef(), bytesConstRef(), _previousBlock, _currentBlock)
	{}

	u256 store(u256 _n)
	{
		return get<2>(addresses[myAddress])[_n];
	}
	void setStore(u256 _n, u256 _v)
	{
		get<2>(addresses[myAddress])[_n] = _v;
	}
	u256 balance(Address _a) { return get<0>(addresses[_a]); }
	void subBalance(u256 _a) { get<0>(addresses[myAddress]) -= _a; }
	u256 txCount(Address _a) { return get<1>(addresses[_a]); }
	void suicide(Address _a)
	{
		get<0>(addresses[_a]) += get<0>(addresses[myAddress]);
		addresses.erase(myAddress);
	}
	h160 create(u256 _endowment, u256* _gas, bytesConstRef _init, OnOpFunc)
	{
		Address na = right160(sha3(rlpList(myAddress, get<1>(addresses[myAddress]))));
/*		if (get<0>(addresses[myAddress]) >= _endowment)
		{
			get<1>(addresses[myAddress])++;
			get<0>(addresses[na]) = _endowment;
			// TODO: actually execute...
		}*/
		Transaction t;
		t.value = _endowment;
		t.gasPrice = gasPrice;
		t.gas = *_gas;
		t.data = _init.toBytes();
		callcreates.push_back(t);
		return na;
	}

	bool call(Address _receiveAddress, u256 _value, bytesConstRef _data, u256* _gas, bytesRef _out, OnOpFunc, Address, Address)
	{
/*		if (get<0>(addresses[myAddress]) >= _value)
		{
			get<1>(addresses[myAddress])++;
			get<0>(addresses[_receiveAddress]) += _value;
			// TODO: actually execute...
		}*/
		Transaction t;
		t.value = _value;
		t.gasPrice = gasPrice;
		t.gas = *_gas;
		t.data = _data.toVector();
		t.receiveAddress = _receiveAddress;
		callcreates.push_back(t);
		(void)_out;
		return true;
	}

	void setTransaction(Address _caller, u256 _value, u256 _gasPrice, bytes const& _data)
	{
		caller = origin = _caller;
		value = _value;
		data = &(thisTxData = _data);
		gasPrice = _gasPrice;
	}
	void setContract(Address _myAddress, u256 _myBalance, u256 _myNonce, map<u256, u256> const& _storage, bytes const& _code)
	{
		myAddress = _myAddress;
		set(myAddress, _myBalance, _myNonce, _storage, _code);
	}
	void set(Address _a, u256 _myBalance, u256 _myNonce, map<u256, u256> const& _storage, bytes const& _code)
	{
		get<0>(addresses[_a]) = _myBalance;
		get<1>(addresses[_a]) = _myNonce;
		get<2>(addresses[_a]) = _storage;
		get<3>(addresses[_a]) = _code;
	}

	void reset(u256 _myBalance, u256 _myNonce, map<u256, u256> const& _storage)
	{
		callcreates.clear();
		addresses.clear();
		set(myAddress, _myBalance, _myNonce, _storage, get<3>(addresses[myAddress]));
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
//		if (_v < (u256)1 << 64)
//			o[_n] = (uint64_t)_v;
//		else
			o[_n] = toString(_v);
	}

	static void push(mArray& a, u256 _v)
	{
//		if (_v < (u256)1 << 64)
//			a.push_back((uint64_t)_v);
//		else
			a.push_back(toString(_v));
	}

	mObject exportEnv()
	{
		mObject ret;
		ret["previousHash"] = toString(previousBlock.hash);
		push(ret, "currentDifficulty", currentBlock.difficulty);
		push(ret, "currentTimestamp", currentBlock.timestamp);
		ret["currentCoinbase"] = toString(currentBlock.coinbaseAddress);
		push(ret, "currentNumber", currentBlock.number);
		push(ret, "currentGasLimit", currentBlock.gasLimit);
		return ret;
	}

	void importEnv(mObject& _o)
	{
		BOOST_REQUIRE(_o.count("previousHash") > 0);
		BOOST_REQUIRE(_o.count("currentGasLimit") > 0);
		BOOST_REQUIRE(_o.count("currentDifficulty") > 0);
		BOOST_REQUIRE(_o.count("currentTimestamp") > 0);
		BOOST_REQUIRE(_o.count("currentCoinbase") > 0);
		BOOST_REQUIRE(_o.count("currentNumber") > 0);

		previousBlock.hash = h256(_o["previousHash"].get_str());
		currentBlock.number = toInt(_o["currentNumber"]);
		currentBlock.gasLimit = toInt(_o["currentGasLimit"]);
		currentBlock.difficulty = toInt(_o["currentDifficulty"]);
		currentBlock.timestamp = toInt(_o["currentTimestamp"]);
		currentBlock.coinbaseAddress = Address(_o["currentCoinbase"].get_str());
	}

	mObject exportState()
	{
		mObject ret;
		for (auto const& a: addresses)
		{
			mObject o;
			push(o, "balance", get<0>(a.second));
			push(o, "nonce", get<1>(a.second));

			{
				mObject store;
				string curKey;
				u256 li = 0;
				mArray curVal;
				for (auto const& s: get<2>(a.second))
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
					store[curKey] = curVal;
				o["storage"] = store;
			}
			o["code"] = "0x" + toHex(get<3>(a.second));

			ret[toString(a.first)] = o;
		}
		return ret;
	}

	void importState(mObject& _object)
	{
		for (auto const& i: _object)
		{
			mObject o = i.second.get_obj();
			BOOST_REQUIRE(o.count("balance") > 0);
			BOOST_REQUIRE(o.count("nonce") > 0);
			BOOST_REQUIRE(o.count("storage") > 0);
			BOOST_REQUIRE(o.count("code") > 0);

			auto& a = addresses[Address(i.first)];
			get<0>(a) = toInt(o["balance"]);
			get<1>(a) = toInt(o["nonce"]);
			for (auto const& j: o["storage"].get_obj())
			{
				u256 adr(j.first);
				for (auto const& k: j.second.get_array())
					get<2>(a)[adr++] = toInt(k);
			}

			if (o["code"].type() == str_type)
				if (o["code"].get_str().find_first_of("0x") != 0)
					get<3>(a) = compileLLL(o["code"].get_str(), false);
				else
					get<3>(a) = fromHex(o["code"].get_str().substr(2));
			else
			{
				get<3>(a).clear();
				for (auto const& j: o["code"].get_array())
					get<3>(a).push_back(toByte(j));
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
		push(ret, "gas", gas);
		ret["data"] = "0x" + toHex(data);
		ret["code"] = "0x" + toHex(code);
		return ret;
	}

	void importExec(mObject& _o)
	{
		BOOST_REQUIRE(_o.count("address")> 0); 
		BOOST_REQUIRE(_o.count("caller") > 0);
		BOOST_REQUIRE(_o.count("origin") > 0); 
		BOOST_REQUIRE(_o.count("value") > 0); 
		BOOST_REQUIRE(_o.count("data") > 0);
		BOOST_REQUIRE(_o.count("gasPrice") > 0);
		BOOST_REQUIRE(_o.count("gas") > 0);

		myAddress = Address(_o["address"].get_str());
		caller = Address(_o["caller"].get_str());
		origin = Address(_o["origin"].get_str());
		value = toInt(_o["value"]);
		gasPrice = toInt(_o["gasPrice"]);
		gas = toInt(_o["gas"]);

		thisTxCode.clear();
		code = &thisTxCode;
		if (_o["code"].type() == str_type)
			if (_o["code"].get_str().find_first_of("0x") == 0)
				thisTxCode = compileLLL(_o["code"].get_str());
			else
				thisTxCode = fromHex(_o["code"].get_str().substr(2));
		else if (_o["code"].type() == array_type)
			for (auto const& j: _o["code"].get_array())
				thisTxCode.push_back(toByte(j));
		else
			code.reset();

		thisTxData.clear();
		if (_o["data"].type() == str_type)
			if (_o["data"].get_str().find_first_of("0x") == 0)
				thisTxData = fromHex(_o["data"].get_str().substr(2));
			else
				thisTxData = fromHex(_o["data"].get_str());
		else
			for (auto const& j: _o["data"].get_array())
				thisTxData.push_back(toByte(j));
		data = &thisTxData;
	}

	mArray exportCallCreates()
	{
		mArray ret;
		for (Transaction const& tx: callcreates)
		{
			mObject o;
			o["destination"] = toString(tx.receiveAddress);
			push(o, "gasLimit", tx.gas);
			push(o, "value", tx.value);
			o["data"] = "0x" + toHex(tx.data);
			ret.push_back(o);
		}
		return ret;
	}

	void importCallCreates(mArray& _callcreates)
	{
		for (mValue& v: _callcreates)
		{
			auto tx = v.get_obj();
			BOOST_REQUIRE(tx.count("data") > 0);
			BOOST_REQUIRE(tx.count("value") > 0);
			BOOST_REQUIRE(tx.count("destination") > 0);
			BOOST_REQUIRE(tx.count("gasLimit") > 0);
			Transaction t;
			t.receiveAddress = Address(tx["destination"].get_str());
			t.value = toInt(tx["value"]);
			t.gas = toInt(tx["gasLimit"]);
			if (tx["data"].type() == str_type)
				if (tx["data"].get_str().find_first_of("0x") == 0)
					t.data = fromHex(tx["data"].get_str().substr(2));
				else
					t.data = fromHex(tx["data"].get_str());
			else
				for (auto const& j: tx["data"].get_array())
					t.data.push_back(toByte(j));
			callcreates.push_back(t);
		}
	}

	map<Address, tuple<u256, u256, map<u256, u256>, bytes>> addresses;
	Transactions callcreates;
	bytes thisTxData;
	bytes thisTxCode;
	u256 gas;
};

void doTests(json_spirit::mValue& v, bool _fillin)
{
	for (auto& i: v.get_obj())
	{
		cnote << i.first;
		mObject& o = i.second.get_obj();

		BOOST_REQUIRE(o.count("env") > 0);
		BOOST_REQUIRE(o.count("pre") > 0);
		BOOST_REQUIRE(o.count("exec") > 0);

		VM vm;
		eth::test::FakeExtVM fev;
		fev.importEnv(o["env"].get_obj());
		fev.importState(o["pre"].get_obj());

		if (_fillin)
			o["pre"] = mValue(fev.exportState());

		fev.importExec(o["exec"].get_obj());
		if (!fev.code)
		{
			fev.thisTxCode = get<3>(fev.addresses.at(fev.myAddress));
			fev.code = &fev.thisTxCode;
		}
		vm.reset(fev.gas);
		bytes output = vm.go(fev).toBytes();

		if (_fillin)
		{
			o["env"] = mValue(fev.exportEnv());
			o["exec"] = mValue(fev.exportExec());
			o["post"] = mValue(fev.exportState());
			o["callcreates"] = fev.exportCallCreates();
			o["out"] = "0x" + toHex(output);
			fev.push(o, "gas", vm.gas());
		}
		else
		{
			BOOST_REQUIRE(o.count("post") > 0);
			BOOST_REQUIRE(o.count("callcreates") > 0);
			BOOST_REQUIRE(o.count("out") > 0);
			BOOST_REQUIRE(o.count("gas") > 0);

			eth::test::FakeExtVM test;
			test.importState(o["post"].get_obj());
			test.importCallCreates(o["callcreates"].get_array());
			int i = 0;
			if (o["out"].type() == array_type)
				for (auto const& d: o["out"].get_array())
				{
					BOOST_CHECK_MESSAGE(output[i] == FakeExtVM::toInt(d), "Output byte [" << i << "] different!");
					++i;
				}
			else if (o["out"].get_str().find("0x") == 0)
				BOOST_CHECK(output == fromHex(o["out"].get_str().substr(2)));
			else
				BOOST_CHECK(output == fromHex(o["out"].get_str()));

			BOOST_CHECK(FakeExtVM::toInt(o["gas"]) == vm.gas());
			BOOST_CHECK(test.addresses == fev.addresses);
			BOOST_CHECK(test.callcreates == fev.callcreates);
		}
	}
}

/*string makeTestCase()
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
}*/

} } // Namespace Close

BOOST_AUTO_TEST_CASE(vm_tests)
{
	// Populate tests first:
//	try
	{
		cnote << "Populating VM tests...";
		json_spirit::mValue v;
		string s = asString(contents("../../../cpp-ethereum/test/vmtests.json"));
		BOOST_REQUIRE_MESSAGE(s.length() > 0, "Contents of 'vmtests.json' is empty.");
		json_spirit::read_string(s, v);
		eth::test::doTests(v, true);
		writeFile("../../../tests/vmtests.json", asBytes(json_spirit::write_string(v, true)));
	}
/*	catch (std::exception const& e)
	{
		BOOST_ERROR("Failed VM Test with Exception: " << e.what());
	}*/

	try
	{
		cnote << "Testing VM...";
		json_spirit::mValue v;
		string s = asString(contents("../../../tests/vmtests.json"));
		BOOST_REQUIRE_MESSAGE(s.length() > 0, "Contents of 'vmtests.json' is empty. Have you cloned the 'tests' repo branch develop?");
		json_spirit::read_string(s, v);
		eth::test::doTests(v, false);
	}
	catch (std::exception const& e)
	{
		BOOST_ERROR("Failed VM Test with Exception: " << e.what()); 
	}
}
