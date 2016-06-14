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
/**
 * @author Christian <c@ethdev.com>
 * @date 2016
 * Framework for executing Solidity contracts and testing them against C++ implementation.
 */

#include <test/libsolidity/SolidityExecutionFramework.h>


using namespace std;
using namespace dev;
using namespace dev::solidity;
using namespace dev::solidity::test;


ExecutionFramework::ExecutionFramework():
	m_rpc(RPCSession::instance("/home/wins/Ethereum/testnet/ethnode1/geth.ipc")),
	m_sender(m_rpc.account(0))
{
	eth::NoProof::init();
	if (g_logVerbosity != -1)
		g_logVerbosity = 0;

	m_rpc.test_rewindToBlock(0);
}

void ExecutionFramework::sendMessage(bytes const& _data, bool _isCreation, u256 const& _value)
{
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

	if (_isCreation)
	{
		m_contractAddress = Address(receipt.contractAddress);
		BOOST_REQUIRE(m_contractAddress);
		string code = m_rpc.eth_getCode(receipt.contractAddress, "latest");
		BOOST_REQUIRE(code.size() > 2);
		m_output = fromHex(code, WhenError::Throw);
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
	return h256(m_rpc.eth_getStorageRoot(toString(_addr), "latest")) == EmptySHA3;
}
