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
/**
 * @author Christian <c@ethdev.com>
 * @date 2014
 * Full-stack compiler that converts a source code string to bytecode.
 */

#include <libsolidity/AST.h>
#include <libsolidity/Scanner.h>
#include <libsolidity/Parser.h>
#include <libsolidity/NameAndTypeResolver.h>
#include <libsolidity/Compiler.h>
#include <libsolidity/CompilerStack.h>

using namespace std;

namespace dev
{
namespace solidity
{

bytes CompilerStack::compile(std::string const& _sourceCode, shared_ptr<Scanner> _scanner,
							 bool _optimize)
{
	if (!_scanner)
		_scanner = make_shared<Scanner>();
	_scanner->reset(CharStream(_sourceCode));

	ASTPointer<ContractDefinition> contract = Parser().parse(_scanner);
	NameAndTypeResolver().resolveNamesAndTypes(*contract);
	return Compiler::compile(*contract, _optimize);
}

}
}
