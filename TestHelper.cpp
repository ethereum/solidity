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
#include <boost/filesystem/path.hpp>
#include <libethereum/Client.h>
#include <liblll/Compiler.h>
#include <libevm/VMFactory.h>

using namespace std;
using namespace dev::eth;

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
}

namespace test
{

struct ValueTooLarge: virtual Exception {};
bigint const c_max256plus1 = bigint(1) << 256;

ImportTest::ImportTest(json_spirit::mObject& _o, bool isFiller): m_TestObject(_o)
{
	importEnv(_o["env"].get_obj());
	importState(_o["pre"].get_obj(), m_statePre);
	importTransaction(_o["transaction"].get_obj());

	if (!isFiller)
	{
		importState(_o["post"].get_obj(), m_statePost);
		m_environment.sub.logs = importLog(_o["logs"].get_array());
	}
}

void ImportTest::importEnv(json_spirit::mObject& _o)
{
	BOOST_REQUIRE(_o.count("previousHash") > 0);
	BOOST_REQUIRE(_o.count("currentGasLimit") > 0);
	BOOST_REQUIRE(_o.count("currentDifficulty") > 0);
	BOOST_REQUIRE(_o.count("currentTimestamp") > 0);
	BOOST_REQUIRE(_o.count("currentCoinbase") > 0);
	BOOST_REQUIRE(_o.count("currentNumber") > 0);

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

		BOOST_REQUIRE(o.count("balance") > 0);
		BOOST_REQUIRE(o.count("nonce") > 0);
		BOOST_REQUIRE(o.count("storage") > 0);
		BOOST_REQUIRE(o.count("code") > 0);

		if (bigint(o["balance"].get_str()) >= c_max256plus1)
			BOOST_THROW_EXCEPTION(ValueTooLarge() << errinfo_comment("State 'balance' is equal or greater than 2**256") );
		if (bigint(o["nonce"].get_str()) >= c_max256plus1)
			BOOST_THROW_EXCEPTION(ValueTooLarge() << errinfo_comment("State 'nonce' is equal or greater than 2**256") );

		Address address = Address(i.first);

		bytes code = importCode(o);

		if (code.size())
		{
			_state.m_cache[address] = Account(toInt(o["balance"]), Account::ContractConception);
			_state.m_cache[address].setCode(code);
		}
		else
			_state.m_cache[address] = Account(toInt(o["balance"]), Account::NormalCreation);

		for (auto const& j: o["storage"].get_obj())
			_state.setStorage(address, toInt(j.first), toInt(j.second));

		for(int i=0; i<toInt(o["nonce"]); ++i)
			_state.noteSending(address);

		_state.ensureCached(address, false, false);
	}
}

void ImportTest::importTransaction(json_spirit::mObject& _o)
{
	BOOST_REQUIRE(_o.count("nonce")> 0);
	BOOST_REQUIRE(_o.count("gasPrice") > 0);
	BOOST_REQUIRE(_o.count("gasLimit") > 0);
	BOOST_REQUIRE(_o.count("to") > 0);
	BOOST_REQUIRE(_o.count("value") > 0);
	BOOST_REQUIRE(_o.count("secretKey") > 0);
	BOOST_REQUIRE(_o.count("data") > 0);

	if (bigint(_o["nonce"].get_str()) >= c_max256plus1)
		BOOST_THROW_EXCEPTION(ValueTooLarge() << errinfo_comment("Transaction 'nonce' is equal or greater than 2**256") );
	if (bigint(_o["gasPrice"].get_str()) >= c_max256plus1)
		BOOST_THROW_EXCEPTION(ValueTooLarge() << errinfo_comment("Transaction 'gasPrice' is equal or greater than 2**256") );
	if (bigint(_o["gasLimit"].get_str()) >= c_max256plus1)
		BOOST_THROW_EXCEPTION(ValueTooLarge() << errinfo_comment("Transaction 'gasLimit' is equal or greater than 2**256") );
	if (bigint(_o["value"].get_str()) >= c_max256plus1)
		BOOST_THROW_EXCEPTION(ValueTooLarge() << errinfo_comment("Transaction 'value' is equal or greater than 2**256") );

	m_transaction = _o["to"].get_str().empty() ?
		Transaction(toInt(_o["value"]), toInt(_o["gasPrice"]), toInt(_o["gasLimit"]), importData(_o), toInt(_o["nonce"]), Secret(_o["secretKey"].get_str())) :
		Transaction(toInt(_o["value"]), toInt(_o["gasPrice"]), toInt(_o["gasLimit"]), Address(_o["to"].get_str()), importData(_o), toInt(_o["nonce"]), Secret(_o["secretKey"].get_str()));
}

