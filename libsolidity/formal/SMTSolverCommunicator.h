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

#pragma once

#include <libsolidity/interface/ReadFile.h>

#include <string>

namespace dev
{
namespace solidity
{
namespace smt
{

/// Platform-specific way to access the SMT solver.
class SMTSolverCommunicator
{
public:
	/// Creates the communicator, the read file callback is used only
	/// on the emscripten platform.
	SMTSolverCommunicator(ReadFile::Callback const& _readFileCallback):
		m_readFileCallback(_readFileCallback)
	{}

	std::string communicate(std::string const& _input);

private:
	ReadFile::Callback m_readFileCallback;
};


}
}
}
