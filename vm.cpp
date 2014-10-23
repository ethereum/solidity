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
/** @file vm.cpp
 * @author Gav Wood <i@gavwood.com>
 * @date 2014
 * vm test functions.
 */

#include "vm.h"
#include <libdevcore/CommonIO.h>
#include <libevmjit/VM.h>
#include <boost/filesystem/path.hpp>

//#define FILL_TESTS

using namespace std;
using namespace json_spirit;
using namespace dev;
using namespace dev::eth;
using namespace dev::test;

FakeExtVM::FakeExtVM(eth::BlockInfo const& _previousBlock, eth::BlockInfo const& _currentBlock, unsigned _depth):			/// TODO: XXX: remove the default argument & fix.
	ExtVMFace(Address(), Address(), Address(), 0, 1, bytesConstRef(), bytesConstRef(), _previousBlock, _currentBlock, _depth) {}

h160 FakeExtVM::create(u256 _endowment, u256* _gas, bytesConstRef _init, OnOpFunc const&)
{
	Transaction t;
	t.value = _endowment;
	t.gasPrice = gasPrice;
	t.gas = *_gas;
	t.data = _init.toBytes();

	m_s.noteSending(myAddress);
	m_ms.internal.resize(m_ms.internal.size() + 1);
	auto ret = m_s.create(myAddress, _endowment, gasPrice, _gas, _init, origin, &suicides, &m_ms ? &(m_ms.internal.back()) : nullptr, {}, 1);
	if (!m_ms.internal.back().from)
		m_ms.internal.pop_back();

	if (get<0>(addresses[myAddress]) >= _endowment)
	{
		get<1>(addresses[myAddress])++;
		get<0>(addresses[ret]) = _endowment;
		get<3>(addresses[ret]) = m_s.code(ret);
	}

	t.receiveAddress = ret;
	callcreates.push_back(t);
	return ret;
}

