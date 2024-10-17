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

#include <libsolidity/formal/Z3SMTLib2Interface.h>

#include <libsolidity/interface/UniversalCallback.h>

#ifdef EMSCRIPTEN_BUILD
#include <z3++.h>
#endif

using namespace solidity::frontend::smt;

Z3SMTLib2Interface::Z3SMTLib2Interface(
	frontend::ReadCallback::Callback _smtCallback,
	std::optional<unsigned int> _queryTimeout
): SMTLib2Interface({}, std::move(_smtCallback), _queryTimeout)
{
#ifdef EMSCRIPTEN_BUILD
	constexpr int resourceLimit = 2000000;
	if (m_queryTimeout)
		z3::set_param("timeout", int(*m_queryTimeout));
	else
		z3::set_param("rlimit", resourceLimit);
	z3::set_param("rewriter.pull_cheap_ite", true);
	z3::set_param("fp.spacer.q3.use_qgen", true);
	z3::set_param("fp.spacer.mbqi", false);
	z3::set_param("fp.spacer.ground_pobs", false);
#endif
}

void Z3SMTLib2Interface::setupSmtCallback() {
	if (auto* universalCallback = m_smtCallback.target<frontend::UniversalCallback>())
		universalCallback->smtCommand().setZ3(m_queryTimeout, true, false);
}

std::string Z3SMTLib2Interface::querySolver(std::string const& _query)
{
#ifdef EMSCRIPTEN_BUILD
	z3::context context;
	return Z3_eval_smtlib2_string(context, _query.c_str());
#else
	return SMTLib2Interface::querySolver(_query);
#endif
}
