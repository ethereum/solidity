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
/** @file CodeFragment.h
 * @author Gav Wood <i@gavwood.com>
 * @date 2014
 */

#pragma once

#include <libethsupport/Common.h>
#include <libethcore/Instruction.h>
#include "Assembly.h"
#include "Exceptions.h"

namespace boost { namespace spirit { class utree; } }
namespace sp = boost::spirit;

namespace eth
{

struct CompilerState;

class CodeFragment
{
public:
	CodeFragment() {}
	CodeFragment(sp::utree const& _t, CompilerState& _s, bool _allowASM = false);

	static CodeFragment compile(std::string const& _src, CompilerState& _s);

	/// Consolidates data and compiles code.
	bytes code() const { return m_asm.assemble(); }

	/// Consolidates data and compiles code.
	std::string assembly() const { return m_asm.out(); }

	/// Optimise the code. Best do this just before calling code() or assembly().
	void optimise() { m_asm.optimise(); }

private:
	template <class T> void error() const { throw T(); }
	void constructOperation(sp::utree const& _t, CompilerState& _s);

	Assembly m_asm;
};

static const CodeFragment NullCodeFragment;

}
