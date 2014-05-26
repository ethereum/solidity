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
/** @file CodeLocation.cpp
 * @author Gav Wood <i@gavwood.com>
 * @date 2014
 */

#include "CodeLocation.h"
#include "CodeFragment.h"
using namespace std;
using namespace eth;

CodeLocation::CodeLocation(CodeFragment* _f)
{
	m_f = _f;
	m_pos = _f->m_code.size();
}

unsigned CodeLocation::get() const
{
	assert(m_f->m_code[m_pos - 1] == (byte)Instruction::PUSH4);
	bytesConstRef r(&m_f->m_code[m_pos], 4);
	return fromBigEndian<uint32_t>(r);
}

void CodeLocation::set(unsigned _val)
{
	assert(m_f->m_code[m_pos - 1] == (byte)Instruction::PUSH4);
	assert(!get());
	bytesRef r(&m_f->m_code[m_pos], 4);
	toBigEndian(_val, r);
}

void CodeLocation::anchor()
{
	set(m_f->m_code.size());
}

void CodeLocation::increase(unsigned _val)
{
	assert(m_f->m_code[m_pos - 1] == (byte)Instruction::PUSH4);
	bytesRef r(&m_f->m_code[m_pos], 4);
	toBigEndian(get() + _val, r);
}

