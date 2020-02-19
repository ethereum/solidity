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

#include "ExtractorExecutionFramework.h"

#include <test/EVMHost.h>

#include <test/evmc/evmc.hpp>

#include <libsolutil/CommonIO.h>

#include <boost/algorithm/string/replace.hpp>
#include <boost/test/framework.hpp>

#include <cstdlib>

using namespace std;
using namespace solidity;
using namespace solidity::util;
using namespace solidity::test;

bool operator==(solidity::bytes const &_left, solidity::bytes const &_right)
{
	std::string sig(ExtractorExecutionFramework::formatString(_left));
	sig = sig.substr(0, sig.find('{'));

	std::string parameters(ExtractorExecutionFramework::formatString(_left));
	parameters = parameters.substr(parameters.find('{') + 1, parameters.find('}') - parameters.find('{') - 1);

	std::string result(ExtractorExecutionFramework::formatString(_right));

	ExtractorExecutionFramework::m_current->addExpectation(sig, parameters, result);

	return true;
};

solidity::bytes operator+(solidity::bytes const &_left, solidity::bytes const &_right)
{
	std::string left(ExtractorExecutionFramework::formatString(_left));
	std::string right(ExtractorExecutionFramework::formatString(_right));
	if (!right.empty())
		right = ", " + right;
	return frontend::test::BytesUtils::convertString(left + right);
}
solidity::bytes operator+(const char *_left, solidity::bytes const &_right)
{
	return operator+(frontend::test::BytesUtils::convertString(_left), _right);
}

solidity::bytes operator+(solidity::bytes const &_left, const char *_right)
{
	return operator+(_left, frontend::test::BytesUtils::convertString(_right));
}

ExtractionTask *ExtractorExecutionFramework::m_current{nullptr};

ExtractorExecutionFramework::ExtractorExecutionFramework()
    : ExtractorExecutionFramework(solidity::test::CommonOptions::get().evmVersion())
{
}

ExtractorExecutionFramework::ExtractorExecutionFramework(langutil::EVMVersion _evmVersion)
    : m_evmVersion(_evmVersion), m_optimiserSettings(solidity::frontend::OptimiserSettings::minimal()),
      m_showMessages(solidity::test::CommonOptions::get().showMessages), m_evmHost(std::make_shared<FakeEvmHost>())
{
	if (solidity::test::CommonOptions::get().optimizeYul)
		m_optimiserSettings = solidity::frontend::OptimiserSettings::full();
	else if (solidity::test::CommonOptions::get().optimize)
		m_optimiserSettings = solidity::frontend::OptimiserSettings::standard();
}

u256 ExtractorExecutionFramework::gasLimit() const
{
	m_current->extractionNotPossible("Use of m_evmHost");
	return 0;
}

u256 ExtractorExecutionFramework::gasPrice() const
{
	m_current->extractionNotPossible("Use of m_evmHost");
	return 0;
}

u256 ExtractorExecutionFramework::blockHash(u256 const &_number) const
{
	(void) _number;

	m_current->extractionNotPossible("Use of m_evmHost");
	return 0;
}

u256 ExtractorExecutionFramework::blockNumber() const
{
	m_current->extractionNotPossible("Use of m_evmHost");
	static u256 block{0};
	return (++block) % 1024;
}