bool FakeExtVM::call(Address _receiveAddress, u256 _value, bytesConstRef _data, u256* _gas, bytesRef _out, OnOpFunc const&, Address _myAddressOverride = Address(), Address _codeAddressOverride = Address())
{

	u256 contractgas = 0xffff;

	Transaction t;
	t.value = _value;
	t.gasPrice = gasPrice;
	t.gas = *_gas;
	t.data = _data.toVector();
	t.receiveAddress = _receiveAddress;
	callcreates.push_back(t);

	string codeOf_CodeAddress = _codeAddressOverride ? toHex(get<3>(addresses[_codeAddressOverride])) : toHex(get<3>(addresses[_receiveAddress]) );
	string sizeOfCode = toHex(toCompactBigEndian((codeOf_CodeAddress.size()+1)/2));

	string codeOf_SenderAddress = toHex(get<3>(addresses[myAddress]) );
	string sizeOfSenderCode = toHex(toCompactBigEndian((codeOf_SenderAddress.size()+1)/2));

	if (codeOf_SenderAddress.size())
	{
		// create init code that returns given contract code
		string initStringHex = "{ (CODECOPY 0 (- (CODESIZE) 0x" + sizeOfSenderCode + "  ) 0x" + sizeOfSenderCode + ") (RETURN 0 0x" + sizeOfSenderCode +")}";
		bytes initBytes = compileLLL(initStringHex, true, NULL);
		initBytes += fromHex(codeOf_SenderAddress);
		bytesConstRef init(&initBytes);

		if (!m_s.addresses().count(myAddress))
		{
			m_ms.internal.resize(m_ms.internal.size() + 1);
			auto na = m_s.createNewAddress(myAddress, myAddress, balance(myAddress), gasPrice, &contractgas, init, origin, &suicides, &m_ms ? &(m_ms.internal.back()) : nullptr, {}, 1);
			if (!m_ms.internal.back().from)
				m_ms.internal.pop_back();
			if (na != myAddress)
			{
				cnote << "not able to call to : " << myAddress << "\n";
				cnote << "in FakeExtVM you can only make a call to " << na << "\n";
				BOOST_THROW_EXCEPTION(FakeExtVMFailure() << errinfo_comment("Address not callable in FakeExtVM\n") << errinfo_wrongAddress(toString(myAddress)));
				return false;
			}
		}
	}

	if (codeOf_CodeAddress.size())
	{
		// create init code that returns given contract code
		string initStringHex = "{ (CODECOPY 0 (- (CODESIZE) 0x" + sizeOfCode + "  ) 0x" + sizeOfCode + ") (RETURN 0 0x" + sizeOfCode +")}";
		bytes initBytes = compileLLL(initStringHex, true, NULL);
		initBytes += fromHex(codeOf_CodeAddress);
		bytesConstRef init(&initBytes);

		if (!m_s.addresses().count(_codeAddressOverride ? _codeAddressOverride : _receiveAddress))
		{
			m_s.noteSending(myAddress);
			m_ms.internal.resize(m_ms.internal.size() + 1);
			auto na = m_s.createNewAddress(_codeAddressOverride ? _codeAddressOverride : _receiveAddress, myAddress, balance(_codeAddressOverride ? _codeAddressOverride : _receiveAddress), gasPrice, &contractgas, init, origin, &suicides, &m_ms ? &(m_ms.internal.back()) : nullptr, OnOpFunc(), 1);
			if (!m_ms.internal.back().from)
				m_ms.internal.pop_back();

			if (na != (_codeAddressOverride ? _codeAddressOverride : _receiveAddress))
			{
				cnote << "not able to call to : " << (_codeAddressOverride ? _codeAddressOverride : _receiveAddress) << "\n";
				cnote << "in FakeExtVM you can only make a call to " << na << "\n";
				BOOST_THROW_EXCEPTION(FakeExtVMFailure() << errinfo_comment("Address not callable in FakeExtVM\n") << errinfo_wrongAddress(toString(_codeAddressOverride ? _codeAddressOverride : _receiveAddress)));
				return false;
			}
		}

		m_ms.internal.resize(m_ms.internal.size() + 1);

		auto ret = m_s.call(_receiveAddress,_codeAddressOverride ? _codeAddressOverride : _receiveAddress, _myAddressOverride ? _myAddressOverride : myAddress, _value, gasPrice, _data, _gas, _out, origin, &suicides, &(m_ms.internal.back()), OnOpFunc(), 1);

		if (!m_ms.internal.back().from)
			m_ms.internal.pop_back();

		// get correct balances, (also for sucicides in the call function)
		for (auto const& f: addresses)
		{
			if (m_s.addressInUse(f.first))
				get<0>(addresses[f.first]) = m_s.balance(f.first);
		}

		if (!ret)
			return false;

		// do suicides
		for (auto const& f: suicides)
			addresses.erase(f);

		// get storage
		if ((get<0>(addresses[myAddress]) >= _value) && (suicides.find(_receiveAddress) == suicides.end()))
		{
			for (auto const& j: m_s.storage(_receiveAddress))
			{
				u256 adr(j.first);
				if ((j.second != 0) )
					get<2>(addresses[_receiveAddress])[adr] = j.second;
			}
		}
	}
	else
		addresses.erase(_receiveAddress); // for the sake of comparison

	return true;
}

void FakeExtVM::setTransaction(Address _caller, u256 _value, u256 _gasPrice, bytes const& _data)
{
	caller = origin = _caller;
	value = _value;
	data = &(thisTxData = _data);
	gasPrice = _gasPrice;
}

void FakeExtVM::setContract(Address _myAddress, u256 _myBalance, u256 _myNonce, map<u256, u256> const& _storage, bytes const& _code)
{
	myAddress = _myAddress;
	set(myAddress, _myBalance, _myNonce, _storage, _code);
}

void FakeExtVM::set(Address _a, u256 _myBalance, u256 _myNonce, map<u256, u256> const& _storage, bytes const& _code)
{
	get<0>(addresses[_a]) = _myBalance;
	get<1>(addresses[_a]) = _myNonce;
	get<2>(addresses[_a]) = _storage;
	get<3>(addresses[_a]) = _code;
}

void FakeExtVM::reset(u256 _myBalance, u256 _myNonce, map<u256, u256> const& _storage)
{
	callcreates.clear();
	addresses.clear();
	set(myAddress, _myBalance, _myNonce, _storage, get<3>(addresses[myAddress]));
}

u256 FakeExtVM::toInt(mValue const& _v)
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

byte FakeExtVM::toByte(mValue const& _v)
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

void FakeExtVM::push(mObject& o, string const& _n, u256 _v)
{
	//		if (_v < (u256)1 << 64)
	//			o[_n] = (uint64_t)_v;
	//		else
	o[_n] = toString(_v);
}

void FakeExtVM::push(mArray& a, u256 _v)
{
	//		if (_v < (u256)1 << 64)
	//			a.push_back((uint64_t)_v);
	//		else
	a.push_back(toString(_v));
}

mObject FakeExtVM::exportEnv()
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

