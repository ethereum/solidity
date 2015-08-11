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
/** @file TestHelper.h
 * @author Marko Simovic <markobarko@gmail.com>
 * @date 2014
 */

#pragma once

#include <functional>

#include <boost/test/unit_test.hpp>
#include <boost/filesystem.hpp>

#include "JsonSpiritHeaders.h"
#include <libethcore/Ethash.h>
#include <libethereum/State.h>
#include <libevm/ExtVMFace.h>
#include <libtestutils/Common.h>


#ifdef NOBOOST
	#define TBOOST_REQUIRE(arg) if(arg == false) throw dev::Exception();
	#define TBOOST_REQUIRE_EQUAL(arg1, arg2) if(arg1 != arg2) throw dev::Exception();
	#define TBOOST_CHECK_EQUAL(arg1, arg2) if(arg1 != arg2) throw dev::Exception();
	#define TBOOST_CHECK(arg) if(arg == false) throw dev::Exception();
	#define TBOOST_REQUIRE_MESSAGE(arg1, arg2) if(arg1 == false) throw dev::Exception();
	#define TBOOST_CHECK_MESSAGE(arg1, arg2) if(arg1 == false) throw dev::Exception();
	#define TBOOST_WARN_MESSAGE(arg1, arg2) throw dev::Exception();
	#define TBOOST_ERROR(arg) throw dev::Exception();
#else
	#define TBOOST_REQUIRE(arg) BOOST_REQUIRE(arg)
	#define TBOOST_REQUIRE_EQUAL(arg1, arg2) BOOST_REQUIRE_EQUAL(arg1, arg2)
	#define TBOOST_CHECK(arg) BOOST_CHECK(arg)
	#define TBOOST_CHECK_EQUAL(arg1, arg2) BOOST_CHECK_EQUAL(arg1, arg2)
	#define TBOOST_CHECK_MESSAGE(arg1, arg2) BOOST_CHECK_MESSAGE(arg1, arg2)
	#define TBOOST_REQUIRE_MESSAGE(arg1, arg2) BOOST_REQUIRE_MESSAGE(arg1, arg2)
	#define TBOOST_WARN_MESSAGE(arg1, arg2) BOOST_WARN_MESSAGE(arg1, arg2)
	#define TBOOST_ERROR(arg) BOOST_ERROR(arg)
#endif

namespace dev
{
namespace eth
{

class Client;
class State;

void mine(Client& c, int numBlocks);
void connectClients(Client& c1, Client& c2);
void mine(Block& _s, BlockChain const& _bc);
void mine(Ethash::BlockHeader& _bi);

}

namespace test
{

/// Make sure that no Exception is thrown during testing. If one is thrown show its info and fail the test.
/// Our version of BOOST_REQUIRE_NO_THROW()
/// @param _statenent    The statement for which to make sure no exceptions are thrown
/// @param _message       A message to act as a prefix to the expression's error information
#define ETH_TEST_REQUIRE_NO_THROW(_statement, _message)				\
	do																	\
	{																	\
		try															\
		{																\
			BOOST_TEST_PASSPOINT();										\
			_statement;												\
		}																\
		catch (boost::exception const& _e)								\
		{																\
			auto msg = std::string(_message " due to an exception thrown by " \
				BOOST_STRINGIZE(_statement) "\n") + boost::diagnostic_information(_e); \
			BOOST_CHECK_IMPL(false, msg, REQUIRE, CHECK_MSG);			\
		}																\
		catch (...)														\
		{																\
			BOOST_CHECK_IMPL(false, "Unknown exception thrown by "		\
				BOOST_STRINGIZE(_statement), REQUIRE, CHECK_MSG);		\
		}																\
	}																	\
	while (0)

/// Check if an Exception is thrown during testing. If one is thrown show its info and continue the test
/// Our version of BOOST_CHECK_NO_THROW()
/// @param _statement    The statement for which to make sure no exceptions are thrown
/// @param _message       A message to act as a prefix to the expression's error information
#define ETH_TEST_CHECK_NO_THROW(_statement, _message)					\
	do																	\
	{																	\
		try															\
		{																\
			BOOST_TEST_PASSPOINT();										\
			_statement;												\
		}																\
		catch (boost::exception const& _e)								\
		{																\
			auto msg = std::string(_message " due to an exception thrown by " \
				BOOST_STRINGIZE(_statement) "\n") + boost::diagnostic_information(_e); \
			BOOST_CHECK_IMPL(false, msg, CHECK, CHECK_MSG);				\
		}																\
		catch (...)														\
		{																\
			BOOST_CHECK_IMPL(false, "Unknown exception thrown by "		\
				BOOST_STRINGIZE(_statement), CHECK, CHECK_MSG );		\
		}																\
	}																	\
	while (0)

enum class testType
{
	StateTests,
	BlockChainTests,
	Other
};

class ImportTest
{
public:
	ImportTest(json_spirit::mObject& _o, bool isFiller, testType testTemplate = testType::StateTests);

	// imports
	void importEnv(json_spirit::mObject& _o);
	static void importState(json_spirit::mObject& _o, eth::State& _state);
	static void importState(json_spirit::mObject& _o, eth::State& _state, eth::AccountMaskMap& o_mask);
	void importTransaction(json_spirit::mObject& _o);
	static json_spirit::mObject& makeAllFieldsHex(json_spirit::mObject& _o);

