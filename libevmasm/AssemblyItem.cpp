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

#include <libevmasm/AssemblyItem.h>

#include <libdevcore/CommonData.h>
#include <libdevcore/FixedHash.h>

#include <fstream>

using namespace std;
using namespace dev;
using namespace dev::eth;

static_assert(sizeof(size_t) <= 8, "size_t must be at most 64-bits wide");

AssemblyItem AssemblyItem::toSubAssemblyTag(size_t _subId) const
{
	assertThrow(data() < (u256(1) << 64), Exception, "Tag already has subassembly set.");
	assertThrow(m_type == PushTag || m_type == Tag, Exception, "");
	size_t tag = size_t(u256(data()) & 0xffffffffffffffffULL);
	AssemblyItem r = *this;
	r.m_type = PushTag;
	r.setPushTagSubIdAndTag(_subId, tag);
	return r;
}

pair<size_t, size_t> AssemblyItem::splitForeignPushTag() const
{
	assertThrow(m_type == PushTag || m_type == Tag, Exception, "");
	u256 combined = u256(data());
	size_t subId = size_t((combined >> 64) - 1);
	size_t tag = size_t(combined & 0xffffffffffffffffULL);
	return make_pair(subId, tag);
}

void AssemblyItem::setPushTagSubIdAndTag(size_t _subId, size_t _tag)
{
	assertThrow(m_type == PushTag || m_type == Tag, Exception, "");
	u256 data = _tag;
	if (_subId != size_t(-1))
		data |= (u256(_subId) + 1) << 64;
	setData(data);
}

unsigned AssemblyItem::bytesRequired(unsigned _addressLength) const
{
	switch (m_type)
	{
	case Operation:
	case Tag: // 1 byte for the JUMPDEST
		return 1;
	case PushString:
		return 1 + 32;
	case Push:
		return 1 + max<unsigned>(1, dev::bytesRequired(data()));
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
	case PushDeployTimeAddress:
		return 1;
	case Tag:
		return 0;
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
	case PushString:
	case PushTag:
	case PushData:
	case PushSub:
	case PushSubSize:
	case PushProgramSize:
	case PushLibraryAddress:
	case PushDeployTimeAddress:
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
		text = toHex(toCompactBigEndian(data(), 1), HexPrefix::Add);
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
	case PushDeployTimeAddress:
		text = string("deployTimeAddress()");
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
		_out << " PUSH " << hex << _item.data() << dec;
		break;
	case PushString:
		_out << " PushString"  << hex << (unsigned)_item.data() << dec;
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
		_out << " PushData " << hex << (unsigned)_item.data() << dec;
		break;
	case PushSub:
		_out << " PushSub " << hex << size_t(_item.data()) << dec;
		break;
	case PushSubSize:
		_out << " PushSubSize " << hex << size_t(_item.data()) << dec;
		break;
	case PushProgramSize:
		_out << " PushProgramSize";
		break;
	case PushLibraryAddress:
	{
		string hash(h256((_item.data())).hex());
		_out << " PushLibraryAddress " << hash.substr(0, 8) + "..." + hash.substr(hash.length() - 8);
		break;
	}
	case PushDeployTimeAddress:
		_out << " PushDeployTimeAddress";
		break;
	case UndefinedItem:
		_out << " ???";
		break;
	default:
		BOOST_THROW_EXCEPTION(InvalidOpcode());
	}
	return _out;
}
