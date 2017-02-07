/*
	This file is part of solidity.

	solidity is free software: you can redistribute it and/or modify
	it under the terms of the GNU General Public License as published by
	the Free Software Foundation, either version 3 of the License, or
	(at your option) any later version.

	solidity is distributed in the hope that it will be useful,
	but WITHOUT ANY WARRANTY; without even the implied warranty of
	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
	GNU General Public License for more details.

	You should have received a copy of the GNU General Public License
	along with solidity.  If not, see <http://www.gnu.org/licenses/>.

	The Implementation originally from https://msdn.microsoft.com/en-us/library/windows/desktop/aa365592(v=vs.85).aspx
*/
/** @file RPCSession.cpp
 * @author Dimtiry Khokhlov <dimitry@ethdev.com>
 * @date 2016
 */

#include <string>
#include <stdio.h>
#include <thread>
#include <libdevcore/CommonData.h>
#include <json/reader.h>
#include <json/writer.h>
#include "RPCSession.h"

using namespace std;
using namespace dev;

IPCSocket::IPCSocket(string const& _path): m_path(_path)
{
#if defined(_WIN32)
	m_socket = CreateFile(
		m_path.c_str(),   // pipe name
		GENERIC_READ |  // read and write access
		GENERIC_WRITE,
		0,              // no sharing
		NULL,           // default security attribute
		OPEN_EXISTING,  // opens existing pipe
		0,              // default attributes
		NULL);          // no template file

	if (m_socket == INVALID_HANDLE_VALUE)
		BOOST_FAIL("Error creating IPC socket object!");

#else
	if (_path.length() >= sizeof(sockaddr_un::sun_path))
		BOOST_FAIL("Error opening IPC: socket path is too long!");

	struct sockaddr_un saun;
	memset(&saun, 0, sizeof(sockaddr_un));
	saun.sun_family = AF_UNIX;
	strcpy(saun.sun_path, _path.c_str());

// http://idletechnology.blogspot.ca/2011/12/unix-domain-sockets-on-osx.html
//
// SUN_LEN() might be optimal, but it seemingly affects the portability,
// with at least Android missing this macro.  Just using the sizeof() for
// structure seemingly works, and would only have the side-effect of
// sending larger-than-required packets over the socket.  Given that this
// code is only used for unit-tests, that approach seems simpler.
#if defined(__APPLE__)
	saun.sun_len = sizeof(struct sockaddr_un);
#endif //  defined(__APPLE__)

	if ((m_socket = socket(AF_UNIX, SOCK_STREAM, 0)) < 0)
		BOOST_FAIL("Error creating IPC socket object");

	if (connect(m_socket, reinterpret_cast<struct sockaddr const*>(&saun), sizeof(struct sockaddr_un)) < 0)
		BOOST_FAIL("Error connecting to IPC socket: " << _path);

	m_fp = fdopen(m_socket, "r");
	if (!m_fp)
		BOOST_FAIL("Error opening IPC socket: " << _path);
#endif
}

string IPCSocket::sendRequest(string const& _req)
{
#if defined(_WIN32)
	string returnStr;
	DWORD cbWritten;
	BOOL fSuccess = WriteFile(
		m_socket,               // pipe handle
		_req.c_str(),           // message
		_req.size(),            // message length
		&cbWritten,             // bytes written
		NULL);                  // not overlapped

	if (!fSuccess)
		BOOST_FAIL("WriteFile to pipe failed");

	DWORD  cbRead;
	TCHAR  chBuf[c_buffsize];

	// Read from the pipe.
	fSuccess = ReadFile(
		m_socket,  // pipe handle
		chBuf,     // buffer to receive reply
		c_buffsize,// size of buffer
		&cbRead,   // number of bytes read
		NULL);     // not overlapped

	returnStr += chBuf;

	if (!fSuccess)
		BOOST_FAIL("ReadFile from pipe failed");

	cerr << ".";  //Output for log activity
	return returnStr;
#else
	send(m_socket, _req.c_str(), _req.length(), 0);

	char c;
	string response;
	while ((c = fgetc(m_fp)) != EOF)
	{
		if (c != '\n')
			response += c;
		else
			break;
	}
	return response;
#endif
}

