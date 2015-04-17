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

#include <boost/filesystem.hpp>

#include <libethereum/Executive.h>
#include <libevm/VMFactory.h>
#include "vm.h"

using namespace std;
using namespace json_spirit;
using namespace dev;
using namespace dev::eth;
using namespace dev::test;

FakeExtVM::FakeExtVM(eth::BlockInfo const& _previousBlock, eth::BlockInfo const& _currentBlock, unsigned _depth):			/// TODO: XXX: remove the default argument & fix.
	ExtVMFace(Address(), Address(), Address(), 0, 1, bytesConstRef(), bytes(), _previousBlock, _currentBlock, test::lastHashes(_currentBlock.number), _depth) {}

h160 FakeExtVM::create(u256 _endowment, u256& io_gas, bytesConstRef _init, OnOpFunc const&)
{
	Address na = right160(sha3(rlpList(myAddress, get<1>(addresses[myAddress]))));

	Transaction t(_endowment, gasPrice, io_gas, _init.toBytes());
	callcreates.push_back(t);
	return na;
}

bool FakeExtVM::call(Address _receiveAddress, u256 _value, bytesConstRef _data, u256& io_gas, bytesRef _out, OnOpFunc const&, Address _myAddressOverride, Address _codeAddressOverride)
{
	Transaction t(_value, gasPrice, io_gas, _receiveAddress, _data.toVector());
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
	o[_n] = "0x" + toHex(toCompactBigEndian(_v));
}

void FakeExtVM::push(mArray& a, u256 _v)
{
	a.push_back(toString(_v));
}

mObject FakeExtVM::exportEnv()
{
	mObject ret;
	ret["previousHash"] = toString(currentBlock.parentHash);
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

	currentBlock.parentHash = h256(_o["previousHash"].get_str());
	currentBlock.number = toInt(_o["currentNumber"]);
	lastHashes = test::lastHashes(currentBlock.number);
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
	code = thisTxCode;

	thisTxCode = importCode(_o);
	if (_o["code"].type() != str_type && _o["code"].type() != array_type)
		code.clear();

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
		assert(tx.count("data") > 0);
		assert(tx.count("value") > 0);
		assert(tx.count("destination") > 0);
		assert(tx.count("gasLimit") > 0);
		Transaction t = tx["destination"].get_str().empty() ?
			Transaction(toInt(tx["value"]), 0, toInt(tx["gasLimit"]), fromHex(tx["data"].get_str())) :
			Transaction(toInt(tx["value"]), 0, toInt(tx["gasLimit"]), Address(tx["destination"].get_str()), fromHex(tx["data"].get_str()));
		callcreates.push_back(t);
	}
}

eth::OnOpFunc FakeExtVM::simpleTrace()
{

	return [](uint64_t steps, eth::Instruction inst, bigint newMemSize, bigint gasCost, dev::eth::VM* voidVM, dev::eth::ExtVMFace const* voidExt)
	{
		FakeExtVM const& ext = *static_cast<FakeExtVM const*>(voidExt);
		eth::VM& vm = *voidVM;

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

		/*creates json stack trace*/
		if (eth::VMTraceChannel::verbosity <= g_logVerbosity)
		{
			Object o_step;

			/*add the stack*/
			Array a_stack;
			for (auto i: vm.stack())
				a_stack.push_back((string)i);

			o_step.push_back(Pair( "stack", a_stack ));

			/*add the memory*/
			Array a_mem;
			for(auto i: vm.memory())
				a_mem.push_back(i);

			o_step.push_back(Pair("memory", a_mem));

			/*add the storage*/
			Object storage;
			for (auto const& i: std::get<2>(ext.addresses.find(ext.myAddress)->second))
				storage.push_back(Pair( (string)i.first , (string)i.second));

			/*add all the other details*/
			o_step.push_back(Pair("storage", storage));
			o_step.push_back(Pair("depth", to_string(ext.depth)));
			o_step.push_back(Pair("gas", (string)vm.gas()));
			o_step.push_back(Pair("address", "0x" + toString(ext.myAddress )));
			o_step.push_back(Pair("step", steps ));
			o_step.push_back(Pair("pc", (int)vm.curPC()));
			o_step.push_back(Pair("opcode", instructionInfo(inst).name ));

			/*append the JSON object to the log file*/
			Value v(o_step);
			ofstream os( "./stackTrace.json", ofstream::app);
			os << write_string(v, true) << ",";
			os.close();
		}
	};
}

