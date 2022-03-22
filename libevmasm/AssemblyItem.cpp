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

#include <libevmasm/AssemblyItem.h>

#include <libevmasm/Assembly.h>

#include <libsolutil/CommonData.h>
#include <libsolutil/CommonIO.h>
#include <libsolutil/Numeric.h>
#include <libsolutil/StringUtils.h>
#include <libsolutil/FixedHash.h>
#include <liblangutil/SourceLocation.h>

#include <fstream>
#include <limits>

using namespace std;
using namespace solidity;
using namespace solidity::evmasm;
using namespace solidity::langutil;

static_assert(sizeof(size_t) <= 8, "size_t must be at most 64-bits wide");

namespace
{

string toStringInHex(u256 _value)
{
	std::stringstream hexStr;
	hexStr << std::uppercase << hex << _value;
	return hexStr.str();
}

}

AssemblyItem AssemblyItem::toSubAssemblyTag(size_t _subId) const
{
	assertThrow(data() < (u256(1) << 64), util::Exception, "Tag already has subassembly set.");
	assertThrow(m_type == PushTag || m_type == Tag, util::Exception, "");
	auto tag = static_cast<size_t>(u256(data()) & 0xffffffffffffffffULL);
	AssemblyItem r = *this;
	r.m_type = PushTag;
	r.setPushTagSubIdAndTag(_subId, tag);
	return r;
}

pair<size_t, size_t> AssemblyItem::splitForeignPushTag() const
{
	assertThrow(m_type == PushTag || m_type == Tag, util::Exception, "");
	u256 combined = u256(data());
	size_t subId = static_cast<size_t>((combined >> 64) - 1);
	size_t tag = static_cast<size_t>(combined & 0xffffffffffffffffULL);
	return make_pair(subId, tag);
}

pair<string, string> AssemblyItem::nameAndData() const
{
	switch (type())
	{
	case Operation:
		return {instructionInfo(instruction()).name, m_data != nullptr ? toStringInHex(*m_data) : ""};
	case Push:
		return {"PUSH", toStringInHex(data())};
	case PushTag:
		if (data() == 0)
			return {"PUSH [ErrorTag]", ""};
		else
			return {"PUSH [tag]", util::toString(data())};
	case PushSub:
		return {"PUSH [$]", toString(util::h256(data()))};
	case PushSubSize:
		return {"PUSH #[$]", toString(util::h256(data()))};
	case PushProgramSize:
		return {"PUSHSIZE", ""};
	case PushLibraryAddress:
		return {"PUSHLIB", toString(util::h256(data()))};
	case PushDeployTimeAddress:
		return {"PUSHDEPLOYADDRESS", ""};
	case PushImmutable:
		return {"PUSHIMMUTABLE", toString(util::h256(data()))};
	case AssignImmutable:
		return {"ASSIGNIMMUTABLE", toString(util::h256(data()))};
	case Tag:
		return {"tag", util::toString(data())};
	case PushData:
		return {"PUSH data", toStringInHex(data())};
	case VerbatimBytecode:
		return {"VERBATIM", util::toHex(verbatimData())};
	default:
		assertThrow(false, InvalidOpcode, "");
	}
}

void AssemblyItem::setPushTagSubIdAndTag(size_t _subId, size_t _tag)
{
	assertThrow(m_type == PushTag || m_type == Tag, util::Exception, "");
	u256 data = _tag;
	if (_subId != numeric_limits<size_t>::max())
		data |= (u256(_subId) + 1) << 64;
	setData(data);
}

size_t AssemblyItem::bytesRequired(size_t _addressLength, Precision _precision) const
{
	switch (m_type)
	{
	case Operation:
	case Tag: // 1 byte for the JUMPDEST
		return 1;
	case Push:
		return 1 + max<size_t>(1, numberEncodingSize(data()));
	case PushSubSize:
	case PushProgramSize:
		return 1 + 4;		// worst case: a 16MB program
	case PushTag:
	case PushData:
	case PushSub:
		return 1 + _addressLength;
	case PushLibraryAddress:
	case PushDeployTimeAddress:
		return 1 + 20;
	case PushImmutable:
		return 1 + 32;
	case AssignImmutable:
	{
		unsigned long immutableOccurrences = 0;

		// Skip exact immutables count if no precise count was requested
		if (_precision == Precision::Approximate)
			immutableOccurrences = 1; // Assume one immut. ref.
		else
		{
			solAssert(m_immutableOccurrences, "No immutable references. `bytesRequired()` called before assembly()?");
			immutableOccurrences = m_immutableOccurrences.value();
		}

		if (immutableOccurrences != 0)
			// (DUP DUP PUSH <n> ADD MSTORE)* (PUSH <n> ADD MSTORE)
			return (immutableOccurrences - 1) * (5 + 32) + (3 + 32);
		else
			// POP POP
			return 2;
	}
	case VerbatimBytecode:
		return std::get<2>(*m_verbatimBytecode).size();
	default:
		break;
	}
	assertThrow(false, InvalidOpcode, "");
}

