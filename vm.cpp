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

using namespace std;
using namespace json_spirit;
using namespace dev;
using namespace dev::eth;
using namespace dev::test;

FakeExtVM::FakeExtVM(eth::BlockInfo const& _previousBlock, eth::BlockInfo const& _currentBlock, unsigned _depth):			/// TODO: XXX: remove the default argument & fix.
	ExtVMFace(Address(), Address(), Address(), 0, 1, bytesConstRef(), bytesConstRef(), _previousBlock, _currentBlock, _depth) {}

h160 FakeExtVM::create(u256 _endowment, u256* _gas, bytesConstRef _init, OnOpFunc const&)
{
	Address na = right160(sha3(rlpList(myAddress, get<1>(addresses[myAddress]))));

	Transaction t(_endowment, gasPrice, *_gas, _init.toBytes());
	callcreates.push_back(t);
	return na;
}

bool FakeExtVM::call(Address _receiveAddress, u256 _value, bytesConstRef _data, u256* _gas, bytesRef _out, OnOpFunc const&, Address _myAddressOverride, Address _codeAddressOverride)
{
	Transaction t(_value, gasPrice, *_gas, _receiveAddress, _data.toVector());
	callcreates.push_back(t);
	(void)_out;
	(void)_myAddressOverride;
	(void)_codeAddressOverride;
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

void FakeExtVM::push(mObject& o, string const& _n, u256 _v)
{
	o[_n] = toString(_v);
}

void FakeExtVM::push(mArray& a, u256 _v)
{
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

		get<3>(a) = importCode(o);
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

	thisTxCode = importCode(_o);
	if (_o["code"].type() != str_type && _o["code"].type() != array_type)
		code.reset();

	thisTxData.clear();
	thisTxData = importData(_o);

	data = &thisTxData;
}

mArray FakeExtVM::exportCallCreates()
{
	mArray ret;
	for (Transaction const& tx: callcreates)
	{
		mObject o;
		o["destination"] = tx.isCreation() ? "" : toString(tx.receiveAddress());
		push(o, "gasLimit", tx.gas());
		push(o, "value", tx.value());
		o["data"] = "0x" + toHex(tx.data());
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
		Transaction t = tx["destination"].get_str().empty() ?
			Transaction(toInt(tx["value"]), 0, toInt(tx["gasLimit"]), data.toBytes()) :
			Transaction(toInt(tx["value"]), 0, toInt(tx["gasLimit"]), Address(tx["destination"].get_str()), data.toBytes());
		callcreates.push_back(t);
	}
}

eth::OnOpFunc FakeExtVM::simpleTrace()
{
	return [](uint64_t steps, eth::Instruction inst, bigint newMemSize, bigint gasCost, void* voidVM, void const* voidExt)
	{
		FakeExtVM const& ext = *(FakeExtVM const*)voidExt;
		eth::VM& vm = *(eth::VM*)voidVM;

		std::ostringstream o;
		o << std::endl << "    STACK" << std::endl;
		for (auto i: vm.stack())
			o << (h256)i << std::endl;
		o << "    MEMORY" << std::endl << memDump(vm.memory());
		o << "    STORAGE" << std::endl;

		for (auto const& i: std::get<2>(ext.addresses.find(ext.myAddress)->second))
			o << std::showbase << std::hex << i.first << ": " << i.second << std::endl;

		dev::LogOutputStream<eth::VMTraceChannel, false>(true) << o.str();
		dev::LogOutputStream<eth::VMTraceChannel, false>(false) << " | " << std::dec << ext.depth << " | " << ext.myAddress << " | #" << steps << " | " << std::hex << std::setw(4) << std::setfill('0') << vm.curPC() << " : " << instructionInfo(inst).name << " | " << std::dec << vm.gas() << " | -" << std::dec << gasCost << " | " << newMemSize << "x32" << " ]";

		if (eth::VMTraceChannel::verbosity <= g_logVerbosity)
		{
			std::ofstream f;
			f.open("./vmtrace.log", std::ofstream::app);
			f << o.str();
			f << " | " << std::dec << ext.depth << " | " << ext.myAddress << " | #" << steps << " | " << std::hex << std::setw(4) << std::setfill('0') << vm.curPC() << " : " << instructionInfo(inst).name << " | " << std::dec << vm.gas() << " | -" << std::dec << gasCost << " | " << newMemSize << "x32";
		}
	};
}

namespace dev { namespace test {

void doVMTests(json_spirit::mValue& v, bool _fillin)
{
	for (auto& i: v.get_obj())
	{
		cnote << i.first;
		mObject& o = i.second.get_obj();

		BOOST_REQUIRE(o.count("env") > 0);
		BOOST_REQUIRE(o.count("pre") > 0);
		BOOST_REQUIRE(o.count("exec") > 0);

		dev::test::FakeExtVM fev;
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

		bytes output;
		VM vm(fev.gas);
		try
		{
			output = vm.go(fev, fev.simpleTrace()).toVector();
		}
		catch (Exception const& _e)
		{
			cnote << "VM did throw an exception: " << diagnostic_information(_e);
			//BOOST_ERROR("Failed VM Test with Exception: " << e.what());
		}
		catch (std::exception const& _e)
		{
			cnote << "VM did throw an exception: " << _e.what();
			//BOOST_ERROR("Failed VM Test with Exception: " << e.what());
		}

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
			fev.push(o, "gas", vm.gas());
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

			checkOutput(output, o);

			BOOST_CHECK_EQUAL(toInt(o["gas"]), vm.gas());

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

					checkStorage(std::get<2>(expectedState), std::get<2>(resultState), expectedAddr);
				}
			}

			checkAddresses<std::map<Address, std::tuple<u256, u256, std::map<u256, u256>, bytes> > >(test.addresses, fev.addresses);
			BOOST_CHECK(test.callcreates == fev.callcreates);
		}
	}
}

} } // Namespace Close

