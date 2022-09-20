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
/**
 * @author Liana <liana@ethdev.com>
 * @date 2015
 * Solidity exception hierarchy.
 */

#include <liblangutil/Exceptions.h>

#include <boost/algorithm/string/case_conv.hpp>
#include <boost/algorithm/string/trim.hpp>

using namespace std;
using namespace solidity;
using namespace solidity::langutil;

Error::Error(
	ErrorId _errorId, Error::Type _type,
	std::string const& _description,
	SourceLocation const& _location,
	SecondarySourceLocation const& _secondaryLocation
):
	m_errorId(_errorId),
	m_type(_type)
{
	if (_location.isValid())
		*this << errinfo_sourceLocation(_location);
	if (!_secondaryLocation.infos.empty())
		*this << errinfo_secondarySourceLocation(_secondaryLocation);
	if (!_description.empty())
		*this << util::errinfo_comment(_description);
}

SourceLocation const* Error::sourceLocation() const noexcept
{
	return boost::get_error_info<errinfo_sourceLocation>(*this);
}

SecondarySourceLocation const* Error::secondarySourceLocation() const noexcept
{
	return boost::get_error_info<errinfo_secondarySourceLocation>(*this);
}
