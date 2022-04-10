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

#include <test/tools/ossfuzz/YulEvmoneInterface.h>

#include <libyul/Exceptions.h>

using namespace solidity;
using namespace solidity::test::fuzzer;
using namespace solidity::yul;

bytes YulAssembler::assemble()
{
	if (
		!m_stack.parseAndAnalyze("source", m_yulProgram) ||
		!m_stack.parserResult()->code ||
		!m_stack.parserResult()->analysisInfo ||
		langutil::Error::containsErrors(m_stack.errors())
	)
		yulAssert(false, "Yul program could not be parsed successfully.");

	if (m_optimiseYul)
		m_stack.optimize();
	return m_stack.assemble(YulStack::Machine::EVM).bytecode->bytecode;
}

evmc::result YulEvmoneUtility::deployCode(bytes const& _input, EVMHost& _host)
{
	// Zero initialize all message fields
	evmc_message msg = {};
	// Gas available (value of type int64_t) is set to its maximum value
	msg.gas = std::numeric_limits<int64_t>::max();
	solAssert(
		_input.size() <= 0xffff,
		"Deployed byte code is larger than the permissible 65535 bytes."
	);
	uint8_t inputSizeHigher = static_cast<uint8_t>(_input.size() >> 8);
	uint8_t inputSizeLower = _input.size() & 0xff;
	/*
	 * CODESIZE
	 * PUSH0 0xc
	 * PUSH0 0x0
	 * CODECOPY
	 * PUSH1 <INPUTSIZE>
	 * PUSH0 0x0
	 * RETURN
	 */
	bytes deployCode = bytes{
		0x38, 0x60, 0x0c, 0x60, 0x00, 0x39, 0x61,
		inputSizeHigher, inputSizeLower,
		0x60, 0x00, 0xf3
	} + _input;
	msg.input_data = deployCode.data();
	msg.input_size = deployCode.size();
	msg.kind = EVMC_CREATE;
	return _host.call(msg);
}

evmc_message YulEvmoneUtility::callMessage(evmc_address _address)
{
	evmc_message call = {};
	call.gas = std::numeric_limits<int64_t>::max();
	call.destination = _address;
	call.kind = EVMC_CALL;
	return call;
}

bool YulEvmoneUtility::seriousCallError(evmc_status_code _code)
{
	if (_code == EVMC_OUT_OF_GAS)
		return true;
	else if (_code == EVMC_STACK_OVERFLOW)
		return true;
	else if (_code == EVMC_STACK_UNDERFLOW)
		return true;
	else if (_code == EVMC_INTERNAL_ERROR)
		return true;
	else if (_code == EVMC_UNDEFINED_INSTRUCTION)
		return true;
	else if (_code == EVMC_INVALID_MEMORY_ACCESS)
		return true;
	else
		return false;
}
