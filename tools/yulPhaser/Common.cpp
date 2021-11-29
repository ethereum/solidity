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

#include <tools/yulPhaser/Common.h>

#include <tools/yulPhaser/Exceptions.h>

#include <libsolutil/Assertions.h>

#include <cerrno>
#include <cstring>
#include <fstream>

using namespace std;
using namespace solidity;
using namespace solidity::phaser;

vector<string> phaser::readLinesFromFile(string const& _path)
{
	ifstream inputStream(_path);
	assertThrow(inputStream.is_open(), FileOpenError, "Could not open file '" + _path + "': " + strerror(errno));

	string line;
	vector<string> lines;
	while (!getline(inputStream, line).fail())
		lines.push_back(line);

	assertThrow(!inputStream.bad(), FileReadError, "Error while reading from file '" + _path + "': " + strerror(errno));

	return lines;
}
