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
/** @file IPCSocket.cpp
 * @author Dimtiry Khokhlov <dimitry@ethdev.com>
 * @date 2016
 */

#include <string>
#include <stdio.h>
#include <thread>
#include <libdevcore/CommonData.h>
#include <jsoncpp/json/reader.h>
#include <jsoncpp/json/writer.h>
#include "IPCSocket.h"

using namespace std;
using namespace dev;

IPCSocket::IPCSocket(string const& _path): m_path(_path)
{
	if (_path.length() >= sizeof(sockaddr_un::sun_path))
		BOOST_FAIL("Error opening IPC: socket path is too long!");

	struct sockaddr_un saun;
	saun.sun_family = AF_UNIX;
	strcpy(saun.sun_path, _path.c_str());

	if ((m_socket = socket(AF_UNIX, SOCK_STREAM, 0)) < 0)
		BOOST_FAIL("Error creating IPC socket object");

	int len = sizeof(saun.sun_family) + strlen(saun.sun_path);

	if (connect(m_socket, reinterpret_cast<struct sockaddr const*>(&saun), len) < 0)
		BOOST_FAIL("Error connecting to IPC socket: " << _path);

	m_fp = fdopen(m_socket, "r");
}

string IPCSocket::sendRequest(string const& _req)
{
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

void RPCSession::personal_unlockAccount(string const& _address, string const& _password, int _duration)
{
	rpcCall("personal_unlockAccount", { quote(_address), quote(_password), to_string(_duration) });
}

string RPCSession::personal_newAccount(string const& _password)
{
	return rpcCall("personal_newAccount", { quote(_password) }).asString();
}

void RPCSession::test_setChainParams(string const& _author, string const& _account, string const& _balance)
{
	if (_account.size() < 40)
		return;
	string config = c_genesisConfiguration;
	std::map<string, string> replaceMap;
	replaceMap["[AUTHOR]"] = _author;
	replaceMap["[ACCOUNT]"] = (_account[0] == '0' && _account[1] == 'x') ? _account.substr(2, 40) : _account;
	replaceMap["[BALANCE]"] = _balance;
	parseString(config, replaceMap);
	test_setChainParams(config);
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
	u256 currentBlock = startBlock;
	u256 targetBlock = startBlock + _number;
	rpcCall("test_mineBlocks", { to_string(_number) }, true);

	//@TODO do not use polling - but that would probably need a change to the test client
	for (size_t polls = 0; polls < 100; ++polls)
	{
		currentBlock = fromBigEndian<u256>(fromHex(rpcCall("eth_blockNumber").asString()));
		if (currentBlock >= targetBlock)
			return;
		std::this_thread::sleep_for(chrono::milliseconds(10)); //it does not work faster then 10 ms
	}

	BOOST_FAIL("Error in test_mineBlocks: block mining timeout!");
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
	Json::Reader().parse(reply, result, false);

	if (result.isMember("error"))
	{
		if (_canFail)
			return Json::Value();

		BOOST_FAIL("Error on JSON-RPC call: " + result["error"]["message"].asString());
	}
	return result["result"];
}

RPCSession::RPCSession(const string& _path):
	m_ipcSocket(_path)
{
	for (size_t i = 0; i < 1; ++i)
	{
		string account = personal_newAccount("");
		personal_unlockAccount(account, "", 100000);
		m_accounts.push_back(account);
	}
	test_setChainParams(
		"0x1000000000000000000000000000000000000000",
		m_accounts.front(),
		"1000000000000000000000000000000000000000000000"
	);
}

void RPCSession::parseString(string& _string, map<string, string> const& _varMap)
{
	std::vector<string> types;
	for (std::map<std::string, std::string>::const_iterator it = _varMap.begin(); it != _varMap.end(); it++)
		types.push_back(it->first);

	for (unsigned i = 0; i < types.size(); i++)
	{
		std::size_t pos = _string.find(types.at(i));
		while (pos != std::string::npos)
		{
			_string.replace(pos, types.at(i).size(), _varMap.at(types.at(i)));
			pos = _string.find(types.at(i));
		}
	}
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
