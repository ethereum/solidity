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
/** @file vm.h
 * @author Christoph Jentzsch <jentzsch.simulationsoftware@gmail.com>
 * @author Gav Wood <i@gavwood.com>
 * @date 2014
 * vm test functions.
 */

#pragma once

#include <fstream>
#include <cstdint>
#include <boost/test/unit_test.hpp>
#include "JsonSpiritHeaders.h"
#include <libdevcore/Log.h>
#include <libevmface/Instruction.h>
#include <libevm/ExtVMFace.h>
#include <libevm/VM.h>
#include <liblll/Compiler.h>
#include <libethereum/Transaction.h>
#include <libethereum/ExtVM.h>
#include <libethereum/State.h>

namespace dev { namespace test {

struct FakeExtVMFailure : virtual Exception {};

class FakeState: public eth::State
{
public:
	/// Execute a contract-creation transaction.
	h160 createNewAddress(Address _newAddress, Address _txSender, u256 _endowment, u256 _gasPrice, u256* _gas, bytesConstRef _code, Address _originAddress = Address(), std::set<Address>* o_suicides = nullptr, eth::Manifest* o_ms = nullptr, eth::OnOpFunc const& _onOp = eth::OnOpFunc(), unsigned _level = 0);
};

class FakeExtVM: public eth::ExtVMFace
{
public:
	FakeExtVM()	{}
	FakeExtVM(eth::BlockInfo const& _previousBlock, eth::BlockInfo const& _currentBlock, unsigned _depth = 0);

	u256 store(u256 _n) override { return std::get<2>(addresses[myAddress])[_n]; }
	void setStore(u256 _n, u256 _v) override { std::get<2>(addresses[myAddress])[_n] = _v; }
	u256 balance(Address _a) override { return std::get<0>(addresses[_a]); }
	void subBalance(u256 _a) override { std::get<0>(addresses[myAddress]) -= _a; }
	u256 txCount(Address _a) override { return std::get<1>(addresses[_a]); }
	void suicide(Address _a) override { std::get<0>(addresses[_a]) += std::get<0>(addresses[myAddress]); addresses.erase(myAddress); }
	bytes const& codeAt(Address _a) override { return std::get<3>(addresses[_a]); }
	h160 create(u256 _endowment, u256* _gas, bytesConstRef _init, eth::OnOpFunc const&) override;
	bool call(Address _receiveAddress, u256 _value, bytesConstRef _data, u256* _gas, bytesRef _out, eth::OnOpFunc const&, Address, Address) override;
	void setTransaction(Address _caller, u256 _value, u256 _gasPrice, bytes const& _data);
	void setContract(Address _myAddress, u256 _myBalance, u256 _myNonce, std::map<u256, u256> const& _storage, bytes const& _code);
	void set(Address _a, u256 _myBalance, u256 _myNonce, std::map<u256, u256> const& _storage, bytes const& _code);
	void reset(u256 _myBalance, u256 _myNonce, std::map<u256, u256> const& _storage);
	u256 toInt(json_spirit::mValue const& _v);
	byte toByte(json_spirit::mValue const& _v);
	void push(json_spirit::mObject& o, std::string const& _n, u256 _v);
	void push(json_spirit::mArray& a, u256 _v);
	u256 doPosts();
	json_spirit::mObject exportEnv();
	void importEnv(json_spirit::mObject& _o);
	json_spirit::mObject exportState();
	void importState(json_spirit::mObject& _object);
	json_spirit::mObject exportExec();
	void importExec(json_spirit::mObject& _o);
	json_spirit::mArray exportCallCreates();
	void importCallCreates(json_spirit::mArray& _callcreates);

	std::map<Address, std::tuple<u256, u256, std::map<u256, u256>, bytes>> addresses;
	eth::Transactions callcreates;
	bytes thisTxData;
	bytes thisTxCode;
	u256 gas;

private:
	FakeState m_s;
	eth::Manifest m_ms;
};

} } // Namespace Close
