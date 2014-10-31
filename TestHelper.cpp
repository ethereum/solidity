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
/** @file TestHelper.cpp
 * @author Marko Simovic <markobarko@gmail.com>
 * @date 2014
 */

#include "TestHelper.h"

#include <thread>
#include <chrono>
#include <libethereum/Client.h>
#include <liblll/Compiler.h>

using namespace std;

namespace dev
{
namespace eth
{

void mine(Client& c, int numBlocks)
{
	auto startBlock = c.blockChain().details().number;

	c.startMining();
	while(c.blockChain().details().number < startBlock + numBlocks)
		std::this_thread::sleep_for(std::chrono::milliseconds(100));
	c.stopMining();
}

void connectClients(Client& c1, Client& c2)
{
	(void)c1;
	(void)c2;
	// TODO: Move to WebThree. eth::Client no longer handles networking.
#if 0
	short c1Port = 20000;
	short c2Port = 21000;
	c1.startNetwork(c1Port);
	c2.startNetwork(c2Port);
	c2.connect("127.0.0.1", c1Port);
#endif
}

namespace test
{

ImportTest::ImportTest(json_spirit::mObject& _o, bool isFiller):m_TestObject(_o)
{

	importEnv(_o["env"].get_obj());
	importState(_o["pre"].get_obj(), m_statePre);
	importExec(_o["exec"].get_obj());

	if (!isFiller)
	{
		importState(_o["post"].get_obj(), m_statePost);
		//importCallCreates(_o["callcreates"].get_array());
		importGas(_o);
		importOutput(_o);
	}
//	else
//		m_TestObject = &_o; // if Filler then change Test object to prepare for export
}

void ImportTest::importEnv(json_spirit::mObject& _o)
{
	assert(_o.count("previousHash") > 0);
	assert(_o.count("currentGasLimit") > 0);
	assert(_o.count("currentDifficulty") > 0);
	assert(_o.count("currentTimestamp") > 0);
	assert(_o.count("currentCoinbase") > 0);
	assert(_o.count("currentNumber") > 0);

	m_environment.previousBlock.hash = h256(_o["previousHash"].get_str());
	m_environment.currentBlock.number = toInt(_o["currentNumber"]);
	m_environment.currentBlock.gasLimit = toInt(_o["currentGasLimit"]);
	m_environment.currentBlock.difficulty = toInt(_o["currentDifficulty"]);
	m_environment.currentBlock.timestamp = toInt(_o["currentTimestamp"]);
	m_environment.currentBlock.coinbaseAddress = Address(_o["currentCoinbase"].get_str());

	m_statePre.m_previousBlock = m_environment.previousBlock;
	m_statePre.m_currentBlock = m_environment.currentBlock;
}

void ImportTest::importState(json_spirit::mObject& _o, State& _state)
{
	for (auto& i: _o)
	{
		json_spirit::mObject o = i.second.get_obj();

		assert(o.count("balance") > 0);
		assert(o.count("nonce") > 0);
		assert(o.count("storage") > 0);
		assert(o.count("code") > 0);

		Address address = Address(i.first);
		_state.m_cache[address] = AddressState(toInt(o["nonce"]), toInt(o["balance"]), EmptyTrie, h256());

		for (auto const& j: o["storage"].get_obj())
			_state.setStorage(address, toInt(j.first), toInt(j.second));

		bytes code;
		if (o["code"].type() == json_spirit::str_type)
			if (o["code"].get_str().find_first_of("0x") != 0)
				code = compileLLL(o["code"].get_str(), false);
			else
				code = fromHex(o["code"].get_str().substr(2));
		else
		{
			code.clear();
			for (auto const& j: o["code"].get_array())
				code.push_back(toByte(j));
		}

		i.second.get_obj()["code"] = "0x" + toHex(code); //preperation for export

		_state.m_cache[address].setCode(bytesConstRef(&code));
		_state.ensureCached(address, true, true);
	}
}

void ImportTest::importExec(json_spirit::mObject& _o)
{
	assert(_o.count("address")> 0);
	assert(_o.count("caller") > 0);
	assert(_o.count("origin") > 0);
	assert(_o.count("value") > 0);
	assert(_o.count("data") > 0);
	assert(_o.count("gasPrice") > 0);
	assert(_o.count("gas") > 0);
	//assert(_o.count("code") > 0);

	m_environment.myAddress = Address(_o["address"].get_str());
	m_environment.caller = Address(_o["caller"].get_str());
	m_environment.origin = Address(_o["origin"].get_str());
	m_environment.value = toInt(_o["value"]);
	m_environment.gasPrice = toInt(_o["gasPrice"]);
	gasExec = toInt(_o["gas"]);

	if (_o["code"].type() == json_spirit::str_type)
		if (_o["code"].get_str().find_first_of("0x") == 0)
			code = fromHex(_o["code"].get_str().substr(2));
		else
			code = compileLLL(_o["code"].get_str());
	else if (_o["code"].type() == json_spirit::array_type)
		for (auto const& j: _o["code"].get_array())
			code.push_back(toByte(j));
	else
		m_environment.code.reset();
	m_environment.code = &code;

	if (_o["data"].type() == json_spirit::str_type)
		if (_o["data"].get_str().find_first_of("0x") == 0)
			data = fromHex(_o["data"].get_str().substr(2));
		else
			data = fromHex(_o["data"].get_str());
	else
		for (auto const& j: _o["data"].get_array())
			data.push_back(toByte(j));
	m_environment.data = &data;
}

void ImportTest::importCallCreates(json_spirit::mArray& _callcreates)
{
	for (json_spirit::mValue& v: _callcreates)
	{
		auto tx = v.get_obj();
		assert(tx.count("data") > 0);
		assert(tx.count("value") > 0);
		assert(tx.count("destination") > 0);
		assert(tx.count("gasLimit") > 0);
		Transaction t;
		t.type = tx["destination"].get_str().empty() ? Transaction::ContractCreation : Transaction::MessageCall;
		t.receiveAddress = Address(tx["destination"].get_str());
		t.value = toInt(tx["value"]);
		t.gas = toInt(tx["gasLimit"]);
		if (tx["data"].type() == json_spirit::str_type)
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

void ImportTest::importGas(json_spirit::mObject& _o)
{
	gas = toInt(_o["gas"]);
}

void ImportTest::importOutput(json_spirit::mObject& _o)
{
	int i = 0;
	if (_o["out"].type() == json_spirit::array_type)
		for (auto const& d: _o["out"].get_array())
		{
			output[i] = uint8_t(toInt(d));
			++i;
		}
	else if (_o["out"].get_str().find("0x") == 0)
		output = fromHex(_o["out"].get_str().substr(2));
	else
		output = fromHex(_o["out"].get_str());
}

void ImportTest::exportTest(bytes _output, u256 _gas, State& _statePost)
{
	// export gas
	m_TestObject["gas"] = toString(_gas);

	// export output
	m_TestObject["out"] = "0x" + toHex(_output);

	// export post state
	json_spirit::mObject postState;

	std::map<Address, AddressState> genesis = genesisState();

	for (auto const& a: _statePost.addresses())
	{
		if (genesis.count(a.first))
			continue;

		json_spirit::mObject o;
		o["balance"] = toString(_statePost.balance(a.first));
		o["nonce"] = toString(_statePost.transactionsFrom(a.first));
		{
			json_spirit::mObject store;
			for (auto const& s: _statePost.storage(a.first))
				store["0x"+toHex(toCompactBigEndian(s.first))] = "0x"+toHex(toCompactBigEndian(s.second));
			o["storage"] = store;
		}
		o["code"] = "0x" + toHex(_statePost.code(a.first));

		postState[toString(a.first)] = o;
	}
	m_TestObject["post"] = json_spirit::mValue(postState);

	m_TestObject["exec"].get_obj()["code"] = "0x" + toHex(code);

//	// export callcreates
//	m_TestObject["callcreates"] = exportCallCreates();

//	for (int i = 0; i < (m_manifest.internal.size(); ++i)
//	{
//		 Transaction t;
//		 t.value = m_manifest.internal[i].value;
//		 t.gas = ;
//		 t.data = m_manifest.internal[i].input; ;
//		 t.receiveAddress = m_manifest.internal[i].to;
//		 t.type =

//	}
}

u256 toInt(json_spirit::mValue const& _v)
{
	switch (_v.type())
	{
	case json_spirit::str_type: return u256(_v.get_str());
	case json_spirit::int_type: return (u256)_v.get_uint64();
	case json_spirit::bool_type: return (u256)(uint64_t)_v.get_bool();
	case json_spirit::real_type: return (u256)(uint64_t)_v.get_real();
	default: cwarn << "Bad type for scalar: " << _v.type();
	}
	return 0;
}

byte toByte(json_spirit::mValue const& _v)
{
	switch (_v.type())
	{
	case json_spirit::str_type: return (byte)stoi(_v.get_str());
	case json_spirit::int_type: return (byte)_v.get_uint64();
	case json_spirit::bool_type: return (byte)_v.get_bool();
	case json_spirit::real_type: return (byte)_v.get_real();
	default: cwarn << "Bad type for scalar: " << _v.type();
	}
	return 0;
}


} } } // namespaces
