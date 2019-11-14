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

#include <test/EVMHost.h>

#include <test/evmc/evmc.hpp>
#include <test/evmc/loader.h>

#include <libdevcore/CommonIO.h>

#include <boost/test/framework.hpp>
#include <boost/algorithm/string/replace.hpp>

#include <cstdlib>

using namespace std;
using namespace dev;
using namespace dev::test;

ExecutionFramework::ExecutionFramework():
	ExecutionFramework(dev::test::Options::get().evmVersion())
{
}

ExecutionFramework::ExecutionFramework(langutil::EVMVersion _evmVersion):
	m_evmVersion(_evmVersion),
	m_optimiserSettings(solidity::OptimiserSettings::minimal()),
	m_showMessages(dev::test::Options::get().showMessages),
	m_evmHost(make_shared<EVMHost>(m_evmVersion))
{
	if (dev::test::Options::get().optimizeYul)
		m_optimiserSettings = solidity::OptimiserSettings::full();
	else if (dev::test::Options::get().optimize)
		m_optimiserSettings = solidity::OptimiserSettings::standard();
	m_evmHost->reset();

	for (size_t i = 0; i < 10; i++)
		m_evmHost->m_state.accounts[EVMHost::convertToEVMC(account(i))].balance =
			EVMHost::convertToEVMC(u256(1) << 100);

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

u256 ExecutionFramework::gasLimit() const
{
	return {m_evmHost->get_tx_context().block_gas_limit};
}

u256 ExecutionFramework::gasPrice() const
{
	return {EVMHost::convertFromEVMC(m_evmHost->get_tx_context().tx_gas_price)};
}

u256 ExecutionFramework::blockHash(u256 const& _number) const
{
	return {EVMHost::convertFromEVMC(m_evmHost->get_block_hash(uint64_t(_number & numeric_limits<uint64_t>::max())))};
}

u256 ExecutionFramework::blockNumber() const
{
	return m_evmHost->m_state.blockNumber;
}

void ExecutionFramework::sendMessage(bytes const& _data, bool _isCreation, u256 const& _value)
{
	m_evmHost->newBlock();

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
	evmc_message message = {};
	message.input_data = _data.data();
	message.input_size = _data.size();
	message.sender = EVMHost::convertToEVMC(m_sender);
	message.value = EVMHost::convertToEVMC(_value);

	if (_isCreation)
	{
		message.kind = EVMC_CREATE;
		message.destination = EVMHost::convertToEVMC(Address{});
	}
	else
	{
		message.kind = EVMC_CALL;
		message.destination = EVMHost::convertToEVMC(m_contractAddress);
	}
	message.gas = m_gas.convert_to<int64_t>();

	evmc::result result = m_evmHost->call(message);

	m_output = bytes(result.output_data, result.output_data + result.output_size);
	if (_isCreation)
		m_contractAddress = EVMHost::convertFromEVMC(result.create_address);

	m_gasUsed = m_gas - result.gas_left;
	m_transactionSuccessful = (result.status_code == EVMC_SUCCESS);

	if (m_showMessages)
	{
		cout << " out:     " << toHex(m_output) << endl;
		cout << " result: " << size_t(result.status_code) << endl;
		cout << " gas used: " << m_gasUsed.str() << endl;
	}
}

void ExecutionFramework::sendEther(Address const& _addr, u256 const& _amount)
{
	m_evmHost->newBlock();

	if (m_showMessages)
	{
		cout << "SEND_ETHER   " << m_sender.hex() << " -> " << _addr.hex() << ":" << endl;
		if (_amount > 0)
			cout << " value: " << _amount << endl;
	}
	evmc_message message = {};
	message.sender = EVMHost::convertToEVMC(m_sender);
	message.value = EVMHost::convertToEVMC(_amount);
	message.kind = EVMC_CALL;
	message.destination = EVMHost::convertToEVMC(_addr);
	message.gas = m_gas.convert_to<int64_t>();

	m_evmHost->call(message);
}

size_t ExecutionFramework::currentTimestamp()
{
	return m_evmHost->get_tx_context().block_timestamp;
}

size_t ExecutionFramework::blockTimestamp(u256 _block)
{
	if (_block > blockNumber())
		return 0;
	else
		return size_t((currentTimestamp() / blockNumber()) * _block);
}

Address ExecutionFramework::account(size_t _idx)
{
	return Address(h256(u256{"0x1212121212121212121212121212120000000012"} + _idx * 0x1000), Address::AlignRight);
}

bool ExecutionFramework::addressHasCode(Address const& _addr)
{
	return m_evmHost->get_code_size(EVMHost::convertToEVMC(_addr)) != 0;
}

size_t ExecutionFramework::numLogs() const
{
	return m_evmHost->m_state.logs.size();
}

size_t ExecutionFramework::numLogTopics(size_t _logIdx) const
{
	return m_evmHost->m_state.logs.at(_logIdx).topics.size();
}

h256 ExecutionFramework::logTopic(size_t _logIdx, size_t _topicIdx) const
{
	return m_evmHost->m_state.logs.at(_logIdx).topics.at(_topicIdx);
}

Address ExecutionFramework::logAddress(size_t _logIdx) const
{
	return m_evmHost->m_state.logs.at(_logIdx).address;
}

bytes const& ExecutionFramework::logData(size_t _logIdx) const
{
	return m_evmHost->m_state.logs.at(_logIdx).data;
}

u256 ExecutionFramework::balanceAt(Address const& _addr)
{
	return u256(EVMHost::convertFromEVMC(m_evmHost->get_balance(EVMHost::convertToEVMC(_addr))));
}

bool ExecutionFramework::storageEmpty(Address const& _addr)
{
	if (EVMHost::Account const* acc = m_evmHost->account(EVMHost::convertToEVMC(_addr)))
	{
		for (auto const& entry: acc->storage)
			if (!(entry.second == evmc::bytes32{}))
				return false;
	}
	return true;
}