	bytes executeTest();
	void exportTest(bytes const& _output);
	static void compareStates(eth::State const& _stateExpect, eth::State const& _statePost, eth::AccountMaskMap const _expectedStateOptions = eth::AccountMaskMap(), WhenError _throw = WhenError::Throw);

	eth::State m_statePre;
	eth::State m_statePost;
	eth::EnvInfo m_envInfo;
	eth::Transaction m_transaction;	
	eth::LogEntries m_logs;
	eth::LogEntries m_logsExpected;

private:
	json_spirit::mObject& m_testObject;
};

class ZeroGasPricer: public eth::GasPricer
{
protected:
	u256 ask(eth::Block const&) const override { return 0; }
	u256 bid(eth::TransactionPriority = eth::TransactionPriority::Medium) const override { return 0; }
};

// helping functions
u256 toInt(json_spirit::mValue const& _v);
byte toByte(json_spirit::mValue const& _v);
bytes importCode(json_spirit::mObject& _o);
bytes importData(json_spirit::mObject& _o);
bytes importByteArray(std::string const& _str);
eth::LogEntries importLog(json_spirit::mArray& _o);
json_spirit::mArray exportLog(eth::LogEntries _logs);
void checkOutput(bytes const& _output, json_spirit::mObject& _o);
void checkStorage(std::map<u256, u256> _expectedStore, std::map<u256, u256> _resultStore, Address _expectedAddr);
void checkLog(eth::LogEntries _resultLogs, eth::LogEntries _expectedLogs);
void checkCallCreates(eth::Transactions _resultCallCreates, eth::Transactions _expectedCallCreates);
dev::eth::Ethash::BlockHeader constructHeader(
	h256 const& _parentHash,
	h256 const& _sha3Uncles,
	Address const& _coinbaseAddress,
	h256 const& _stateRoot,
	h256 const& _transactionsRoot,
	h256 const& _receiptsRoot,
	dev::eth::LogBloom const& _logBloom,
	u256 const& _difficulty,
	u256 const& _number,
	u256 const& _gasLimit,
	u256 const& _gasUsed,
	u256 const& _timestamp,
	bytes const& _extraData);
void updateEthashSeal(dev::eth::Ethash::BlockHeader& _header, h256 const& _mixHash, dev::eth::Nonce const& _nonce);
void executeTests(const std::string& _name, const std::string& _testPathAppendix, const boost::filesystem::path _pathToFiller, std::function<void(json_spirit::mValue&, bool)> doTests);
void userDefinedTest(std::function<void(json_spirit::mValue&, bool)> doTests);
RLPStream createRLPStreamFromTransactionFields(json_spirit::mObject& _tObj);
eth::LastHashes lastHashes(u256 _currentBlockNumber);
json_spirit::mObject fillJsonWithState(eth::State _state);
json_spirit::mObject fillJsonWithTransaction(eth::Transaction _txn);

//Fill Test Functions
void doTransactionTests(json_spirit::mValue& _v, bool _fillin);
void doStateTests(json_spirit::mValue& v, bool _fillin);
void doVMTests(json_spirit::mValue& v, bool _fillin);
void doBlockchainTests(json_spirit::mValue& _v, bool _fillin);
void doRlpTests(json_spirit::mValue& v, bool _fillin);

/*template<typename mapType>
void checkAddresses(mapType& _expectedAddrs, mapType& _resultAddrs)
{
	for (auto& resultPair : _resultAddrs)
	{
		auto& resultAddr = resultPair.first;
		auto expectedAddrIt = _expectedAddrs.find(resultAddr);
		if (expectedAddrIt == _expectedAddrs.end())
			TBOOST_ERROR("Missing result address " << resultAddr);
	}
	TBOOST_CHECK((_expectedAddrs == _resultAddrs));
}*/

class Options
{
public:
	bool vmtrace = false;	///< Create EVM execution tracer // TODO: Link with log verbosity?
	bool fillTests = false; ///< Create JSON test files from execution results
	bool stats = false;		///< Execution time stats
	std::string statsOutFile; ///< Stats output file. "out" for standard output
	bool checkState = false;///< Throw error when checking test states
	bool fulloutput = false;///< Replace large output to just it's length

	/// Test selection
	/// @{
	bool singleTest = false;
	std::string singleTestFile;
	std::string singleTestName;
	bool performance = false;
	bool quadratic = false;
	bool memory = false;
	bool inputLimits = false;
	bool bigData = false;
	bool wallet = false;
	bool nonetwork = true;
	bool nodag = true;
	/// @}

	/// Get reference to options
	/// The first time used, options are parsed
	static Options const& get();

private:
	Options();
	Options(Options const&) = delete;
};

/// Allows observing test execution process.
/// This class also provides methods for registering and notifying the listener
class Listener
{
public:
	virtual ~Listener() = default;

	virtual void suiteStarted(std::string const&) {}
	virtual void testStarted(std::string const& _name) = 0;
	virtual void testFinished() = 0;

	static void registerListener(Listener& _listener);
	static void notifySuiteStarted(std::string const& _name);
	static void notifyTestStarted(std::string const& _name);
	static void notifyTestFinished();

	/// Test started/finished notification RAII helper
	class ExecTimeGuard
	{
	public:
		ExecTimeGuard(std::string const& _testName) { notifyTestStarted(_testName);	}
		~ExecTimeGuard() { notifyTestFinished(); }
		ExecTimeGuard(ExecTimeGuard const&) = delete;
		ExecTimeGuard& operator=(ExecTimeGuard) = delete;
	};
};

}
}