void FakeExtVM::importEnv(mObject& _o)
{
	// cant use BOOST_REQUIRE, because this function is used outside boost test (createRandomTest)
	assert(_o.count("previousHash") > 0);
	assert(_o.count("currentGasLimit") > 0);
	assert(_o.count("currentDifficulty") > 0);
	assert(_o.count("currentTimestamp") > 0);
	assert(_o.count("currentCoinbase") > 0);
	assert(_o.count("currentNumber") > 0);

	previousBlock.hash = h256(_o["previousHash"].get_str());
	currentBlock.number = toInt(_o["currentNumber"]);
	currentBlock.gasLimit = toInt(_o["currentGasLimit"]);
	currentBlock.difficulty = toInt(_o["currentDifficulty"]);
	currentBlock.timestamp = toInt(_o["currentTimestamp"]);
	currentBlock.coinbaseAddress = Address(_o["currentCoinbase"].get_str());
}

mObject FakeExtVM::exportState()
{
	mObject ret;
	for (auto const& a: addresses)
	{
		mObject o;
		push(o, "balance", get<0>(a.second));
		push(o, "nonce", get<1>(a.second));

		{
			mObject store;
			for (auto const& s: get<2>(a.second))
				store["0x"+toHex(toCompactBigEndian(s.first))] = "0x"+toHex(toCompactBigEndian(s.second));
			o["storage"] = store;
		}
		o["code"] = "0x" + toHex(get<3>(a.second));

		ret[toString(a.first)] = o;
	}
	return ret;
}

