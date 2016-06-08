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
#include "IPCSocket.h"
using namespace std;

IPCSocket::IPCSocket(string const& _path): m_address(_path)
{
	if (_path.length() > 108)
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

string RPCRequest::eth_getCode(string const& _address, string const& _blockNumber)
{
	return getReply("result\":", rpcCall("eth_getCode", { makeString(_address), makeString(_blockNumber) }));
}

RPCRequest::transactionReceipt RPCRequest::eth_getTransactionReceipt(string const& _transactionHash)
{
	transactionReceipt receipt;
	string srpcCall = rpcCall("eth_getTransactionReceipt", { makeString(_transactionHash) });
	receipt.gasUsed = getReply("gasUsed\":" , srpcCall);
	receipt.contractAddress = getReply("contractAddress\":" , srpcCall);
	return receipt;
}

string RPCRequest::eth_sendTransaction(transactionData const& _td)
{
	string transaction = c_transaction;
	std::map<string, string> replaceMap;
	replaceMap["[FROM]"] = (_td.from.length() == 20) ? "0x" + _td.from : _td.from;
	replaceMap["[TO]"] = (_td.to.length() == 20 || _td.to == "") ? "0x" + _td.to :  _td.to;
	replaceMap["[GAS]"] = _td.gas;
	replaceMap["[GASPRICE]"] = _td.gasPrice;
	replaceMap["[VALUE]"] = _td.value;
	replaceMap["[DATA]"] = _td.data;
	parseString(transaction, replaceMap);
	return getReply("result\":", rpcCall("eth_sendTransaction", { transaction }));
}

string RPCRequest::eth_call(transactionData const& _td, string const& _blockNumber)
{
	string transaction = c_transaction;
	std::map<string, string> replaceMap;
	replaceMap["[FROM]"] = (_td.from.length() == 20) ? "0x" + _td.from : _td.from;
	replaceMap["[TO]"] = (_td.to.length() == 20 || _td.to == "") ? "0x" + _td.to : _td.to;
	replaceMap["[GAS]"] = _td.gas;
	replaceMap["[GASPRICE]"] = _td.gasPrice;
	replaceMap["[VALUE]"] = _td.value;
	replaceMap["[DATA]"] = _td.data;
	parseString(transaction, replaceMap);
	return getReply("result\":", rpcCall("eth_call", { transaction, makeString(_blockNumber) }));
}

string RPCRequest::eth_sendTransaction(string const& _transaction)
{
	return getReply("result\":", rpcCall("eth_sendTransaction", { _transaction }));
}

string RPCRequest::eth_getBalance(string const& _address, string const& _blockNumber)
{
	string address = (_address.length() == 20) ? "0x" + _address : _address;
	return getReply("result\":", rpcCall("eth_getBalance", { makeString(address), makeString(_blockNumber) }));
}

void RPCRequest::personal_unlockAccount(string const& _address, string const& _password, int _duration)
{
	rpcCall("personal_unlockAccount", { makeString(_address), makeString(_password), to_string(_duration) });
}

string RPCRequest::personal_newAccount(string const& _password)
{
	return getReply("result\":", rpcCall("personal_newAccount", { makeString(_password) }));
}

void RPCRequest::test_setChainParams(string const& _author, string const& _account, string const& _balance)
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

void RPCRequest::test_setChainParams(string const& _config)
{
	rpcCall("test_setChainParams", { _config });
}

void RPCRequest::test_mineBlocks(int _number)
{
	rpcCall("test_mineBlocks", { to_string(_number) });
	std::this_thread::sleep_for(chrono::seconds(1));
}

string RPCRequest::rpcCall(string const& _methodName, vector<string> const& _args)
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

	string reply = m_ipcSocket.sendRequest(request);
	//cout << "Request: " << request << endl;
	//cout << "Reply: " << reply << endl;
	return reply;
}

void RPCRequest::parseString(string& _string, map<string, string> const& _varMap)
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

string RPCRequest::getReply(string const& _what, string const& _arg)
{
	string reply = "";
	size_t posStart = _arg.find(_what);
	size_t posEnd = _arg.find(",", posStart);
	if (posEnd == string::npos)
		posEnd = _arg.find("}", posStart);
	if (posStart != string::npos)
		reply = _arg.substr(posStart + _what.length(), posEnd - posStart - _what.length());
	reply.erase(std::remove(reply.begin(), reply.end(), '"'), reply.end());
	return reply;
}
