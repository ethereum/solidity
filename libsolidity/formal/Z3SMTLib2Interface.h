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

#include <libsmtutil/SMTLib2Interface.h>

namespace solidity::frontend::smt
{

class Z3SMTLib2Interface: public smtutil::SMTLib2Interface
{
public:
	explicit Z3SMTLib2Interface(
		frontend::ReadCallback::Callback _smtCallback = {},
		std::optional<unsigned> _queryTimeout = {}
	);
private:
	void setupSmtCallback() override;
	std::string querySolver(std::string const& _query) override;
};

}
