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

#include <libsolidity/formal/SMTSolverCommunicator.h>

#include <libdevcore/Common.h>

#include <boost/filesystem/operations.hpp>

#include <fstream>

#include <stdio.h>

using namespace std;
using namespace dev;
using namespace dev::solidity::smt;

#ifdef EMSCRIPTEN

string SMTSolverCommunicator::communicate(string const& _input)
{
	auto result = m_readFileCallback("SMTLIB2Solver>> " + _input);
	if (result.success)
		return result.contentsOrErrorMessage;
	else
		return "";
}

#else

#ifndef _WIN32
inline FILE* _popen(char const* command, char const* type)
{
	return popen(command, type);
}
inline int _pclose(FILE* file)
{
	return pclose(file);
}
#endif

string SMTSolverCommunicator::communicate(string const& _input)
{
	namespace fs = boost::filesystem;
	auto tempPath = fs::unique_path(fs::temp_directory_path() / "%%%%-%%%%-%%%%.smt2");
	ScopeGuard s1([&]() { fs::remove(tempPath); });
	ofstream(tempPath.string()) << _input << "(exit)" << endl;

	// TODO Escaping might not be 100% perfect.
	FILE* solverOutput = _popen(("z3 -smt2 \"" + tempPath.string() + "\"").c_str(), "r");
	ScopeGuard s2([&]() { _pclose(solverOutput); });

	string result;
	array<char, 128> buffer;
	while (!feof(solverOutput))
		if (fgets(buffer.data(), 127, solverOutput) != nullptr)
			result += buffer.data();
	return result;
}

#endif
