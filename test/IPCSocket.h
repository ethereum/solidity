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
#include <jsoncpp/json/value.h>
#include <boost/test/unit_test.hpp>

class IPCSocket: public boost::noncopyable
{
public:
	IPCSocket(std::string const& _path);
	std::string sendRequest(std::string const& _req);
	~IPCSocket() { close(m_socket); fclose(m_fp); }

	std::string const& path() const { return m_path; }

private:
	FILE *m_fp;
	std::string m_path;
	int m_socket;

};

class RPCSession: public boost::noncopyable
{
public:
	struct TransactionData
	{
		std::string from;
		std::string to;
		std::string gas;
		std::string gasPrice;
		std::string value;
		std::string data;

		std::string toJson() const;
	};

	struct TransactionReceipt
	{
		std::string gasUsed;
		std::string contractAddress;
	};

	static RPCSession& instance(std::string const& _path);

	std::string eth_getCode(std::string const& _address, std::string const& _blockNumber);
	std::string eth_call(TransactionData const& _td, std::string const& _blockNumber);
	TransactionReceipt eth_getTransactionReceipt(std::string const& _transactionHash);
	std::string eth_sendTransaction(TransactionData const& _transactionData);
	std::string eth_sendTransaction(std::string const& _transaction);
	std::string eth_getBalance(std::string const& _address, std::string const& _blockNumber);
	std::string personal_newAccount(std::string const& _password);
	void personal_unlockAccount(std::string const& _address, std::string const& _password, int _duration);
	void test_setChainParams(std::string const& _author, std::string const& _account, std::string const& _balance);
	void test_setChainParams(std::string const& _config);
	void test_rewindToBlock(size_t _blockNr);
	void test_mineBlocks(int _number);
	Json::Value rpcCall(std::string const& _methodName, std::vector<std::string> const& _args = std::vector<std::string>(), bool _canFail = false);

	std::string const& account(size_t _id) const { return m_accounts.at(_id); }

private:
	RPCSession(std::string const& _path);

	inline std::string quote(std::string const& _arg) { return "\"" + _arg + "\""; }
	/// Parse std::string replacing keywords to values
	void parseString(std::string& _string, std::map<std::string, std::string> const& _varMap);

	IPCSocket m_ipcSocket;
	size_t m_rpcSequence = 1;

	//Just working example of the node configuration file
	std::string const c_genesisConfiguration = R"(
	{
		"sealEngine": "NoProof",
		"options": {
		},
		"params": {
			"accountStartNonce": "0x",
			"maximumExtraDataSize": "0x1000000",
			"blockReward": "0x",
			"registrar": "",
			"allowFutureBlocks": "1"
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

	std::vector<std::string> m_accounts;
};

