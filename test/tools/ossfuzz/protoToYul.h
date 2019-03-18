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

#include <cstdint>
#include <cstddef>
#include <string>

#include <test/tools/ossfuzz/yulProto.pb.h>

namespace yul
{
namespace test
{
namespace yul_fuzzer
{
class Function;

std::string functionToString(Function const& input);
std::string protoToYul(uint8_t const* data, size_t size);
std::ostream& operator<<(std::ostream& _os, BinaryOp const& _x);
std::ostream& operator<<(std::ostream& _os, Block const& _x);
std::ostream& operator<<(std::ostream& _os, Literal const& _x);
std::ostream& operator<<(std::ostream& _os, VarRef const& _x);
std::ostream& operator<<(std::ostream& _os, Expression const& _x);
std::ostream& operator<<(std::ostream& _os, BinaryOp const& _x);
std::ostream& operator<<(std::ostream& _os, VarDecl const& _x);
std::ostream& operator<<(std::ostream& _os, TypedVarDecl const& _x);
std::ostream& operator<<(std::ostream& _os, UnaryOp const& _x);
std::ostream& operator<<(std::ostream& _os, AssignmentStatement const& _x);
std::ostream& operator<<(std::ostream& _os, IfStmt const& _x);
std::ostream& operator<<(std::ostream& _os, StoreFunc const& _x);
std::ostream& operator<<(std::ostream& _os, Statement const& _x);
std::ostream& operator<<(std::ostream& _os, Block const& _x);
std::ostream& operator<<(std::ostream& _os, Function const& _x);
std::ostream& operator<<(std::ostream& _os, ForStmt const& _x);
std::ostream& operator<<(std::ostream& _os, CaseStmt const& _x);
std::ostream& operator<<(std::ostream& _os, SwitchStmt const& _x);
}
}
}
