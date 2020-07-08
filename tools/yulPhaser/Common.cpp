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