RPCSession& RPCSession::instance(const string& _path)
{
	static RPCSession session(_path);
	BOOST_REQUIRE_EQUAL(session.m_ipcSocket.path(), _path);
	return session;
}

string RPCSession::eth_getCode(string const& _address, string const& _blockNumber)
{
	return rpcCall("eth_getCode", { quote(_address), quote(_blockNumber) }).asString();
}

RPCSession::TransactionReceipt RPCSession::eth_getTransactionReceipt(string const& _transactionHash)
{
	TransactionReceipt receipt;
	Json::Value const result = rpcCall("eth_getTransactionReceipt", { quote(_transactionHash) });
	BOOST_REQUIRE(!result.isNull());
	receipt.gasUsed = result["gasUsed"].asString();
	receipt.contractAddress = result["contractAddress"].asString();
	for (auto const& log: result["logs"])
	{
		LogEntry entry;
		entry.address = log["address"].asString();
		entry.data = log["data"].asString();
		for (auto const& topic: log["topics"])
			entry.topics.push_back(topic.asString());
		receipt.logEntries.push_back(entry);
	}
	return receipt;
}

string RPCSession::eth_sendTransaction(TransactionData const& _td)
{
	return rpcCall("eth_sendTransaction", { _td.toJson() }).asString();
}

string RPCSession::eth_call(TransactionData const& _td, string const& _blockNumber)
{
	return rpcCall("eth_call", { _td.toJson(), quote(_blockNumber) }).asString();
}

string RPCSession::eth_sendTransaction(string const& _transaction)
{
	return rpcCall("eth_sendTransaction", { _transaction }).asString();
}

string RPCSession::eth_getBalance(string const& _address, string const& _blockNumber)
{
	string address = (_address.length() == 20) ? "0x" + _address : _address;
	return rpcCall("eth_getBalance", { quote(address), quote(_blockNumber) }).asString();
}

string RPCSession::eth_getStorageRoot(string const& _address, string const& _blockNumber)
{
	string address = (_address.length() == 20) ? "0x" + _address : _address;
	return rpcCall("eth_getStorageRoot", { quote(address), quote(_blockNumber) }).asString();
}

void RPCSession::personal_unlockAccount(string const& _address, string const& _password, int _duration)
{
	rpcCall("personal_unlockAccount", { quote(_address), quote(_password), to_string(_duration) });
}

string RPCSession::personal_newAccount(string const& _password)
{
	return rpcCall("personal_newAccount", { quote(_password) }).asString();
}

void RPCSession::test_setChainParams(vector<string> const& _accounts)
{
	static std::string const c_configString = R"(
	{
		"sealEngine": "NoProof",
		"params": {
			"accountStartNonce": "0x",
			"maximumExtraDataSize": "0x1000000",
			"blockReward": "0x",
			"allowFutureBlocks": "1"
		},
		"genesis": {
			"author": "0000000000000010000000000000000000000000",
			"timestamp": "0x00",
			"parentHash": "0x0000000000000000000000000000000000000000000000000000000000000000",
			"extraData": "0x",
			"gasLimit": "0x1000000000000"
		},
		"accounts": {
			"0000000000000000000000000000000000000001": { "wei": "1", "precompiled": { "name": "ecrecover", "linear": { "base": 3000, "word": 0 } } },
			"0000000000000000000000000000000000000002": { "wei": "1", "precompiled": { "name": "sha256", "linear": { "base": 60, "word": 12 } } },
			"0000000000000000000000000000000000000003": { "wei": "1", "precompiled": { "name": "ripemd160", "linear": { "base": 600, "word": 120 } } },
			"0000000000000000000000000000000000000004": { "wei": "1", "precompiled": { "name": "identity", "linear": { "base": 15, "word": 3 } } }
		}
	}
	)";

	Json::Value config;
	BOOST_REQUIRE(Json::Reader().parse(c_configString, config));
	for (auto const& account: _accounts)
		config["accounts"][account]["wei"] = "0x100000000000000000000000000000000000000000";
	test_setChainParams(Json::FastWriter().write(config));
}

