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
/** @file Assembly.cpp
 * @author Gav Wood <i@gavwood.com>
 * @date 2014
 */

#include "AssemblyItem.h"
#include <libevmasm/SemanticInformation.h>
#include <fstream>

using namespace std;
using namespace dev;
using namespace dev::eth;

AssemblyItem AssemblyItem::toSubAssemblyTag(size_t _subId) const
{
	assertThrow(data() < (u256(1) << 64), Exception, "Tag already has subassembly set.");

	assertThrow(m_type == PushTag || m_type == Tag, Exception, "");
	AssemblyItem r = *this;
	r.m_type = PushTag;
	r.setPushTagSubIdAndTag(_subId, size_t(data()));
	return r;
}

pair<size_t, size_t> AssemblyItem::splitForeignPushTag() const
{
	assertThrow(m_type == PushTag || m_type == Tag, Exception, "");
	return make_pair(size_t((data()) / (u256(1) << 64)) - 1, size_t(data()));
}

void AssemblyItem::setPushTagSubIdAndTag(size_t _subId, size_t _tag)
{
	assertThrow(m_type == PushTag || m_type == Tag, Exception, "");
	setData(_tag + (u256(_subId + 1) << 64));
}

unsigned AssemblyItem::bytesRequired(unsigned _addressLength) const
{
	switch (m_type)
	{
	case Operation:
	case Tag: // 1 byte for the JUMPDEST
		return 1;
	case PushString:
		return 33;
	case Push:
		return 1 + max<unsigned>(1, dev::bytesRequired(data()));
	case PushSubSize:
	case PushProgramSize:
		return 4;		// worst case: a 16MB program
	case PushTag:
	case PushData:
	case PushSub:
		return 1 + _addressLength;
	case PushLibraryAddress:
		return 21;
	default:
		break;
	}
	BOOST_THROW_EXCEPTION(InvalidOpcode());
}

int AssemblyItem::arguments() const
{
	if (type() == Operation)
		return instructionInfo(instruction()).args;
	else
		return 0;
}

int AssemblyItem::returnValues() const
{
	switch (m_type)
	{
	case Operation:
		return instructionInfo(instruction()).ret;
	case Push:
	case PushString:
	case PushTag:
	case PushData:
	case PushSub:
	case PushSubSize:
	case PushProgramSize:
	case PushLibraryAddress:
		return 1;
	case Tag:
		return 0;
	default:;
	}
	return 0;
}

bool AssemblyItem::canBeFunctional() const
{
	switch (m_type)
	{
	case Operation:
		return !SemanticInformation::isDupInstruction(*this) && !SemanticInformation::isSwapInstruction(*this);
	case Push:
	case PushString:
	case PushTag:
	case PushData:
	case PushSub:
	case PushSubSize:
	case PushProgramSize:
	case PushLibraryAddress:
		return true;
	case Tag:
		return false;
	default:;
	}
	return 0;
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

string AssemblyItem::toAssemblyText() const
{
	string text;
	switch (type())
	{
	case Operation:
	{
		assertThrow(isValidInstruction(instruction()), AssemblyException, "Invalid instruction.");
		string name = instructionInfo(instruction()).name;
		transform(name.begin(), name.end(), name.begin(), [](unsigned char _c) { return tolower(_c); });
		text = name;
		break;
	}
	case Push:
		text = toHex(toCompactBigEndian(data(), 1), 1, HexPrefix::Add);
		break;
	case PushString:
		text = string("data_") + toHex(data());
		break;
	case PushTag:
	{
		size_t sub{0};
		size_t tag{0};
		tie(sub, tag) = splitForeignPushTag();
		if (sub == size_t(-1))
			text = string("tag_") + to_string(tag);
		else
			text = string("tag_") + to_string(sub) + "_" + to_string(tag);
		break;
	}
	case Tag:
		assertThrow(data() < 0x10000, AssemblyException, "Declaration of sub-assembly tag.");
		text = string("tag_") + to_string(size_t(data())) + ":";
		break;
	case PushData:
		text = string("data_") + toHex(data());
		break;
	case PushSub:
		text = string("dataOffset(sub_") + to_string(size_t(data())) + ")";
		break;
	case PushSubSize:
		text = string("dataSize(sub_") + to_string(size_t(data())) + ")";
		break;
	case PushProgramSize:
		text = string("bytecodeSize");
		break;
	case PushLibraryAddress:
		text = string("linkerSymbol(\"") + toHex(data()) + string("\")");
		break;
	case UndefinedItem:
		assertThrow(false, AssemblyException, "Invalid assembly item.");
		break;
	default:
		BOOST_THROW_EXCEPTION(InvalidOpcode());
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

ostream& dev::eth::operator<<(ostream& _out, AssemblyItem const& _item)
{
	switch (_item.type())
	{
	case Operation:
		_out << " " << instructionInfo(_item.instruction()).name;
		if (_item.instruction() == solidity::Instruction::JUMP || _item.instruction() == solidity::Instruction::JUMPI)
			_out << "\t" << _item.getJumpTypeAsString();
		break;
	case Push:
		_out << " PUSH " << hex << _item.data();
		break;
	case PushString:
		_out << " PushString"  << hex << (unsigned)_item.data();
		break;
	case PushTag:
	{
		size_t subId = _item.splitForeignPushTag().first;
		if (subId == size_t(-1))
			_out << " PushTag " << _item.splitForeignPushTag().second;
		else
			_out << " PushTag " << subId << ":" << _item.splitForeignPushTag().second;
		break;
	}
	case Tag:
		_out << " Tag " << _item.data();
		break;
	case PushData:
		_out << " PushData " << hex << (unsigned)_item.data();
		break;
	case PushSub:
		_out << " PushSub " << hex << size_t(_item.data());
		break;
	case PushSubSize:
		_out << " PushSubSize " << hex << size_t(_item.data());
		break;
	case PushProgramSize:
		_out << " PushProgramSize";
		break;
	case PushLibraryAddress:
		_out << " PushLibraryAddress " << hex << h256(_item.data()).abridgedMiddle();
		break;
	case UndefinedItem:
		_out << " ???";
		break;
	default:
		BOOST_THROW_EXCEPTION(InvalidOpcode());
	}
	return _out;
}