BOOST_AUTO_TEST_SUITE(VMTests)

BOOST_AUTO_TEST_CASE(vm_tests)
{
	dev::test::executeTests("vmtests", "/VMTests", dev::test::doVMTests);
}

BOOST_AUTO_TEST_CASE(vmArithmeticTest)
{
	dev::test::executeTests("vmArithmeticTest", "/VMTests", dev::test::doVMTests);
}

BOOST_AUTO_TEST_CASE(vmBitwiseLogicOperationTest)
{
	dev::test::executeTests("vmBitwiseLogicOperationTest", "/VMTests", dev::test::doVMTests);
}

BOOST_AUTO_TEST_CASE(vmSha3Test)
{
	dev::test::executeTests("vmSha3Test", "/VMTests", dev::test::doVMTests);
}

BOOST_AUTO_TEST_CASE(vmEnvironmentalInfoTest)
{
	dev::test::executeTests("vmEnvironmentalInfoTest", "/VMTests", dev::test::doVMTests);
}

BOOST_AUTO_TEST_CASE(vmBlockInfoTest)
{
	dev::test::executeTests("vmBlockInfoTest", "/VMTests", dev::test::doVMTests);
}

BOOST_AUTO_TEST_CASE(vmIOandFlowOperationsTest)
{
	dev::test::executeTests("vmIOandFlowOperationsTest", "/VMTests", dev::test::doVMTests);
}

BOOST_AUTO_TEST_CASE(vmPushDupSwapTest)
{
	dev::test::executeTests("vmPushDupSwapTest", "/VMTests", dev::test::doVMTests);
}

BOOST_AUTO_TEST_CASE(userDefinedFile)
{
	if (boost::unit_test::framework::master_test_suite().argc == 2)
	{
		string filename = boost::unit_test::framework::master_test_suite().argv[1];
		int currentVerbosity = g_logVerbosity;
		g_logVerbosity = 12;
		try
		{
			cnote << "Testing VM..." << "user defined test";
			json_spirit::mValue v;
			string s = asString(contents(filename));
			BOOST_REQUIRE_MESSAGE(s.length() > 0, "Contents of " + filename + " is empty. ");
			json_spirit::read_string(s, v);
			dev::test::doVMTests(v, false);
		}
		catch (Exception const& _e)
		{
			BOOST_ERROR("Failed VM Test with Exception: " << diagnostic_information(_e));
		}
		catch (std::exception const& _e)
		{
			BOOST_ERROR("Failed VM Test with Exception: " << _e.what());
		}
		g_logVerbosity = currentVerbosity;
	}
}

BOOST_AUTO_TEST_SUITE_END()
