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
// SPDX-License-Identifier: GPL-3.0
/**
 * @author Christian <c@ethdev.com>
 * @date 2016
 * Framework for executing contracts and testing them using RPC.
 */

#include <test/ExecutionFramework.h>

#include <test/EVMHost.h>

#include <test/evmc/evmc.hpp>

#include <test/libsolidity/util/SoltestTypes.h>

#include <libsolutil/CommonIO.h>
#include <libsolutil/FunctionSelector.h>

#include <liblangutil/Exceptions.h>

#include <boost/test/framework.hpp>
#include <boost/algorithm/string/replace.hpp>
#include <range/v3/range.hpp>
#include <range/v3/view/transform.hpp>

#include <cstdlib>
#include <limits>

using namespace std;
using namespace solidity;
using namespace solidity::util;
using namespace solidity::test;
using namespace solidity::frontend::test;

ExecutionFramework::ExecutionFramework():
	ExecutionFramework(solidity::test::CommonOptions::get().evmVersion(), solidity::test::CommonOptions::get().vmPaths)
{
}

ExecutionFramework::ExecutionFramework(langutil::EVMVersion _evmVersion, vector<boost::filesystem::path> const& _vmPaths):
	m_evmVersion(_evmVersion),
	m_optimiserSettings(solidity::frontend::OptimiserSettings::minimal()),
	m_showMessages(solidity::test::CommonOptions::get().showMessages),
	m_vmPaths(_vmPaths)
{
	if (solidity::test::CommonOptions::get().optimize)
		m_optimiserSettings = solidity::frontend::OptimiserSettings::standard();

	for (auto const& path: m_vmPaths)
		if (EVMHost::getVM(path.string()).has_capability(EVMC_CAPABILITY_EWASM))
			m_supportsEwasm = true;

	selectVM(evmc_capabilities::EVMC_CAPABILITY_EVM1);
}

void ExecutionFramework::selectVM(evmc_capabilities _cap)
{
	m_evmcHost.reset();
	for (auto const& path: m_vmPaths)
	{
		evmc::VM& vm = EVMHost::getVM(path.string());
		if (vm.has_capability(_cap))
		{
			m_evmcHost = make_unique<EVMHost>(m_evmVersion, vm);
			break;
		}
	}
	solAssert(m_evmcHost != nullptr, "");
	reset();
}