void ImportTest::exportTest(bytes _output, State& _statePost)
{
	// export output
	m_TestObject["out"] = "0x" + toHex(_output);

	// export logs
	m_TestObject["logs"] = exportLog(_statePost.pending().size() ? _statePost.log(0) : LogEntries());

	// export post state
	json_spirit::mObject postState;

	std::map<Address, Account> genesis = genesisState();

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

	// export pre state
	json_spirit::mObject preState;

	for (auto const& a: m_statePre.addresses())
	{
		if (genesis.count(a.first))
			continue;

		json_spirit::mObject o;
		o["balance"] = toString(m_statePre.balance(a.first));
		o["nonce"] = toString(m_statePre.transactionsFrom(a.first));
		{
			json_spirit::mObject store;
			for (auto const& s: m_statePre.storage(a.first))
				store["0x"+toHex(toCompactBigEndian(s.first))] = "0x"+toHex(toCompactBigEndian(s.second));
			o["storage"] = store;
		}
		o["code"] = "0x" + toHex(m_statePre.code(a.first));

		preState[toString(a.first)] = o;
	}
	m_TestObject["pre"] = json_spirit::mValue(preState);
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

bytes importByteArray(std::string const& _str)
{
	return fromHex(_str.substr(0, 2) == "0x" ? _str.substr(2) : _str, ThrowType::Throw);
}

bytes importData(json_spirit::mObject& _o)
{
	bytes data;
	if (_o["data"].type() == json_spirit::str_type)
		data = importByteArray(_o["data"].get_str());
	else
		for (auto const& j: _o["data"].get_array())
			data.push_back(toByte(j));
	return data;
}

bytes importCode(json_spirit::mObject& _o)
{
	bytes code;
	if (_o["code"].type() == json_spirit::str_type)
		if (_o["code"].get_str().find_first_of("0x") != 0)
			code = compileLLL(_o["code"].get_str(), false);
		else
			code = fromHex(_o["code"].get_str().substr(2));
	else if (_o["code"].type() == json_spirit::array_type)
	{
		code.clear();
		for (auto const& j: _o["code"].get_array())
			code.push_back(toByte(j));
	}
	return code;
}

LogEntries importLog(json_spirit::mArray& _a)
{
	LogEntries logEntries;
	for (auto const& l: _a)
	{
		json_spirit::mObject o = l.get_obj();
		// cant use BOOST_REQUIRE, because this function is used outside boost test (createRandomTest)
		assert(o.count("address") > 0);
		assert(o.count("topics") > 0);
		assert(o.count("data") > 0);
		assert(o.count("bloom") > 0);
		LogEntry log;
		log.address = Address(o["address"].get_str());
		for (auto const& t: o["topics"].get_array())
			log.topics.push_back(h256(t.get_str()));
		log.data = importData(o);
		logEntries.push_back(log);
	}
	return logEntries;
}

json_spirit::mArray exportLog(eth::LogEntries _logs)
{
	json_spirit::mArray ret;
	if (_logs.size() == 0) return ret;
	for (LogEntry const& l: _logs)
	{
		json_spirit::mObject o;
		o["address"] = toString(l.address);
		json_spirit::mArray topics;
		for (auto const& t: l.topics)
			topics.push_back(toString(t));
		o["topics"] = topics;
		o["data"] = "0x" + toHex(l.data);
		o["bloom"] = toString(l.bloom());
		ret.push_back(o);
	}
	return ret;
}

void checkOutput(bytes const& _output, json_spirit::mObject& _o)
{
	int j = 0;
	if (_o["out"].type() == json_spirit::array_type)
		for (auto const& d: _o["out"].get_array())
		{
			BOOST_CHECK_MESSAGE(_output[j] == toInt(d), "Output byte [" << j << "] different!");
			++j;
		}
	else if (_o["out"].get_str().find("0x") == 0)
		BOOST_CHECK(_output == fromHex(_o["out"].get_str().substr(2)));
	else
		BOOST_CHECK(_output == fromHex(_o["out"].get_str()));
}

void checkStorage(map<u256, u256> _expectedStore, map<u256, u256> _resultStore, Address _expectedAddr)
{
	for (auto&& expectedStorePair : _expectedStore)
	{
		auto& expectedStoreKey = expectedStorePair.first;
		auto resultStoreIt = _resultStore.find(expectedStoreKey);
		if (resultStoreIt == _resultStore.end())
			BOOST_ERROR(_expectedAddr << ": missing store key " << expectedStoreKey);
		else
		{
			auto& expectedStoreValue = expectedStorePair.second;
			auto& resultStoreValue = resultStoreIt->second;
			BOOST_CHECK_MESSAGE(expectedStoreValue == resultStoreValue, _expectedAddr << ": store[" << expectedStoreKey << "] = " << resultStoreValue << ", expected " << expectedStoreValue);
		}
	}
	BOOST_CHECK_EQUAL(_resultStore.size(), _expectedStore.size());
	for (auto&& resultStorePair: _resultStore)
	{
		if (!_expectedStore.count(resultStorePair.first))
			BOOST_ERROR(_expectedAddr << ": unexpected store key " << resultStorePair.first);
	}
}

void checkLog(LogEntries _resultLogs, LogEntries _expectedLogs)
{
	BOOST_REQUIRE_EQUAL(_resultLogs.size(), _expectedLogs.size());

	for (size_t i = 0; i < _resultLogs.size(); ++i)
	{
		BOOST_CHECK_EQUAL(_resultLogs[i].address, _expectedLogs[i].address);
		BOOST_CHECK_EQUAL(_resultLogs[i].topics, _expectedLogs[i].topics);
		BOOST_CHECK(_resultLogs[i].data == _expectedLogs[i].data);
	}
}

void checkCallCreates(eth::Transactions _resultCallCreates, eth::Transactions _expectedCallCreates)
{
	BOOST_REQUIRE_EQUAL(_resultCallCreates.size(), _expectedCallCreates.size());

	for (size_t i = 0; i < _resultCallCreates.size(); ++i)
	{
		BOOST_CHECK(_resultCallCreates[i].data() == _expectedCallCreates[i].data());
		BOOST_CHECK(_resultCallCreates[i].receiveAddress() == _expectedCallCreates[i].receiveAddress());
		BOOST_CHECK(_resultCallCreates[i].gas() == _expectedCallCreates[i].gas());
		BOOST_CHECK(_resultCallCreates[i].value() == _expectedCallCreates[i].value());
	}
}

std::string getTestPath()
{
	string testPath;
	const char* ptestPath = getenv("ETHEREUM_TEST_PATH");

	if (ptestPath == NULL)
	{
		cnote << " could not find environment variable ETHEREUM_TEST_PATH \n";
		testPath = "../../../tests";
	}
	else
		testPath = ptestPath;

	return testPath;
}

void userDefinedTest(string testTypeFlag, std::function<void(json_spirit::mValue&, bool)> doTests)
{
	for (int i = 1; i < boost::unit_test::framework::master_test_suite().argc; ++i)
	{
		string arg = boost::unit_test::framework::master_test_suite().argv[i];
		if (arg == testTypeFlag)
		{
			if (boost::unit_test::framework::master_test_suite().argc <= i + 2)
			{
				cnote << "Missing filename\nUsage: testeth " << testTypeFlag << " <filename> <testname>\n";
				return;
			}
			string filename = boost::unit_test::framework::master_test_suite().argv[i + 1];
			string testname = boost::unit_test::framework::master_test_suite().argv[i + 2];
			int currentVerbosity = g_logVerbosity;
			g_logVerbosity = 12;
			try
			{
				cnote << "Testing user defined test: " << filename;
				json_spirit::mValue v;
				string s = asString(contents(filename));
				BOOST_REQUIRE_MESSAGE(s.length() > 0, "Contents of " + filename + " is empty. ");
				json_spirit::read_string(s, v);
				json_spirit::mObject oSingleTest;

				json_spirit::mObject::const_iterator pos = v.get_obj().find(testname);
				if (pos == v.get_obj().end())
				{
					cnote << "Could not find test: " << testname << " in " << filename << "\n";
					return;
				}
				else
					oSingleTest[pos->first] = pos->second;

				json_spirit::mValue v_singleTest(oSingleTest);
				doTests(v_singleTest, false);
			}
			catch (Exception const& _e)
			{
				BOOST_ERROR("Failed Test with Exception: " << diagnostic_information(_e));
				g_logVerbosity = currentVerbosity;
			}
			catch (std::exception const& _e)
			{
				BOOST_ERROR("Failed Test with Exception: " << _e.what());
				g_logVerbosity = currentVerbosity;
			}
			g_logVerbosity = currentVerbosity;
		}
		else
			continue;
	}
}

void executeTests(const string& _name, const string& _testPathAppendix, std::function<void(json_spirit::mValue&, bool)> doTests)
{
	string testPath = getTestPath();
	testPath += _testPathAppendix;

	for (int i = 1; i < boost::unit_test::framework::master_test_suite().argc; ++i)
	{
		string arg = boost::unit_test::framework::master_test_suite().argv[i];
		if (arg == "--filltests")
		{
			try
			{
				cnote << "Populating tests...";
				json_spirit::mValue v;
				boost::filesystem::path p(__FILE__);
				boost::filesystem::path dir = p.parent_path();
				string s = asString(dev::contents(dir.string() + "/" + _name + "Filler.json"));
				BOOST_REQUIRE_MESSAGE(s.length() > 0, "Contents of " + dir.string() + "/" + _name + "Filler.json is empty.");
				json_spirit::read_string(s, v);
				doTests(v, true);
				writeFile(testPath + "/" + _name + ".json", asBytes(json_spirit::write_string(v, true)));
			}
			catch (Exception const& _e)
			{
				BOOST_ERROR("Failed test with Exception: " << diagnostic_information(_e));
			}
			catch (std::exception const& _e)
			{
				BOOST_ERROR("Failed test with Exception: " << _e.what());
			}
			break;
		}
	}

	try
	{
		cnote << "Testing ..." << _name;
		json_spirit::mValue v;
		string s = asString(dev::contents(testPath + "/" + _name + ".json"));
		BOOST_REQUIRE_MESSAGE(s.length() > 0, "Contents of " + testPath + "/" + _name + ".json is empty. Have you cloned the 'tests' repo branch develop and set ETHEREUM_TEST_PATH to its path?");
		json_spirit::read_string(s, v);
		doTests(v, false);
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


void processCommandLineOptions()
{
	auto argc = boost::unit_test::framework::master_test_suite().argc;
	auto argv = boost::unit_test::framework::master_test_suite().argv;

	for (auto i =  0; i < argc; ++i)
	{
		if (std::string(argv[i]) == "--jit")
		{
			eth::VMFactory::setKind(eth::VMKind::JIT);
			break;
		}
	}
}

LastHashes lastHashes(u256 _currentBlockNumber)
{
	LastHashes ret;
	for (u256 i = 1; i <= 256 && i <= _currentBlockNumber; ++i)
		ret.push_back(sha3(toString(_currentBlockNumber - i)));
	return ret;
}

} } // namespaces