void FakeExtVM::importState(mObject& _object)
{
	for (auto const& i: _object)
	{
		mObject o = i.second.get_obj();
		// cant use BOOST_REQUIRE, because this function is used outside boost test (createRandomTest)
		assert(o.count("balance") > 0);
		assert(o.count("nonce") > 0);
		assert(o.count("storage") > 0);
		assert(o.count("code") > 0);

		auto& a = addresses[Address(i.first)];
		get<0>(a) = toInt(o["balance"]);
		get<1>(a) = toInt(o["nonce"]);
		for (auto const& j: o["storage"].get_obj())
			get<2>(a)[toInt(j.first)] = toInt(j.second);

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

mObject FakeExtVM::exportExec()
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

void FakeExtVM::importExec(mObject& _o)
{
	// cant use BOOST_REQUIRE, because this function is used outside boost test (createRandomTest)
	assert(_o.count("address")> 0);
	assert(_o.count("caller") > 0);
	assert(_o.count("origin") > 0);
	assert(_o.count("value") > 0);
	assert(_o.count("data") > 0);
	assert(_o.count("gasPrice") > 0);
	assert(_o.count("gas") > 0);

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

mArray FakeExtVM::exportCallCreates()
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

void FakeExtVM::importCallCreates(mArray& _callcreates)
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

h160 FakeState::createNewAddress(Address _newAddress, Address _sender, u256 _endowment, u256 _gasPrice, u256* _gas, bytesConstRef _code, Address _origin, std::set<Address>* o_suicides, Manifest* o_ms, OnOpFunc const& _onOp, unsigned _level)
{
	if (!_origin)
		_origin = _sender;

	if (o_ms)
	{
		o_ms->from = _sender;
		o_ms->to = Address();
		o_ms->value = _endowment;
		o_ms->input = _code.toBytes();
	}

	// Set up new account...
	m_cache[_newAddress] = AddressState(0, balance(_newAddress) + _endowment, h256(), h256());

	// Execute init code.
	auto vmObj = VMFace::create(getVMKind(), *_gas);
	auto& vm = *vmObj;
	ExtVM evm(*this, _newAddress, _sender, _origin, _endowment, _gasPrice, bytesConstRef(), _code, o_ms, _level);
	bool revert = false;
	bytesConstRef out;

	try
	{
		out = vm.go(evm, _onOp);
		if (o_ms)
			o_ms->output = out.toBytes();
		if (o_suicides)
			for (auto i: evm.suicides)
				o_suicides->insert(i);
	}
	catch (OutOfGas const& /*_e*/)
	{
		clog(StateChat) << "Out of Gas! Reverting.";
		revert = true;
	}
	catch (VMException const& _e)
	{
		clog(StateChat) << "VM Exception: " << diagnostic_information(_e);
	}
	catch (Exception const& _e)
	{
		clog(StateChat) << "Exception in VM: " << diagnostic_information(_e);
	}
	catch (std::exception const& _e)
	{
		clog(StateChat) << "std::exception in VM: " << _e.what();
	}

	// TODO: CHECK: IS THIS CORRECT?! (esp. given account created prior to revertion init.)

	// Write state out only in the case of a non-out-of-gas transaction.
	if (revert)
		evm.revert();

	// Set code.
	if (addressInUse(_newAddress))
		m_cache[_newAddress].setCode(out);

	*_gas = vm.gas();

	return _newAddress;
}



namespace dev { namespace test {

void doTests(json_spirit::mValue& v, bool _fillin)
{
	for (auto& i: v.get_obj())
	{
		cnote << i.first;
		mObject& o = i.second.get_obj();

		BOOST_REQUIRE(o.count("env") > 0);
		BOOST_REQUIRE(o.count("pre") > 0);
		BOOST_REQUIRE(o.count("exec") > 0);

		auto argc = boost::unit_test::framework::master_test_suite().argc;
		auto argv = boost::unit_test::framework::master_test_suite().argv;
		auto useJit = argc >= 2 && std::string(argv[1]) == "--jit";
		auto vmKind = useJit ? VMFace::JIT : VMFace::Interpreter;

		dev::test::FakeExtVM fev;
		fev.setVMKind(vmKind);

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
		
		auto vm = VMFace::create(fev.getVMKind(), fev.gas);
		bytes output;
		auto outOfGas = false;
		try
		{
			output = vm->go(fev).toVector();
		}
		catch (OutOfGas const&)
		{
			outOfGas = true;
		}
		catch (Exception const& _e)
		{
			cnote << "VM did throw an exception: " << diagnostic_information(_e);
		}
		catch (std::exception const& _e)
		{
			cnote << "VM did throw an exception: " << _e.what();
		}
		auto gas = vm->gas();

		// delete null entries in storage for the sake of comparison

		for (auto  &a: fev.addresses)
		{
			vector<u256> keystoDelete;
			for (auto &s: get<2>(a.second))
			{
				if (s.second == 0)
					keystoDelete.push_back(s.first);
			}
			for (auto const key: keystoDelete )
			{
				get<2>(a.second).erase(key);
			}
		}


		if (_fillin)
		{
			o["env"] = mValue(fev.exportEnv());
			o["exec"] = mValue(fev.exportExec());
			o["post"] = mValue(fev.exportState());
			o["callcreates"] = fev.exportCallCreates();
			o["out"] = "0x" + toHex(output);
			fev.push(o, "gas", gas);
		}
		else
		{
			BOOST_REQUIRE(o.count("post") > 0);
			BOOST_REQUIRE(o.count("callcreates") > 0);
			BOOST_REQUIRE(o.count("out") > 0);
			BOOST_REQUIRE(o.count("gas") > 0);

			dev::test::FakeExtVM test;
			test.importState(o["post"].get_obj());
			test.importCallCreates(o["callcreates"].get_array());
			int i = 0;
			if (o["out"].type() == array_type)
				for (auto const& d: o["out"].get_array())
				{
					BOOST_CHECK_MESSAGE(output[i] == test.toInt(d), "Output byte [" << i << "] different!");
					++i;
				}
			else if (o["out"].get_str().find("0x") == 0)
				BOOST_CHECK(output == fromHex(o["out"].get_str().substr(2)));
			else
				BOOST_CHECK(output == fromHex(o["out"].get_str()));

			BOOST_CHECK_EQUAL(test.toInt(o["gas"]), gas);
			
			if (outOfGas)
				BOOST_CHECK_MESSAGE(gas == 0, "Remaining gas not 0 in out-of-gas state");

			auto& expectedAddrs = test.addresses;
			auto& resultAddrs = fev.addresses;
			for (auto&& expectedPair : expectedAddrs)
			{
				auto& expectedAddr = expectedPair.first;
				auto resultAddrIt = resultAddrs.find(expectedAddr);
				if (resultAddrIt == resultAddrs.end())
					BOOST_ERROR("Missing expected address " << expectedAddr);
				else
				{
					auto& expectedState = expectedPair.second;
					auto& resultState = resultAddrIt->second;
					BOOST_CHECK_MESSAGE(std::get<0>(expectedState) == std::get<0>(resultState), expectedAddr << ": incorrect balance " << std::get<0>(resultState) << ", expected " << std::get<0>(expectedState));
					BOOST_CHECK_MESSAGE(std::get<1>(expectedState) == std::get<1>(resultState), expectedAddr << ": incorrect txCount " << std::get<1>(resultState) << ", expected " << std::get<1>(expectedState));
					BOOST_CHECK_MESSAGE(std::get<3>(expectedState) == std::get<3>(resultState), expectedAddr << ": incorrect code");

					auto&& expectedStore = std::get<2>(expectedState);
					auto&& resultStore = std::get<2>(resultState);

					for (auto&& expectedStorePair : expectedStore)
					{
						auto& expectedStoreKey = expectedStorePair.first;
						auto resultStoreIt = resultStore.find(expectedStoreKey);
						if (resultStoreIt == resultStore.end())
							BOOST_ERROR(expectedAddr << ": missing store key " << expectedStoreKey);
						else
						{
							auto& expectedStoreValue = expectedStorePair.second;
							auto& resultStoreValue = resultStoreIt->second;
							BOOST_CHECK_MESSAGE(expectedStoreValue == resultStoreValue, expectedAddr << ": store[" << expectedStoreKey << "] = " << resultStoreValue << ", expected " << expectedStoreValue);
						}
					}
				}
			}

			BOOST_CHECK(test.addresses == fev.addresses);	// Just to make sure nothing missed
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

void executeTests(const string& _name)
{
	const char* ptestPath = getenv("ETHEREUM_TEST_PATH");
	string testPath;

	if (ptestPath == NULL)
	{
		cnote << " could not find environment variable ETHEREUM_TEST_PATH \n";
		testPath = "../../../tests/vmtests";
	}
	else
		testPath = ptestPath;

#ifdef FILL_TESTS
	try
	{
		cnote << "Populating VM tests...";
		json_spirit::mValue v;
		boost::filesystem::path p(__FILE__);
		boost::filesystem::path dir = p.parent_path();
		string s = asString(contents(dir.string() + "/" + _name + "Filler.json"));
		BOOST_REQUIRE_MESSAGE(s.length() > 0, "Contents of " + _name + "Filler.json is empty.");
		json_spirit::read_string(s, v);
		dev::test::doTests(v, true);
		writeFile(testPath + "/" + _name + ".json", asBytes(json_spirit::write_string(v, true)));
	}
	catch (Exception const& _e)
	{
		BOOST_ERROR("Failed VM Test with Exception: " << diagnostic_information(_e));
	}
	catch (std::exception const& _e)
	{
		BOOST_ERROR("Failed VM Test with Exception: " << _e.what());
	}
#endif

	try
	{
		cnote << "Testing VM..." << _name;
		json_spirit::mValue v;
		string s = asString(contents(testPath + "/" + _name + ".json"));
		BOOST_REQUIRE_MESSAGE(s.length() > 0, "Contents of " + _name + ".json is empty. Have you cloned the 'tests' repo branch develop and set ETHEREUM_TEST_PATH to its path?");
		json_spirit::read_string(s, v);
		dev::test::doTests(v, false);
	}
	catch (Exception const& _e)
	{
		BOOST_ERROR("Failed VM Test with Exception: " << diagnostic_information(_e));
	}
	catch (std::exception const& _e)
	{
		BOOST_ERROR("Failed VM Test with Exception: " << _e.what());
	}

}

} } // Namespace Close

BOOST_AUTO_TEST_CASE(vm_tests)
{
	dev::test::executeTests("vmtests");
}

BOOST_AUTO_TEST_CASE(vmArithmeticTest)
{
	dev::test::executeTests("vmArithmeticTest");
}

BOOST_AUTO_TEST_CASE(vmBitwiseLogicOperationTest)
{
	dev::test::executeTests("vmBitwiseLogicOperationTest");
}

BOOST_AUTO_TEST_CASE(vmSha3Test)
{
	dev::test::executeTests("vmSha3Test");
}

BOOST_AUTO_TEST_CASE(vmEnvironmentalInfoTest)
{
	dev::test::executeTests("vmEnvironmentalInfoTest");
}

BOOST_AUTO_TEST_CASE(vmBlockInfoTest)
{
	dev::test::executeTests("vmBlockInfoTest");
}

BOOST_AUTO_TEST_CASE(vmIOandFlowOperationsTest)
{
	dev::test::executeTests("vmIOandFlowOperationsTest");
}

BOOST_AUTO_TEST_CASE(vmPushDupSwapTest)
{
	dev::test::executeTests("vmPushDupSwapTest");
}

BOOST_AUTO_TEST_CASE(vmSystemOperationsTest)
{
	dev::test::executeTests("vmSystemOperationsTest");
}

