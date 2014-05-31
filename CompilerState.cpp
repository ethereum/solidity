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
/** @file CompilerState.cpp
 * @author Gav Wood <i@gavwood.com>
 * @date 2014
 */

#include "CompilerState.h"
#include "CodeFragment.h"

using namespace std;
using namespace eth;

CodeFragment const& CompilerState::getDef(std::string const& _s)
{
	if (defs.count(_s))
		return defs.at(_s);
	else if (args.count(_s))
		return args.at(_s);
	else if (outers.count(_s))
		return outers.at(_s);
	else
		return NullCodeFragment;
}

void CompilerState::populateStandard()
{
	static const string s = "{"
	"(def 'gav 0x8a40bfaa73256b60764c1bf40675a99083efb075)"
	"(def 'send (to value) (call (- (gas) 21) to value 0 0 0 0))"
	"(def 'send (gaslimit to value) (call gaslimit to value 0 0 0 0))"
	"(def 'alloc (len) (asm msize 0 1 len msize add sub mstore8))"
	"(def 'msg (gaslimit to value data datasize outsize) { [32]:outsize [0]:(alloc @32) (call gaslimit to value data datasize @0 @32) @0 })"
	"(def 'msg (gaslimit to value data datasize) { (call gaslimit to value data datasize 0 32) @0 })"
	"(def 'msg (gaslimit to value data) { [0]:data (msg gaslimit to value 0 32) })"
	"(def 'msg (to value data) { [0]:data (msg 0 to value 0 32) })"
	"(def 'msg (to data) { [0]:data (msg 0 to 0 0 32) })"
	"(def 'create (value code) { [0]:(msize) (create value @0 (lll code @0)) })"
	"(def 'create (code) { [0]:(msize) (create 0 @0 (lll code @0)) })"
	"(def 'sha3 (val) { [0]:val (sha3 0 32) })"
	"(def 'return (val) { [0]:val (return 0 32) })"
	"(def 'makeperm (name pos) { (def name (sload pos)) (def name (v) (sstore pos v)) } )"
	"(def 'permcount 0)"
	"(def 'perm (name) { (makeperm name permcount) (def 'permcount (+ permcount 1)) } )"
	"}";
	CodeFragment::compile(s, *this);
}