void RPCSession::test_setChainParams(string const& _config)
{
	rpcCall("test_setChainParams", { _config });
}

void RPCSession::test_rewindToBlock(size_t _blockNr)
{
	rpcCall("test_rewindToBlock", { to_string(_blockNr) });
}

void RPCSession::test_mineBlocks(int _number)
{
	u256 startBlock = fromBigEndian<u256>(fromHex(rpcCall("eth_blockNumber").asString()));
	rpcCall("test_mineBlocks", { to_string(_number) }, true);

	bool mined = false;

	// We auto-calibrate the time it takes to mine the transaction.
	// It would be better to go without polling, but that would probably need a change to the test client

	unsigned sleepTime = m_sleepTime;
	size_t polls = 0;
	for (; polls < 14 && !mined; ++polls)
	{
		std::this_thread::sleep_for(chrono::milliseconds(sleepTime));
		if (fromBigEndian<u256>(fromHex(rpcCall("eth_blockNumber").asString())) >= startBlock + _number)
			mined = true;
		else
			sleepTime *= 2;
	}
	if (polls > 1)
	{
		m_successfulMineRuns = 0;
		m_sleepTime += 2;
	}
	else if (polls == 1)
	{
		m_successfulMineRuns++;
		if (m_successfulMineRuns > 5)
		{
			m_successfulMineRuns = 0;
			if (m_sleepTime > 2)
				m_sleepTime--;
		}
	}

	if (!mined)
		BOOST_FAIL("Error in test_mineBlocks: block mining timeout!");
}

void RPCSession::test_modifyTimestamp(size_t _timestamp)
{
	rpcCall("test_modifyTimestamp", { to_string(_timestamp) });
}

Json::Value RPCSession::rpcCall(string const& _methodName, vector<string> const& _args, bool _canFail)
{
	string request = "{\"jsonrpc\":\"2.0\",\"method\":\"" + _methodName + "\",\"params\":[";
	for (size_t i = 0; i < _args.size(); ++i)
	{
		request += _args[i];
		if (i + 1 != _args.size())
			request += ", ";
	}

	request += "],\"id\":" + to_string(m_rpcSequence) + "}";
	++m_rpcSequence;

	//cout << "Request: " << request << endl;
	string reply = m_ipcSocket.sendRequest(request);
	//cout << "Reply: " << reply << endl;

	Json::Value result;
	BOOST_REQUIRE(Json::Reader().parse(reply, result, false));

	if (result.isMember("error"))
	{
		if (_canFail)
			return Json::Value();

		BOOST_FAIL("Error on JSON-RPC call: " + result["error"]["message"].asString());
	}
	return result["result"];
}

string const& RPCSession::accountCreateIfNotExists(size_t _id)
{
	if (_id >= m_accounts.size())
	{
		m_accounts.push_back(personal_newAccount(""));
		personal_unlockAccount(m_accounts.back(), "", 100000);
	}
	return m_accounts[_id];
}

RPCSession::RPCSession(const string& _path):
	m_ipcSocket(_path)
{
	string account = personal_newAccount("");
	personal_unlockAccount(account, "", 100000);
	m_accounts.push_back(account);
	test_setChainParams(m_accounts);
}

string RPCSession::TransactionData::toJson() const
{
	Json::Value json;
	json["from"] = (from.length() == 20) ? "0x" + from : from;
	json["to"] = (to.length() == 20 || to == "") ? "0x" + to :  to;
	json["gas"] = gas;
	json["gasprice"] = gasPrice;
	json["value"] = value;
	json["data"] = data;
	return Json::FastWriter().write(json);

}