size_t AssemblyItem::arguments() const
{
	if (type() == Operation)
		return static_cast<size_t>(instructionInfo(instruction()).args);
	else if (type() == VerbatimBytecode)
		return get<0>(*m_verbatimBytecode);
	else if (type() == AssignImmutable)
		return 2;
	else
		return 0;
}

size_t AssemblyItem::returnValues() const
{
	switch (m_type)
	{
	case Operation:
		return static_cast<size_t>(instructionInfo(instruction()).ret);
	case Push:
	case PushTag:
	case PushData:
	case PushSub:
	case PushSubSize:
	case PushProgramSize:
	case PushLibraryAddress:
	case PushImmutable:
	case PushDeployTimeAddress:
		return 1;
	case Tag:
		return 0;
	case VerbatimBytecode:
		return get<1>(*m_verbatimBytecode);
	default:
		break;
	}
	return 0;
}

bool AssemblyItem::canBeFunctional() const
{
	if (m_jumpType != JumpType::Ordinary)
		return false;
	switch (m_type)
	{
	case Operation:
		return !isDupInstruction(instruction()) && !isSwapInstruction(instruction());
	case Push:
	case PushTag:
	case PushData:
	case PushSub:
	case PushSubSize:
	case PushProgramSize:
	case PushLibraryAddress:
	case PushDeployTimeAddress:
	case PushImmutable:
		return true;
	case Tag:
		return false;
	default:
		break;
	}
	return false;
}

string AssemblyItem::getJumpTypeAsString() const
{
	switch (m_jumpType)
	{
	case JumpType::IntoFunction:
		return "[in]";
	case JumpType::OutOfFunction:
		return "[out]";
	case JumpType::Ordinary:
	default:
		return "";
	}
}

void AssemblyItem::setJumpType(std::string const& _jumpType)
{
	if (_jumpType == "[in]")
		m_jumpType = JumpType::IntoFunction;
	else if (_jumpType == "[out]")
		m_jumpType = JumpType::OutOfFunction;
	else if (_jumpType.empty())
		m_jumpType = JumpType::Ordinary;
	else
		assertThrow(false, AssemblyException, "Invalid jump type.");
}

string AssemblyItem::toAssemblyText(Assembly const& _assembly) const
{
	string text;
	switch (type())
	{
	case Operation:
	{
		assertThrow(isValidInstruction(instruction()), AssemblyException, "Invalid instruction.");
		text = util::toLower(instructionInfo(instruction()).name);
		break;
	}
	case Push:
		text = toHex(toCompactBigEndian(data(), 1), util::HexPrefix::Add);
		break;
	case PushTag:
	{
		size_t sub{0};
		size_t tag{0};
		tie(sub, tag) = splitForeignPushTag();
		if (sub == numeric_limits<size_t>::max())
			text = string("tag_") + to_string(tag);
		else
			text = string("tag_") + to_string(sub) + "_" + to_string(tag);
		break;
	}
	case Tag:
		assertThrow(data() < 0x10000, AssemblyException, "Declaration of sub-assembly tag.");
		text = string("tag_") + to_string(static_cast<size_t>(data())) + ":";
		break;
	case PushData:
		text = string("data_") + toHex(data());
		break;
	case PushSub:
	case PushSubSize:
	{
		vector<string> subPathComponents;
		for (size_t subPathComponentId: _assembly.decodeSubPath(static_cast<size_t>(data())))
			subPathComponents.emplace_back("sub_" + to_string(subPathComponentId));
		text =
			(type() == PushSub ? "dataOffset"s : "dataSize"s) +
			"(" +
			solidity::util::joinHumanReadable(subPathComponents, ".") +
			")";
		break;
	}
	case PushProgramSize:
		text = string("bytecodeSize");
		break;
	case PushLibraryAddress:
		text = string("linkerSymbol(\"") + toHex(data()) + string("\")");
		break;
	case PushDeployTimeAddress:
		text = string("deployTimeAddress()");
		break;
	case PushImmutable:
		text = string("immutable(\"") + "0x" + util::toHex(toCompactBigEndian(data(), 1)) + "\")";
		break;
	case AssignImmutable:
		text = string("assignImmutable(\"") + "0x" + util::toHex(toCompactBigEndian(data(), 1)) + "\")";
		break;
	case UndefinedItem:
		assertThrow(false, AssemblyException, "Invalid assembly item.");
		break;
	case VerbatimBytecode:
		text = string("verbatimbytecode_") + util::toHex(get<2>(*m_verbatimBytecode));
		break;
	default:
		assertThrow(false, InvalidOpcode, "");
	}
	if (m_jumpType == JumpType::IntoFunction || m_jumpType == JumpType::OutOfFunction)
	{
		text += "\t//";
		if (m_jumpType == JumpType::IntoFunction)
			text += " in";
		else
			text += " out";
	}
	return text;
}

