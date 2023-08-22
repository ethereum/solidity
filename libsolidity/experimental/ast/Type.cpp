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

#include <libsolidity/experimental/ast/Type.h>
#include <libsolidity/ast/AST.h>
#include <libsolutil/Visitor.h>

#include <range/v3/view/drop_last.hpp>
#include <range/v3/view/zip.hpp>

#include <sstream>

using namespace solidity;
using namespace solidity::frontend::experimental;

bool Sort::operator==(Sort const& _rhs) const
{
	if (classes.size() != _rhs.classes.size())
		return false;
	for (auto [lhs, rhs]: ranges::zip_view(classes, _rhs.classes))
		if (lhs != rhs)
			return false;
	return true;
}

bool Sort::operator<=(Sort const& _rhs) const
{
	for (auto c: classes)
		if (!_rhs.classes.count(c))
			return false;
	return true;
}

Sort Sort::operator+(Sort const& _rhs) const
{
	Sort result { classes };
	result.classes += _rhs.classes;
	return result;
}


Sort Sort::operator-(Sort const& _rhs) const
{
	Sort result { classes };
	result.classes -= _rhs.classes;
	return result;
}
