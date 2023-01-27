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

#include <libevmasm/Disassemble.h>

#include <libsolutil/Common.h>
#include <libsolutil/CommonIO.h>
#include <functional>

using namespace std;
using namespace solidity;
using namespace solidity::util;
using namespace solidity::evmasm;


void solidity::evmasm::eachInstruction(
	bytes const& _mem,
	langutil::EVMVersion _evmVersion,
	function<void(Instruction,u256 const&)> const& _onInstruction
)
{
	for (auto it = _mem.begin(); it < _mem.end(); ++it)
	{
		Instruction const instr{*it};
		int additional = 0;
		if (isValidInstruction(instr))
			additional = instructionInfo(instr, _evmVersion).additional;

		u256 data{};

		// fill the data with the additional data bytes from the instruction stream
		while (additional > 0 && std::next(it) < _mem.end())
		{
			data <<= 8;
			data |= *++it;
			--additional;
		}

		// pad the remaining number of additional octets with zeros
		data <<= 8 * additional;

		_onInstruction(instr, data);
	}
}

string solidity::evmasm::disassemble(bytes const& _mem, langutil::EVMVersion _evmVersion, string const& _delimiter)
{
	stringstream ret;
	eachInstruction(_mem, _evmVersion, [&](Instruction _instr, u256 const& _data) {
		if (!isValidInstruction(_instr))
			ret << "0x" << std::uppercase << std::hex << static_cast<int>(_instr) << _delimiter;
		else
		{
			InstructionInfo info = instructionInfo(_instr, _evmVersion);
			ret << info.name;
			if (info.additional)
				ret << " 0x" << std::uppercase << std::hex << _data;
			ret << _delimiter;
		}
	});
	return ret.str();
}