namespace dev { namespace test {

void doVMTests(json_spirit::mValue& v, bool _fillin)
{
	for (auto& i: v.get_obj())
	{
		std::cout << "  " << i.first << "\n";
		mObject& o = i.second.get_obj();

		BOOST_REQUIRE(o.count("env") > 0);
		BOOST_REQUIRE(o.count("pre") > 0);
		BOOST_REQUIRE(o.count("exec") > 0);

		FakeExtVM fev;
		fev.importEnv(o["env"].get_obj());
		fev.importState(o["pre"].get_obj());

		if (_fillin)
			o["pre"] = mValue(fev.exportState());

		fev.importExec(o["exec"].get_obj());
		if (fev.code.empty())
		{
			fev.thisTxCode = get<3>(fev.addresses.at(fev.myAddress));
			fev.code = fev.thisTxCode;
		}

		bytes output;
		u256 gas;
		bool vmExceptionOccured = false;
		try
		{
			auto vm = eth::VMFactory::create(fev.gas);
			auto vmtrace = Options::get().vmtrace ? fev.simpleTrace() : OnOpFunc{};
			auto outputRef = bytesConstRef{};
			{
				Listener::ExecTimeGuard guard{i.first};
				outputRef = vm->go(fev, vmtrace);
			}
			output = outputRef.toBytes();
			gas = vm->gas();
		}
		catch (VMException const&)
		{
			std::cout << "    Safe VM Exception\n";
			vmExceptionOccured = true;
		}
		catch (Exception const& _e)
		{
			cnote << "VM did throw an exception: " << diagnostic_information(_e);
			BOOST_ERROR("Failed VM Test with Exception: " << _e.what());
		}
		catch (std::exception const& _e)
		{
			cnote << "VM did throw an exception: " << _e.what();
			BOOST_ERROR("Failed VM Test with Exception: " << _e.what());
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
			if (!vmExceptionOccured)
			{
				o["post"] = mValue(fev.exportState());

				if (o.count("expect") > 0)
				{
					State postState(OverlayDB(), eth::BaseState::Empty);
					State expectState(OverlayDB(), eth::BaseState::Empty);
					stateOptionsMap expectStateMap;
					ImportTest::importState(o["post"].get_obj(), postState);
					ImportTest::importState(o["expect"].get_obj(), expectState, expectStateMap);
					ImportTest::checkExpectedState(expectState, postState, expectStateMap, Options::get().checkState ? WhenError::Throw : WhenError::DontThrow);
					o.erase(o.find("expect"));
				}

				o["callcreates"] = fev.exportCallCreates();
				o["out"] = "0x" + toHex(output);
				fev.push(o, "gas", gas);
				o["logs"] = exportLog(fev.sub.logs);
			}
		}
		else
		{
			if (o.count("post") > 0)	// No exceptions expected
			{
				BOOST_CHECK(!vmExceptionOccured);

				BOOST_REQUIRE(o.count("post") > 0);
				BOOST_REQUIRE(o.count("callcreates") > 0);
				BOOST_REQUIRE(o.count("out") > 0);
				BOOST_REQUIRE(o.count("gas") > 0);
				BOOST_REQUIRE(o.count("logs") > 0);

				dev::test::FakeExtVM test;
				test.importState(o["post"].get_obj());
				test.importCallCreates(o["callcreates"].get_array());
				test.sub.logs = importLog(o["logs"].get_array());

				checkOutput(output, o);

				BOOST_CHECK_EQUAL(toInt(o["gas"]), gas);

				State postState, expectState;
				mObject mPostState = fev.exportState();
				ImportTest::importState(mPostState, postState);
				ImportTest::importState(o["post"].get_obj(), expectState);
				ImportTest::checkExpectedState(expectState, postState);

				checkAddresses<std::map<Address, std::tuple<u256, u256, std::map<u256, u256>, bytes> > >(test.addresses, fev.addresses);

				checkCallCreates(fev.callcreates, test.callcreates);

				checkLog(fev.sub.logs, test.sub.logs);
			}
			else	// Exception expected
				BOOST_CHECK(vmExceptionOccured);
		}
	}
}

} } // Namespace Close

BOOST_AUTO_TEST_SUITE(VMTests)

BOOST_AUTO_TEST_CASE(vmtests)
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

BOOST_AUTO_TEST_CASE(vmLogTest)
{
	dev::test::executeTests("vmLogTest", "/VMTests", dev::test::doVMTests);
}

BOOST_AUTO_TEST_CASE(vmSystemOperationsTest)
{
	dev::test::executeTests("vmSystemOperationsTest", "/VMTests", dev::test::doVMTests);
}

BOOST_AUTO_TEST_CASE(vmPerformanceTest)
{
	if (test::Options::get().performance)
		dev::test::executeTests("vmPerformanceTest", "/VMTests", dev::test::doVMTests);
}

BOOST_AUTO_TEST_CASE(vmInputLimitsTest1)
{
	if (test::Options::get().inputLimits)
		dev::test::executeTests("vmInputLimits1", "/VMTests", dev::test::doVMTests);
}

BOOST_AUTO_TEST_CASE(vmInputLimitsTest2)
{
	if (test::Options::get().inputLimits)
		dev::test::executeTests("vmInputLimits2", "/VMTests", dev::test::doVMTests);
}

BOOST_AUTO_TEST_CASE(vmInputLimitsLightTest)
{
	if (test::Options::get().inputLimits)
		dev::test::executeTests("vmInputLimitsLight", "/VMTests", dev::test::doVMTests);
}

BOOST_AUTO_TEST_CASE(vmRandom)
{
	test::Options::get(); // parse command line options, e.g. to enable JIT

	string testPath = getTestPath();
	testPath += "/VMTests/RandomTests";

	vector<boost::filesystem::path> testFiles;
	boost::filesystem::directory_iterator iterator(testPath);
	for(; iterator != boost::filesystem::directory_iterator(); ++iterator)
		if (boost::filesystem::is_regular_file(iterator->path()) && iterator->path().extension() == ".json")
			testFiles.push_back(iterator->path());

	for (auto& path: testFiles)
	{
		try
		{
			std::cout << "TEST " << path.filename() << "\n";
			json_spirit::mValue v;
			string s = asString(dev::contents(path.string()));
			BOOST_REQUIRE_MESSAGE(s.length() > 0, "Content of " + path.string() + " is empty. Have you cloned the 'tests' repo branch develop and set ETHEREUM_TEST_PATH to its path?");
			json_spirit::read_string(s, v);
			test::Listener::notifySuiteStarted(path.filename().string());
			doVMTests(v, false);
		}
		catch (Exception const& _e)
		{
			BOOST_ERROR("Failed test with Exception: " << diagnostic_information(_e));
		}
		catch (std::exception const& _e)
		{
			BOOST_ERROR("Failed test with Exception: " << _e.what());
		}
	}
}

BOOST_AUTO_TEST_CASE(userDefinedFile)
{
	dev::test::userDefinedTest("--singletest", dev::test::doVMTests);
}

BOOST_AUTO_TEST_SUITE_END()
