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
*/
/**
 * @author Christian <c@ethdev.com>
 * @date 2016
 * Framework for executing contracts and testing them using RPC.
 */

#include <test/ExecutionFramework.h>

#include <libdevcore/CommonIO.h>

#include <boost/test/framework.hpp>
#include <boost/algorithm/string/replace.hpp>

#include <cstdlib>

using namespace std;
using namespace dev;
using namespace dev::test;

namespace // anonymous
{

h256 const EmptyTrie("0x56e81f171bcc55a6ff8345e692c0f86e5b48e01b996cadc001622fb5e363b421");

string getIPCSocketPath()
{
	string ipcPath = dev::test::Options::get().ipcPath;
	if (ipcPath.empty())
		BOOST_FAIL("ERROR: ipcPath not set! (use --ipcpath <path> or the environment variable ETH_TEST_IPC)");

	return ipcPath;
}

}

ExecutionFramework::ExecutionFramework() :
	m_rpc(RPCSession::instance(getIPCSocketPath())),
	m_evmVersion(dev::test::Options::get().evmVersion()),
	m_optimize(dev::test::Options::get().optimize),
	m_showMessages(dev::test::Options::get().showMessages),
	m_sender(m_rpc.account(0))
{
	m_rpc.test_rewindToBlock(0);
}

std::pair<bool, string> ExecutionFramework::compareAndCreateMessage(
	bytes const& _result,
	bytes const& _expectation
)
{
	if (_result == _expectation)
		return std::make_pair(true, std::string{});
	std::string message =
			"Invalid encoded data\n"
			"   Result                                                           Expectation\n";
	auto resultHex = boost::replace_all_copy(toHex(_result), "0", ".");
	auto expectedHex = boost::replace_all_copy(toHex(_expectation), "0", ".");
	for (size_t i = 0; i < std::max(resultHex.size(), expectedHex.size()); i += 0x40)
	{
		std::string result{i >= resultHex.size() ? string{} : resultHex.substr(i, 0x40)};
		std::string expected{i > expectedHex.size() ? string{} : expectedHex.substr(i, 0x40)};
		message +=
			(result == expected ? "   " : " X ") +
			result +
			std::string(0x41 - result.size(), ' ') +
			expected +
			"\n";
	}
	return make_pair(false, message);
}

void ExecutionFramework::sendMessage(bytes const& _data, bool _isCreation, u256 const& _value)
{
	if (m_showMessages)
	{
		if (_isCreation)
			cout << "CREATE " << m_sender.hex() << ":" << endl;
		else
			cout << "CALL   " << m_sender.hex() << " -> " << m_contractAddress.hex() << ":" << endl;
		if (_value > 0)
			cout << " value: " << _value << endl;
		cout << " in:      " << toHex(_data) << endl;
	}
	RPCSession::TransactionData d;
	d.data = "0x" + toHex(_data);
	d.from = "0x" + toString(m_sender);
	d.gas = toHex(m_gas, HexPrefix::Add);
	d.gasPrice = toHex(m_gasPrice, HexPrefix::Add);
	d.value = toHex(_value, HexPrefix::Add);
	if (!_isCreation)
	{
		d.to = dev::toString(m_contractAddress);
		BOOST_REQUIRE(m_rpc.eth_getCode(d.to, "latest").size() > 2);
		// Use eth_call to get the output
		m_output = fromHex(m_rpc.eth_call(d, "latest"), WhenError::Throw);
	}

	string txHash = m_rpc.eth_sendTransaction(d);
	m_rpc.test_mineBlocks(1);
	RPCSession::TransactionReceipt receipt(m_rpc.eth_getTransactionReceipt(txHash));

	m_blockNumber = u256(receipt.blockNumber);

	if (_isCreation)
	{
		m_contractAddress = Address(receipt.contractAddress);
		BOOST_REQUIRE(m_contractAddress);
		string code = m_rpc.eth_getCode(receipt.contractAddress, "latest");
		m_output = fromHex(code, WhenError::Throw);
	}

	if (m_showMessages)
	{
		cout << " out:     " << toHex(m_output) << endl;
		cout << " tx hash: " << txHash << endl;
	}

	m_gasUsed = u256(receipt.gasUsed);
	m_logs.clear();
	for (auto const& log: receipt.logEntries)
	{
		LogEntry entry;
		entry.address = Address(log.address);
		for (auto const& topic: log.topics)
			entry.topics.push_back(h256(topic));
		entry.data = fromHex(log.data, WhenError::Throw);
		m_logs.push_back(entry);
	}

	if (!receipt.status.empty())
		m_transactionSuccessful = (receipt.status == "1");
	else
		m_transactionSuccessful = (m_gas != m_gasUsed);
}

void ExecutionFramework::sendEther(Address const& _to, u256 const& _value)
{
	RPCSession::TransactionData d;
	d.data = "0x";
	d.from = "0x" + toString(m_sender);
	d.gas = toHex(m_gas, HexPrefix::Add);
	d.gasPrice = toHex(m_gasPrice, HexPrefix::Add);
	d.value = toHex(_value, HexPrefix::Add);
	d.to = dev::toString(_to);

	string txHash = m_rpc.eth_sendTransaction(d);
	m_rpc.test_mineBlocks(1);
}

size_t ExecutionFramework::currentTimestamp()
{
	auto latestBlock = m_rpc.eth_getBlockByNumber("latest", false);
	return size_t(u256(latestBlock.get("timestamp", "invalid").asString()));
}

size_t ExecutionFramework::blockTimestamp(u256 _number)
{
	auto latestBlock = m_rpc.eth_getBlockByNumber(toString(_number), false);
	return size_t(u256(latestBlock.get("timestamp", "invalid").asString()));
}

Address ExecutionFramework::account(size_t _i)
{
	return Address(m_rpc.accountCreateIfNotExists(_i));
}

bool ExecutionFramework::addressHasCode(Address const& _addr)
{
	string code = m_rpc.eth_getCode(toString(_addr), "latest");
	return !code.empty() && code != "0x";
}

u256 ExecutionFramework::balanceAt(Address const& _addr)
{
	return u256(m_rpc.eth_getBalance(toString(_addr), "latest"));
}

bool ExecutionFramework::storageEmpty(Address const& _addr)
{
	h256 root(m_rpc.eth_getStorageRoot(toString(_addr), "latest"));
	BOOST_CHECK(root);
	return root == EmptyTrie;
}
