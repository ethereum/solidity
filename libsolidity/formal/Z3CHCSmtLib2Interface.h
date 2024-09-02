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

#pragma once

#include <libsmtutil/CHCSmtLib2Interface.h>

namespace solidity::frontend::smt
{

class Z3CHCSmtLib2Interface: public smtutil::CHCSmtLib2Interface
{
public:
	Z3CHCSmtLib2Interface(
		frontend::ReadCallback::Callback _smtCallback,
		std::optional<unsigned int> _queryTimeout,
		bool _computeInvariants
	);

private:
	void setupSmtCallback(bool _disablePreprocessing);

	CHCSolverInterface::QueryResult query(smtutil::Expression const& _expr) override;

	CHCSolverInterface::CexGraph graphFromZ3Answer(std::string const& _proof) const;

	static CHCSolverInterface::CexGraph graphFromSMTLib2Expression(
		smtutil::SMTLib2Expression const& _proof,
		ScopedParser& _context
	);

	bool m_computeInvariants;
};

}
