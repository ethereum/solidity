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
/** @file CodeLocation.h
 * @author Gav Wood <i@gavwood.com>
 * @date 2014
 */

#pragma once

#include <libethsupport/Common.h>
#include <libethcore/Instruction.h>
#include "Exceptions.h"

namespace eth
{

class CodeFragment;

class CodeLocation
{
	friend class CodeFragment;

public:
	CodeLocation(CodeFragment* _f);
	CodeLocation(CodeFragment* _f, unsigned _p): m_f(_f), m_pos(_p) {}

	unsigned get() const;
	void increase(unsigned _val);
	void set(unsigned _val);
	void set(CodeLocation _loc) { assert(_loc.m_f == m_f); set(_loc.m_pos); }
	void anchor();

	CodeLocation operator+(unsigned _i) const { return CodeLocation(m_f, m_pos + _i); }

private:
	CodeFragment* m_f;
	unsigned m_pos;
};

}
