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

#include <libsolidity/formal/EldaricaCHCSmtLib2Interface.h>

#include <libsolidity/interface/UniversalCallback.h>

using namespace solidity::frontend::smt;

EldaricaCHCSmtLib2Interface::EldaricaCHCSmtLib2Interface(
	frontend::ReadCallback::Callback _smtCallback,
	std::optional<unsigned int> _queryTimeout,
	bool computeInvariants
): CHCSmtLib2Interface({}, std::move(_smtCallback), _queryTimeout), m_computeInvariants(computeInvariants)
{
}

std::string EldaricaCHCSmtLib2Interface::querySolver(std::string const& _input)
{
	if (auto* universalCallback = m_smtCallback.target<frontend::UniversalCallback>())
		universalCallback->smtCommand().setEldarica(m_queryTimeout, m_computeInvariants);

	return CHCSmtLib2Interface::querySolver(_input);
}