void ExecutionFramework::reset()
{
	m_evmcHost->reset();
	for (size_t i = 0; i < 10; i++)
		m_evmcHost->accounts[EVMHost::convertToEVMC(account(i))].balance =
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
	auto resultHex = boost::replace_all_copy(util::toHex(_result), "0", ".");
	auto expectedHex = boost::replace_all_copy(util::toHex(_expectation), "0", ".");
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

bytes ExecutionFramework::panicData(util::PanicCode _code)
{
	return
		m_evmVersion.supportsReturndata() ?
		toCompactBigEndian(selectorFromSignature32("Panic(uint256)"), 4) + encode(u256(static_cast<unsigned>(_code))) :
		bytes();
}

u256 ExecutionFramework::gasLimit() const
{
	return {m_evmcHost->tx_context.block_gas_limit};
}

u256 ExecutionFramework::gasPrice() const
{
	// here and below we use "return u256{....}" instead of just "return {....}"
	// to please MSVC and avoid unexpected
	// warning C4927 : illegal conversion; more than one user - defined conversion has been implicitly applied
	return u256{EVMHost::convertFromEVMC(m_evmcHost->tx_context.tx_gas_price)};
}

u256 ExecutionFramework::blockHash(u256 const& _number) const
{
	return u256{EVMHost::convertFromEVMC(
		m_evmcHost->get_block_hash(static_cast<int64_t>(_number & numeric_limits<uint64_t>::max()))
	)};
}

u256 ExecutionFramework::blockNumber() const
{
	return m_evmcHost->tx_context.block_number;
}

void ExecutionFramework::sendMessage(bytes const& _data, bool _isCreation, u256 const& _value)
{
	m_evmcHost->newBlock();

	if (m_showMessages)
	{
		if (_isCreation)
			cout << "CREATE " << m_sender.hex() << ":" << endl;
		else
			cout << "CALL   " << m_sender.hex() << " -> " << m_contractAddress.hex() << ":" << endl;
		if (_value > 0)
			cout << " value: " << _value << endl;
		cout << " in:      " << util::toHex(_data) << endl;
	}
	evmc_message message = {};
	message.input_data = _data.data();
	message.input_size = _data.size();
	message.sender = EVMHost::convertToEVMC(m_sender);
	message.value = EVMHost::convertToEVMC(_value);

	if (_isCreation)
	{
		message.kind = EVMC_CREATE;
		message.destination = EVMHost::convertToEVMC(h160{});
	}
	else
	{
		message.kind = EVMC_CALL;
		message.destination = EVMHost::convertToEVMC(m_contractAddress);
	}
	message.gas = InitialGas.convert_to<int64_t>();

	evmc::result result = m_evmcHost->call(message);

	m_output = bytes(result.output_data, result.output_data + result.output_size);
	if (_isCreation)
		m_contractAddress = EVMHost::convertFromEVMC(result.create_address);

	m_gasUsed = InitialGas - result.gas_left;
	m_transactionSuccessful = (result.status_code == EVMC_SUCCESS);

	if (m_showMessages)
	{
		cout << " out:     " << util::toHex(m_output) << endl;
		cout << " result: " << static_cast<size_t>(result.status_code) << endl;
		cout << " gas used: " << m_gasUsed.str() << endl;
	}
}

void ExecutionFramework::sendEther(h160 const& _addr, u256 const& _amount)
{
	m_evmcHost->newBlock();

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
	message.gas = InitialGas.convert_to<int64_t>();

	m_evmcHost->call(message);
}

size_t ExecutionFramework::currentTimestamp()
{
	return static_cast<size_t>(m_evmcHost->tx_context.block_timestamp);
}

size_t ExecutionFramework::blockTimestamp(u256 _block)
{
	if (_block > blockNumber())
		return 0;
	else
		return static_cast<size_t>((currentTimestamp() / blockNumber()) * _block);
}

h160 ExecutionFramework::account(size_t _idx)
{
	return h160(h256(u256{"0x1212121212121212121212121212120000000012"} + _idx * 0x1000), h160::AlignRight);
}

bool ExecutionFramework::addressHasCode(h160 const& _addr) const
{
	return m_evmcHost->get_code_size(EVMHost::convertToEVMC(_addr)) != 0;
}

size_t ExecutionFramework::numLogs() const
{
	return m_evmcHost->recorded_logs.size();
}

size_t ExecutionFramework::numLogTopics(size_t _logIdx) const
{
	return m_evmcHost->recorded_logs.at(_logIdx).topics.size();
}

h256 ExecutionFramework::logTopic(size_t _logIdx, size_t _topicIdx) const
{
	return EVMHost::convertFromEVMC(m_evmcHost->recorded_logs.at(_logIdx).topics.at(_topicIdx));
}

h160 ExecutionFramework::logAddress(size_t _logIdx) const
{
	return EVMHost::convertFromEVMC(m_evmcHost->recorded_logs.at(_logIdx).creator);
}

bytes ExecutionFramework::logData(size_t _logIdx) const
{
	auto const& data = m_evmcHost->recorded_logs.at(_logIdx).data;
	// TODO: Return a copy of log data, because this is expected from REQUIRE_LOG_DATA(),
	//       but reference type like string_view would be preferable.
	return {data.begin(), data.end()};
}

u256 ExecutionFramework::balanceAt(h160 const& _addr) const
{
	return u256(EVMHost::convertFromEVMC(m_evmcHost->get_balance(EVMHost::convertToEVMC(_addr))));
}

bool ExecutionFramework::storageEmpty(h160 const& _addr) const
{
	const auto it = m_evmcHost->accounts.find(EVMHost::convertToEVMC(_addr));
	if (it != m_evmcHost->accounts.end())
	{
		for (auto const& entry: it->second.storage)
			if (!(entry.second.value == evmc::bytes32{}))
				return false;
	}
	return true;
}

vector<solidity::frontend::test::LogRecord> ExecutionFramework::recordedLogs() const
{
	vector<LogRecord> logs;
	for (evmc::MockedHost::log_record const& logRecord: m_evmcHost->recorded_logs)
		logs.emplace_back(
			EVMHost::convertFromEVMC(logRecord.creator),
			bytes{logRecord.data.begin(), logRecord.data.end()},
			logRecord.topics | ranges::views::transform([](evmc::bytes32 _bytes) { return EVMHost::convertFromEVMC(_bytes); }) | ranges::to<vector>
		);
	return logs;
}
