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
/** @file CodeFragment.h
 * @author Gav Wood <i@gavwood.com>
 * @date 2014
 */

#pragma once

#include <libdevcore/Common.h>
#include <libevmasm/Instruction.h>
#include <libevmasm/Assembly.h>
#include "Exceptions.h"

namespace boost { namespace spirit { class utree; } }
namespace sp = boost::spirit;

namespace dev
{
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
	Assembly& assembly(CompilerState const& _cs) { finalise(_cs); return m_asm; }

private:
	void finalise(CompilerState const& _cs);

	template <class T> void error() const { BOOST_THROW_EXCEPTION(T() ); }
	template <class T> void error(std::string const& reason) const {
		auto err = T();
		err << errinfo_comment(reason);
		BOOST_THROW_EXCEPTION(err);
	}
	void constructOperation(sp::utree const& _t, CompilerState& _s);

	bool m_finalised = false;
	Assembly m_asm;
};

static const CodeFragment NullCodeFragment;

}
}