void ExtractorExecutionFramework::sendMessage(bytes const &_data, bool _isCreation, u256 const &_value)
{
	(void) _data;
	(void) _isCreation;
	(void) _value;

	//	m_evmHost->newBlock();
	//
	//	if (m_showMessages)
	//	{
	//		if (_isCreation)
	//			cout << "CREATE " << m_sender.hex() << ":" << endl;
	//		else
	//			cout << "CALL   " << m_sender.hex() << " -> " << m_contractAddress.hex() << ":" << endl;
	//		if (_value > 0)
	//			cout << " value: " << _value << endl;
	//		cout << " in:      " << toHex(_data) << endl;
	//	}
	//	evmc_message message = {};
	//	message.input_data = _data.data();
	//	message.input_size = _data.size();
	//	message.sender = EVMHost::convertToEVMC(m_sender);
	//	message.value = EVMHost::convertToEVMC(_value);
	//
	//	if (_isCreation)
	//	{
	//		message.kind = EVMC_CREATE;
	//		message.destination = EVMHost::convertToEVMC(Address{});
	//	}
	//	else
	//	{
	//		message.kind = EVMC_CALL;
	//		message.destination = EVMHost::convertToEVMC(m_contractAddress);
	//	}
	//	message.gas = m_gas.convert_to<int64_t>();
	//
	//	evmc::result result = m_evmHost->call(message);
	//
	//	m_output = bytes(result.output_data, result.output_data + result.output_size);
	//	if (_isCreation)
	//		m_contractAddress = EVMHost::convertFromEVMC(result.create_address);
	//
	//	m_gasUsed = m_gas - result.gas_left;
	//	m_transactionSuccessful = (result.status_code == EVMC_SUCCESS);
	//
	//	if (m_showMessages)
	//	{
	//		cout << " out:     " << toHex(m_output) << endl;
	//		cout << " result: " << size_t(result.status_code) << endl;
	//		cout << " gas used: " << m_gasUsed.str() << endl;
	//	}
	m_current->extractionNotPossible("send used");
}

void ExtractorExecutionFramework::sendEther(Address const &_addr, u256 const &_amount)
{
	(void) _addr;
	(void) _amount;
	m_current->extractionNotPossible("Use of m_evmHost");

	//	m_evmHost->newBlock();
	//
	//	if (m_showMessages)
	//	{
	//		cout << "SEND_ETHER   " << m_sender.hex() << " -> " << _addr.hex() << ":" << endl;
	//		if (_amount > 0)
	//			cout << " value: " << _amount << endl;
	//	}
	//	evmc_message message = {};
	//	message.sender = EVMHost::convertToEVMC(m_sender);
	//	message.value = EVMHost::convertToEVMC(_amount);
	//	message.kind = EVMC_CALL;
	//	message.destination = EVMHost::convertToEVMC(_addr);
	//	message.gas = m_gas.convert_to<int64_t>();
	//
	//	m_evmHost->call(message);
}

size_t ExtractorExecutionFramework::currentTimestamp()
{
	m_current->extractionNotPossible("Use of m_evmHost");
	return 0;
}

size_t ExtractorExecutionFramework::blockTimestamp(u256 _block)
{
	(void) _block;

	m_current->extractionNotPossible("Use of m_evmHost");
	return 0;
}

Address ExtractorExecutionFramework::account(size_t _idx)
{
	(void) _idx;

	if (m_current)
		m_current->extractionNotPossible("Use of m_evmHost");
	return Address(h256(u256{"0x1212121212121212121212121212120000000012"} + _idx * 0x1000), Address::AlignRight);
}

bool ExtractorExecutionFramework::addressHasCode(Address const &_addr)
{
	(void) _addr;

	m_current->extractionNotPossible("Use of m_evmHost");
	return 0;
}

size_t ExtractorExecutionFramework::numLogs() const
{
	m_current->extractionNotPossible("Use of m_evmHost");
	return 0;
}

size_t ExtractorExecutionFramework::numLogTopics(size_t _logIdx) const
{
	(void) _logIdx;

	m_current->extractionNotPossible("Use of m_evmHost");
	return 0;
}

h256 ExtractorExecutionFramework::logTopic(size_t _logIdx, size_t _topicIdx) const
{
	(void) _logIdx;
	(void) _topicIdx;

	m_current->extractionNotPossible("Use of m_evmHost");
	return h256();
}

Address ExtractorExecutionFramework::logAddress(size_t _logIdx) const
{
	(void) _logIdx;

	m_current->extractionNotPossible("Use of m_evmHost");
	return Address();
}

bytes ExtractorExecutionFramework::logData(size_t _logIdx) const
{
	(void) _logIdx;

	m_current->extractionNotPossible("Use of m_evmHost");
	return bytes();
}

u256 ExtractorExecutionFramework::balanceAt(Address const &_addr)
{
	(void) _addr;

	m_current->extractionNotPossible("Use of m_evmHost");
	return 0;
}

bool ExtractorExecutionFramework::storageEmpty(Address const &_addr)
{
	(void) _addr;

	m_current->extractionNotPossible("Use of m_evmHost");
	return true;
}
