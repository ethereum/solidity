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

#include <liblangutil/ErrorReporter.h>
#include <liblangutil/Exceptions.h>

namespace solidity::langutil
{

/*
 * Wrapper for ErrorReporter that removes duplicates.
 * Two errors are considered the same if their error ID and location are the same.
 */
class UniqueErrorReporter
{
public:
	UniqueErrorReporter(): m_errorReporter(m_uniqueErrors) {}

	void append(UniqueErrorReporter const& _other)
	{
		m_errorReporter.append(_other.m_errorReporter.errors());
	}

	void warning(ErrorId _error, SourceLocation const& _location, std::string const& _description)
	{
		if (!seen(_error, _location, _description))
		{
			m_errorReporter.warning(_error, _location, _description);
			markAsSeen(_error, _location, _description);
		}
	}

	void warning(
		ErrorId _error,
		SourceLocation const& _location,
		std::string const& _description,
		SecondarySourceLocation const& _secondaryLocation
	)
	{
		if (!seen(_error, _location, _description))
		{
			m_errorReporter.warning(_error, _location, _description, _secondaryLocation);
			markAsSeen(_error, _location, _description);
		}
	}

	void warning(ErrorId _error, std::string const& _description)
	{
		m_errorReporter.warning(_error, _description);
	}

	void info(ErrorId _error, SourceLocation const& _location, std::string const& _description)
	{
		if (!seen(_error, _location, _description))
		{
			m_errorReporter.info(_error, _location, _description);
			markAsSeen(_error, _location, _description);
		}
	}

	void info(ErrorId _error, std::string const& _description)
	{
		m_errorReporter.info(_error, _description);
	}

	bool seen(ErrorId _error, SourceLocation const& _location, std::string const& _description) const
	{
		if (m_seenErrors.count({_error, _location}))
		{
			solAssert(m_seenErrors.at({_error, _location}) == _description, "");
			return true;
		}
		return false;
	}

	void markAsSeen(ErrorId _error, SourceLocation const& _location, std::string const& _description)
	{
		if (_location != SourceLocation{})
			m_seenErrors[{_error, _location}] = _description;
	}

	ErrorList const& errors() const { return m_errorReporter.errors(); }

	void clear() { m_errorReporter.clear(); }

private:
	ErrorList m_uniqueErrors;
	ErrorReporter m_errorReporter;
	std::map<std::pair<ErrorId, SourceLocation>, std::string> m_seenErrors;
};

}