ostream& solidity::evmasm::operator<<(ostream& _out, AssemblyItem const& _item)
{
	switch (_item.type())
	{
	case Operation:
		_out << " " << instructionInfo(_item.instruction()).name;
		if (_item.instruction() == Instruction::JUMP || _item.instruction() == Instruction::JUMPI)
			_out << "\t" << _item.getJumpTypeAsString();
		break;
	case Push:
		_out << " PUSH " << hex << _item.data() << dec;
		break;
	case PushTag:
	{
		size_t subId = _item.splitForeignPushTag().first;
		if (subId == numeric_limits<size_t>::max())
			_out << " PushTag " << _item.splitForeignPushTag().second;
		else
			_out << " PushTag " << subId << ":" << _item.splitForeignPushTag().second;
		break;
	}
	case Tag:
		_out << " Tag " << _item.data();
		break;
	case PushData:
		_out << " PushData " << hex << static_cast<unsigned>(_item.data()) << dec;
		break;
	case PushSub:
		_out << " PushSub " << hex << static_cast<size_t>(_item.data()) << dec;
		break;
	case PushSubSize:
		_out << " PushSubSize " << hex << static_cast<size_t>(_item.data()) << dec;
		break;
	case PushProgramSize:
		_out << " PushProgramSize";
		break;
	case PushLibraryAddress:
	{
		string hash(util::h256((_item.data())).hex());
		_out << " PushLibraryAddress " << hash.substr(0, 8) + "..." + hash.substr(hash.length() - 8);
		break;
	}
	case PushDeployTimeAddress:
		_out << " PushDeployTimeAddress";
		break;
	case PushImmutable:
		_out << " PushImmutable";
		break;
	case AssignImmutable:
		_out << " AssignImmutable";
		break;
	case VerbatimBytecode:
		_out << " Verbatim " << util::toHex(_item.verbatimData());
		break;
	case UndefinedItem:
		_out << " ???";
		break;
	default:
		assertThrow(false, InvalidOpcode, "");
	}
	return _out;
}

size_t AssemblyItem::opcodeCount() const noexcept
{
	switch (m_type)
	{
		case AssemblyItemType::AssignImmutable:
			// Append empty items if this AssignImmutable was referenced more than once.
			// For n immutable occurrences the first (n - 1) occurrences will
			// generate 5 opcodes and the last will generate 3 opcodes,
			// because it is reusing the 2 top-most elements on the stack.
			solAssert(m_immutableOccurrences, "");

			if (m_immutableOccurrences.value() != 0)
				return (*m_immutableOccurrences - 1) * 5 + 3;
			else
				return 2; // two POP's
		default:
			return 1;
	}
}

std::string AssemblyItem::computeSourceMapping(
	AssemblyItems const& _items,
	map<string, unsigned> const& _sourceIndicesMap
)
{
	string ret;

	int prevStart = -1;
	int prevLength = -1;
	int prevSourceIndex = -1;
	int prevModifierDepth = -1;
	char prevJump = 0;

	for (auto const& item: _items)
	{
		if (!ret.empty())
			ret += ";";

		SourceLocation const& location = item.location();
		int length = location.start != -1 && location.end != -1 ? location.end - location.start : -1;
		int sourceIndex =
			(location.sourceName && _sourceIndicesMap.count(*location.sourceName)) ?
			static_cast<int>(_sourceIndicesMap.at(*location.sourceName)) :
			-1;
		char jump = '-';
		if (item.getJumpType() == evmasm::AssemblyItem::JumpType::IntoFunction)
			jump = 'i';
		else if (item.getJumpType() == evmasm::AssemblyItem::JumpType::OutOfFunction)
			jump = 'o';
		int modifierDepth = static_cast<int>(item.m_modifierDepth);

		unsigned components = 5;
		if (modifierDepth == prevModifierDepth)
		{
			components--;
			if (jump == prevJump)
			{
				components--;
				if (sourceIndex == prevSourceIndex)
				{
					components--;
					if (length == prevLength)
					{
						components--;
						if (location.start == prevStart)
							components--;
					}
				}
			}
		}

		if (components-- > 0)
		{
			if (location.start != prevStart)
				ret += to_string(location.start);
			if (components-- > 0)
			{
				ret += ':';
				if (length != prevLength)
					ret += to_string(length);
				if (components-- > 0)
				{
					ret += ':';
					if (sourceIndex != prevSourceIndex)
						ret += to_string(sourceIndex);
					if (components-- > 0)
					{
						ret += ':';
						if (jump != prevJump)
							ret += jump;
						if (components-- > 0)
						{
							ret += ':';
							if (modifierDepth != prevModifierDepth)
								ret += to_string(modifierDepth);
						}
					}
				}
			}
		}

		if (item.opcodeCount() > 1)
			ret += string(item.opcodeCount() - 1, ';');

		prevStart = location.start;
		prevLength = length;
		prevSourceIndex = sourceIndex;
		prevJump = jump;
		prevModifierDepth = modifierDepth;
	}
	return ret;
}
