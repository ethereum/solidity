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

#include <liblangutil/Exceptions.h>

#include <boost/algorithm/string/split.hpp>
#include <boost/algorithm/string.hpp>

#include <iostream>

using namespace solidity;
using namespace solidity::langutil;
using namespace std;

SourceLocation solidity::langutil::parseSourceLocation(string const& _input, vector<shared_ptr<string const>> const& _sourceNames)
{
	// Expected input: "start:length:sourceindex"
	enum SrcElem: size_t { Start, Length, Index };

	vector<string> pos;

	boost::algorithm::split(pos, _input, boost::is_any_of(":"));

	solAssert(pos.size() == 3, "SourceLocation string must have 3 colon separated numeric fields.");
	auto const sourceIndex = stoi(pos[Index]);

	astAssert(
		sourceIndex == -1 || (0 <= sourceIndex && static_cast<size_t>(sourceIndex) < _sourceNames.size()),
		"'src'-field ill-formatted or src-index too high"
	);

	int start = stoi(pos[Start]);
	int end = start + stoi(pos[Length]);

	SourceLocation result{start, end, {}};
	if (sourceIndex != -1)
		result.sourceName = _sourceNames.at(static_cast<size_t>(sourceIndex));
	return result;
}

std::ostream& solidity::langutil::operator<<(std::ostream& _out, SourceLocation const& _location)
{
	if (!_location.isValid())
		return _out << "NO_LOCATION_SPECIFIED";

	if (_location.sourceName)
		_out << *_location.sourceName;

	_out << "[" << _location.start << "," << _location.end << "]";

	return _out;
}
