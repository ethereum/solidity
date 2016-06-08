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
/** @file IPCSocket.h
 * @author Dimtiry Khokhlov <dimitry@ethdev.com>
 * @date 2016
 */

#include <string>
#include <stdio.h>
#include <map>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/un.h>
#include <boost/test/unit_test.hpp>

using namespace std;

class IPCSocket
{
public:
	IPCSocket(string const& _address);
	string sendRequest(string const& _req);
	~IPCSocket() { close(m_socket); fclose(m_fp); }

private:
	FILE *m_fp;
	string m_address;
	int m_socket;

};

class RPCRequest
{
public:
	struct transactionData
	{
		string from;
		string to;
		string gas;
		string gasPrice;
		string value;
		string data;
	};

	struct transactionReceipt
	{
		string gasUsed;
		string contractAddress;
	};

	RPCRequest(string const& _localSocketAddress): m_ipcSocket(_localSocketAddress) {}
	string eth_getCode(string const& _address, string const& _blockNumber);
	string eth_call(transactionData const& _td, string const& _blockNumber);
	transactionReceipt eth_getTransactionReceipt(string const& _transactionHash);
	string eth_sendTransaction(transactionData const& _transactionData);
	string eth_sendTransaction(string const& _transaction);
	string eth_getBalance(string const& _address, string const& _blockNumber);
	string personal_newAccount(string const& _password);
	void personal_unlockAccount(string const& _address, string const& _password, int _duration);
	void test_setChainParams(string const& _author, string const& _account, string const& _balance);
	void test_setChainParams(string const& _config);
	void test_mineBlocks(int _number);
	string rpcCall(string const& _methodName, vector<string> const& _args);

private:
	inline string makeString(string const& _arg) { return "\"" + _arg + "\""; }
	inline string getReply(string const& _what, string const& _arg);
	/// Parse string replacing keywords to values
	void parseString(string& _string, map<string, string> const& _varMap);

	IPCSocket m_ipcSocket;
	size_t m_rpcSequence = 1;

	//Just working example of the node configuration file
	string const c_genesisConfiguration = R"(
	{
		"sealEngine": "NoProof",
		"options": {
		},
		"params": {
			"accountStartNonce": "0x",
			"maximumExtraDataSize": "0x1000000",
			"blockReward": "0x",
			"registrar": ""
		},
		"genesis": {
			"author": "[AUTHOR]",
			"timestamp": "0x00",
			"parentHash": "0x0000000000000000000000000000000000000000000000000000000000000000",
			"extraData": "0x",
			"gasLimit": "0x1000000000000"
		},
		"accounts": {
			"0000000000000000000000000000000000000001": { "wei": "1", "precompiled": { "name": "ecrecover", "linear": { "base": 3000, "word": 0 } } },
			"0000000000000000000000000000000000000002": { "wei": "1", "precompiled": { "name": "sha256", "linear": { "base": 60, "word": 12 } } },
			"0000000000000000000000000000000000000003": { "wei": "1", "precompiled": { "name": "ripemd160", "linear": { "base": 600, "word": 120 } } },
			"0000000000000000000000000000000000000004": { "wei": "1", "precompiled": { "name": "identity", "linear": { "base": 15, "word": 3 } } },
			"[ACCOUNT]": { "wei": "[BALANCE]" }
		},
		"network": {
			"nodes": [
					 ]
		}
	}
	)";

	string const c_transaction = R"(
	{
		"from": "[FROM]",
		"to": "[TO]",
		"gas": "[GAS]",
		"gasPrice": "[GASPRICE]",
		"value": "[VALUE]",
		"data": "[DATA]"
	}
	)";
};

