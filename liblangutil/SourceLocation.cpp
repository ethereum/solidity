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

#include <liblangutil/Exceptions.h>

#include <boost/algorithm/string/split.hpp>
#include <boost/algorithm/string.hpp>

using namespace solidity;
namespace solidity::langutil
{

SourceLocation const parseSourceLocation(std::string const& _input, std::string const& _sourceName, size_t _maxIndex)
{
	// Expected input: "start:length:sourceindex"
	enum SrcElem : size_t { Start, Length, Index };

	std::vector<std::string> pos;

	boost::algorithm::split(pos, _input, boost::is_any_of(":"));

	astAssert(
		pos.size() == 3 &&
		_maxIndex >= static_cast<size_t>(stoi(pos[Index])),
		"'src'-field ill-formatted or src-index too high"
	);

	int start = stoi(pos[Start]);
	int end = start + stoi(pos[Length]);

	// ASSUMPTION: only the name of source is used from here on, the m_source of the CharStream-Object can be empty
	std::shared_ptr<langutil::CharStream> source = std::make_shared<langutil::CharStream>("", _sourceName);

	return SourceLocation{start, end, source};
}

}
