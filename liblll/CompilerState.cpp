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
/** @file CompilerState.cpp
 * @author Gav Wood <i@gavwood.com>
 * @date 2014
 */

#include "CompilerState.h"
#include "CodeFragment.h"

using namespace std;
using namespace dev;
using namespace dev::eth;

CompilerState::CompilerState()
{
}

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
	"(def 'allgas (- (gas) 21))"
	"(def 'send (to value) (call allgas to value 0 0 0 0))"
	"(def 'send (gaslimit to value) (call gaslimit to value 0 0 0 0))"
	"(def 'msg (gaslimit to value data datasize outsize) { (set x outsize) (set y (alloc @32)) (call gaslimit to value data datasize @0 @32) @0 })"
	"(def 'msg (gaslimit to value data datasize) { (call gaslimit to value data datasize 0 32) @0 })"
	"(def 'msg (gaslimit to value data) { [0]:data (msg gaslimit to value 0 32) })"
	"(def 'msg (to value data) { [0]:data (msg allgas to value 0 32) })"
	"(def 'msg (to data) { [0]:data (msg allgas to 0 0 32) })"
	"(def 'create (value code) { [0]:(msize) (create value @0 (lll code @0)) })"
	"(def 'create (code) { [0]:(msize) (create 0 @0 (lll code @0)) })"
	"(def 'sha3 (val) { [0]:val (sha3 0 32) })"
	"(def 'sha3pair (a b) { [0]:a [32]:b (sha3 0 64) })"
	"(def 'sha3trip (a b c) { [0]:a [32]:b [64]:c (sha3 0 96) })"
	"(def 'keccak256 (loc len) (sha3 loc len))"
	"(def 'return (val) { [0]:val (return 0 32) })"
	"(def 'returnlll (code) (return 0 (lll code 0)) )"
	"(def 'makeperm (name pos) { (def name (sload pos)) (def name (v) (sstore pos v)) } )"
	"(def 'permcount 0)"
	"(def 'perm (name) { (makeperm name permcount) (def 'permcount (+ permcount 1)) } )"
	"(def 'ecrecover (r s v hash) { [0] r [32] s [64] v [96] hash (msg allgas 1 0 0 128) })"
	"(def 'sha256 (data datasize) (msg allgas 2 0 data datasize))"
	"(def 'ripemd160 (data datasize) (msg allgas 3 0 data datasize))"
	"(def 'sha256 (val) { [0]:val (sha256 0 32) })"
	"(def 'ripemd160 (val) { [0]:val (ripemd160 0 32) })"
	"(def 'wei 1)"
	"(def 'szabo 1000000000000)"
	"(def 'finney 1000000000000000)"
	"(def 'ether 1000000000000000000)"
	"}";
	CodeFragment::compile(s, *this);
}
