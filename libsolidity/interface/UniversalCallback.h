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

#include <libsolidity/interface/FileReader.h>
#include <libsolidity/interface/SMTSolverCommand.h>

namespace solidity::frontend
{

/// UniversalCallback is used to wrap both FileReader and SMTSolverCommand
/// callbacks in a single callback.  It uses the Kind of the callback to
/// determine which to call internally.
class UniversalCallback
{
public:
	UniversalCallback(FileReader* _fileReader, SMTSolverCommand& _solver) :
		m_fileReader{_fileReader},
		m_solver{_solver}
	{}

	frontend::ReadCallback::Callback callback()
	{
		return [this](std::string const& _kind, std::string const& _data) -> ReadCallback::Result {
			if (_kind == ReadCallback::kindString(ReadCallback::Kind::ReadFile))
				if (!m_fileReader)
					return ReadCallback::Result{false, "No import callback."};
				else
					return m_fileReader->readFile(_kind, _data);
			else if (_kind == ReadCallback::kindString(ReadCallback::Kind::SMTQuery))
				return m_solver.solve(_kind, _data);
			solAssert(false, "Unknown callback kind.");
		};
	}

	void resetImportCallback() { m_fileReader = nullptr; }

private:
	FileReader* m_fileReader;
	SMTSolverCommand& m_solver;
};

}
