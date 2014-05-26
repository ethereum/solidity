/*
	This file is part of cpp-ethereum.

	cpp-ethereum is free software: you can redistribute it and/or modify
	it under the terms of the GNU General Public License as published by
	the Free Software Foundation, either version 3 of the License, or
	(at your option) any later version.

	cpp-ethereum is distributed in the hope that it will be useful,
	but WITHOUT ANY WARRANTY; without even the implied warranty of
	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
	GNU General Public License for more details.

	You should have received a copy of the GNU General Public License
	along with cpp-ethereum.  If not, see <http://www.gnu.org/licenses/>.
*/
/** @file Compiler.cpp
 * @author Gav Wood <i@gavwood.com>
 * @date 2014
 */

#include "Compiler.h"
#include "Parser.h"
#include "CompilerState.h"
#include "CodeFragment.h"

using namespace std;
using namespace eth;

bytes eth::compileLLL(string const& _s, vector<string>* _errors)
{
	try
	{
		CompilerState cs;
		bytes ret = CodeFragment::compile(_s, cs).code();
		for (auto i: cs.treesToKill)
			killBigints(i);
		return ret;
	}
	catch (Exception const& _e)
	{
		if (_errors)
			_errors->push_back(_e.description());
	}
	catch (std::exception)
	{
		if (_errors)
			_errors->push_back("Parse error.");
	}
	return bytes();
}

string eth::parseLLL(string const& _src)
{
	sp::utree o;
	parseTreeLLL(_src, o);
	ostringstream ret;
	debugOutAST(ret, o);
	killBigints(o);
	return ret.str();
}
