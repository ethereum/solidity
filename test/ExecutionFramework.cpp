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

#include <libsolutil/CommonIO.h>

#include <boost/test/framework.hpp>
#include <boost/algorithm/string/replace.hpp>

#include <cstdlib>
#include <memory>
#include <tuple>

using namespace std;
using namespace solidity;
using namespace solidity::util;
using namespace solidity::test;

ExecutionFramework::ExecutionFramework():
	ExecutionFramework(solidity::test::CommonOptions::get().evmVersion(), solidity::test::CommonOptions::get().evmcPaths)
{
}

ExecutionFramework::ExecutionFramework(langutil::EVMVersion _evmVersion, std::vector<boost::filesystem::path> const& _evmPaths):
	m_evmVersion(_evmVersion),
	m_optimiserSettings(solidity::frontend::OptimiserSettings::minimal()),
	m_showMessages(solidity::test::CommonOptions::get().showMessages)
{
	m_evmHost = make_shared<EVMHosts>(m_evmVersion, _evmPaths);

	if (solidity::test::CommonOptions::get().optimize)
		m_optimiserSettings = solidity::frontend::OptimiserSettings::standard();

	reset();
}

void ExecutionFramework::reset()
{
	m_evmHost->forEach([&](EVMHost& _host) {
		_host.reset();
		for (size_t i = 0; i < 10; i++)
			_host.accounts[EVMHost::convertToEVMC(account(i))].balance = EVMHost::convertToEVMC(u256(1) << 100);
	});
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

u256 ExecutionFramework::gasLimit() const { return {m_evmHost->tx_context.block_gas_limit}; }

u256 ExecutionFramework::gasPrice() const { return {EVMHost::convertFromEVMC(m_evmHost->tx_context.tx_gas_price)}; }

u256 ExecutionFramework::blockHash(u256 const& _number) const
{
	return {EVMHost::convertFromEVMC(m_evmHost->get_block_hash(uint64_t(_number & numeric_limits<uint64_t>::max())))};
}

u256 ExecutionFramework::blockNumber() const
{
	return m_evmHost->forEach<u256>([](EVMHost& _host) { return _host.tx_context.block_number; });
}

bool operator==(const evmc::result& a, const evmc::result& b)
{
	bool result = false;
	result = a.gas_left == b.gas_left && a.output_size == b.output_size && a.status_code == b.status_code;
	result &= ::strncmp((const char*) a.create_address.bytes, (const char*) b.create_address.bytes, 20) == 0;
	if (result && a.output_data != nullptr && b.output_data != nullptr)
		result &= ::strncmp((const char*) a.output_data, (const char*) b.output_data, a.output_size) == 0;
	return result;
}

bool operator!=(const evmc::result& a, const evmc::result& b) { return !(a == b); }

void ExecutionFramework::sendCreationMessage(
	ContractBytecode const& _contractBytecode, bytes const& _arguments, u256 const& _value)
{
	solAssert(!m_evmHost->empty(), "");
	std::vector<std::tuple<evmc::result, bytes, EVMHost*, Address>> results;

	m_evmHost->forEach([&](EVMHost& _host) {
		_host.newBlock();

		bytes _data;

		if (_host.executesEvmBytecode())
			_data = _contractBytecode.evmBytecode + _arguments;
		if (_host.executesEwasmBytecode())
			_data = _contractBytecode.ewasmBytecode + _arguments;

		evmc_message message = {};
		message.input_data = _data.data();
		message.input_size = _data.size();
		message.sender = EVMHost::convertToEVMC(m_sender);
		message.value = EVMHost::convertToEVMC(_value);

		message.kind = EVMC_CREATE;
		message.destination = EVMHost::convertToEVMC(Address{});
		message.gas = m_gas.convert_to<int64_t>();

		if (m_showMessages)
		{
			cout << "EVMC VM: " << _host.toString() << std::endl;
			cout << "CREATE " << m_sender.hex() << ":" << endl;
			if (_value > 0)
				cout << " value: " << _value << endl;
			cout << " in:      " << toHex(_data) << endl;
		}

		evmc::result result = _host.call(message);
		bytes output = bytes(result.output_data, result.output_data + result.output_size);
		Address address = EVMHost::convertFromEVMC(result.create_address);

		if (m_showMessages)
		{
			cout << " out:     " << toHex(m_output) << endl;
			cout << " result: " << size_t(result.status_code) << endl;
			cout << " gas used: " << m_gasUsed.str() << endl;
		}
		results.emplace_back(std::make_tuple(std::move(result), std::move(output), &_host, address));
	});
	solAssert(!results.empty(), "");
	bool same = true;
	for (auto& result: results)
	{
		if (&result == &*results.begin())
			continue;
		if (std::get<0>(result) != std::get<0>(*results.begin()))
			same = false;
		if (same && std::get<1>(result) != std::get<1>(*results.begin()))
			same = false;
		if (same && std::get<3>(result) != std::get<3>(*results.begin()))
			same = false;
		if (!same)
			break;
	}
	solAssert(same, "different results returned by different evmc vm's.");
	if (m_showMessages)
		cout << std::endl;

	m_transactionSuccessful = std::get<0>(*results.begin()).status_code == EVMC_SUCCESS;
	m_gasUsed = m_gas - std::get<0>(*results.begin()).gas_left;
	m_output = std::get<1>(*results.begin());
	m_evmHost->accounts = std::get<2>(*results.begin())->accounts;
	m_evmHost->tx_context = std::get<2>(*results.begin())->tx_context;
	m_evmHost->block_hash = std::get<2>(*results.begin())->block_hash;
	m_evmHost->call_result = std::get<2>(*results.begin())->call_result;
	m_evmHost->recorded_blockhashes = std::get<2>(*results.begin())->recorded_blockhashes;
	m_evmHost->recorded_account_accesses = std::get<2>(*results.begin())->recorded_account_accesses;
	m_evmHost->recorded_calls = std::get<2>(*results.begin())->recorded_calls;
	m_evmHost->recorded_logs = std::get<2>(*results.begin())->recorded_logs;
	m_evmHost->recorded_selfdestructs = std::get<2>(*results.begin())->recorded_selfdestructs;
	m_contractAddress = std::get<3>(*results.begin());
}

void ExecutionFramework::sendMessage(bytes const& _data, u256 const& _value)
{
	solAssert(!m_evmHost->empty(), "");
	std::vector<std::tuple<evmc::result, bytes, EVMHost*>> results;
	m_evmHost->forEach([&](EVMHost& _host) {
		_host.tx_context = m_evmHost->tx_context;
		_host.newBlock();

		evmc_message message = {};
		message.input_data = _data.data();
		message.input_size = _data.size();
		message.sender = EVMHost::convertToEVMC(m_sender);
		message.value = EVMHost::convertToEVMC(_value);
		message.kind = EVMC_CALL;
		message.destination = EVMHost::convertToEVMC(m_contractAddress);
		message.gas = m_gas.convert_to<int64_t>();

		if (m_showMessages)
		{
			cout << "EVMC VM: " << _host.toString() << std::endl;
			cout << "CALL   " << m_sender.hex() << " -> " << m_contractAddress.hex() << ":" << endl;
			if (_value > 0)
				cout << " value: " << _value << endl;
			cout << " in:      " << toHex(_data) << endl;
		}

		evmc::result result = _host.call(message);
		bytes output = bytes(result.output_data, result.output_data + result.output_size);
		u256 gasUsed = m_gas - result.gas_left;

		if (m_showMessages)
		{
			cout << " EVMC VM: " << _host.toString() << std::endl;
			cout << " out:     " << toHex(output) << endl;
			cout << " result: " << size_t(result.status_code) << endl;
			cout << " gas used: " << gasUsed.str() << endl;
		}
		results.emplace_back(std::make_tuple(std::move(result), std::move(output), &_host));
	});
	solAssert(!results.empty(), "");
	if (m_showMessages)
		cout << std::endl;
	bool same = true;
	for (auto& result: results)
	{
		if (&result == &*results.begin())
			continue;
		if (std::get<0>(result) != std::get<0>(*results.begin()))
			same = false;
		if (same && std::get<1>(result) != std::get<1>(*results.begin()))
			same = false;
		if (!same)
			break;
	}
	solAssert(same, "different results returned by different evmc vm's.");

	m_transactionSuccessful = std::get<0>(*results.begin()).status_code == EVMC_SUCCESS;
	m_gasUsed = m_gas - std::get<0>(*results.begin()).gas_left;
	m_output = std::get<1>(*results.begin());
	m_evmHost->accounts = std::get<2>(*results.begin())->accounts;
	m_evmHost->tx_context = std::get<2>(*results.begin())->tx_context;
	m_evmHost->block_hash = std::get<2>(*results.begin())->block_hash;
	m_evmHost->call_result = std::get<2>(*results.begin())->call_result;
	m_evmHost->recorded_blockhashes = std::get<2>(*results.begin())->recorded_blockhashes;
	m_evmHost->recorded_account_accesses = std::get<2>(*results.begin())->recorded_account_accesses;
	m_evmHost->recorded_calls = std::get<2>(*results.begin())->recorded_calls;
	m_evmHost->recorded_logs = std::get<2>(*results.begin())->recorded_logs;
	m_evmHost->recorded_selfdestructs = std::get<2>(*results.begin())->recorded_selfdestructs;
}

void ExecutionFramework::sendEther(Address const& _addr, u256 const& _amount)
{
	m_evmHost->forEach([&](EVMHost& _host) {
		_host.newBlock();

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

		_host.call(message);
	});
}

size_t ExecutionFramework::currentTimestamp()
{
	return m_evmHost->forEach<int64_t>([](EVMHost& _host) -> int64_t { return _host.tx_context.block_timestamp; });
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
	return m_evmHost->forEach<bool>(
		[&_addr](EVMHost& _host) -> bool { return _host.get_code_size(EVMHost::convertToEVMC(_addr)) != 0; });
}

size_t ExecutionFramework::numLogs() const
{
	return m_evmHost->forEach<size_t>([](EVMHost& _host) -> size_t { return _host.recorded_logs.size(); });
}

size_t ExecutionFramework::numLogTopics(size_t _logIdx) const
{
	return m_evmHost->forEach<size_t>(
		[&_logIdx](EVMHost& _host) -> size_t { return _host.recorded_logs.at(_logIdx).topics.size(); });
}

h256 ExecutionFramework::logTopic(size_t _logIdx, size_t _topicIdx) const
{
	return m_evmHost->forEach<u256>([&_logIdx, &_topicIdx](EVMHost& _host) -> u256 {
		return EVMHost::convertFromEVMC(_host.recorded_logs.at(_logIdx).topics.at(_topicIdx));
	});
}

Address ExecutionFramework::logAddress(size_t _logIdx) const
{
	return m_evmHost->forEach<Address>([&_logIdx](EVMHost& _host) -> Address {
		return EVMHost::convertFromEVMC(_host.recorded_logs.at(_logIdx).creator);
	});
}

bytes ExecutionFramework::logData(size_t _logIdx) const
{
	return m_evmHost->forEach<bytes>([&_logIdx](EVMHost& _host) -> bytes {
		const auto& data = _host.recorded_logs.at(_logIdx).data;
		// TODO: Return a copy of log data, because this is expected from REQUIRE_LOG_DATA(),
		//       but reference type like string_view would be preferable.
		return {data.begin(), data.end()};
	});
}

u256 ExecutionFramework::balanceAt(Address const& _addr)
{
	return m_evmHost->forEach<u256>([&](EVMHost& _host) {
		return u256(EVMHost::convertFromEVMC(_host.get_balance(EVMHost::convertToEVMC(_addr))));
	});
}

bool ExecutionFramework::storageEmpty(Address const& _addr)
{
	const auto it = m_evmHost->accounts.find(EVMHost::convertToEVMC(_addr));
	if (it != m_evmHost->accounts.end())
	{
		for (auto const& entry: it->second.storage)
			if (!(entry.second.value == evmc::bytes32{}))
				return false;
	}
	return true;
}
