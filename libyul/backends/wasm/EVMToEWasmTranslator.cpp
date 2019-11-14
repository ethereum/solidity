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
/**
 * Translates Yul code from EVM dialect to eWasm dialect.
 */

#include <libyul/backends/wasm/EVMToEWasmTranslator.h>

#include <libyul/backends/wasm/WordSizeTransform.h>
#include <libyul/backends/wasm/WasmDialect.h>
#include <libyul/optimiser/ExpressionSplitter.h>
#include <libyul/optimiser/FunctionGrouper.h>
#include <libyul/optimiser/MainFunction.h>
#include <libyul/optimiser/FunctionHoister.h>
#include <libyul/optimiser/Disambiguator.h>
#include <libyul/optimiser/NameDisplacer.h>
#include <libyul/optimiser/OptimiserStep.h>

#include <libyul/AsmParser.h>
#include <libyul/AsmAnalysis.h>
#include <libyul/AsmAnalysisInfo.h>
#include <libyul/Object.h>

#include <liblangutil/ErrorReporter.h>
#include <liblangutil/Scanner.h>
#include <liblangutil/SourceReferenceFormatter.h>

using namespace std;
using namespace dev;
using namespace yul;
using namespace langutil;

namespace
{
static string const polyfill{R"({
function or_bool(a, b, c, d) -> r {
	r := i64.or(i64.or(a, b), i64.or(c, d))
}
// returns a + y + c plus carry value on 64 bit values.
// c should be at most 1
function add_carry(x, y, c) -> r, r_c {
	let t := i64.add(x, y)
	r := i64.add(t, c)
	r_c := i64.or(
		i64.lt_u(t, x),
		i64.lt_u(r, t)
	)
}
function add(x1, x2, x3, x4, y1, y2, y3, y4) -> r1, r2, r3, r4 {
	let carry
	r4, carry := add_carry(x4, y4, 0)
	r3, carry := add_carry(x3, y3, carry)
	r2, carry := add_carry(x2, y2, carry)
	r1, carry := add_carry(x1, y1, carry)
}
function bit_negate(x) -> y {
	y := i64.xor(x, 0xffffffffffffffff)
}
function sub(x1, x2, x3, x4, y1, y2, y3, y4) -> r1, r2, r3, r4 {
	// x - y = x + (~y + 1)
	let carry
	r4, carry := add_carry(x4, bit_negate(y4), 1)
	r3, carry := add_carry(x3, bit_negate(y3), carry)
	r2, carry := add_carry(x2, bit_negate(y2), carry)
	r1, carry := add_carry(x1, bit_negate(y1), carry)
}
function split(x) -> hi, lo {
	hi := i64.shr_u(x, 32)
	lo := i64.and(x, 0xffffffff)
}
// Multiplies two 64 bit values resulting in a 128 bit
// value split into two 64 bit values.
function mul_64x64_128(x, y) -> hi, lo {
	let xh, xl := split(x)
	let yh, yl := split(y)

	let t0 := i64.mul(xl, yl)
	let t1 := i64.mul(xh, yl)
	let t2 := i64.mul(xl, yh)
	let t3 := i64.mul(xh, yh)

	let t0h, t0l := split(t0)
	let u1 := i64.add(t1, t0h)
	let u1h, u1l := split(u1)
	let u2 := i64.add(t2, u1l)

	lo := i64.or(i64.shl(u2, 32), t0l)
	hi := i64.add(t3, i64.add(i64.shr_u(u2, 32), u1h))
}
// Multiplies two 128 bit values resulting in a 256 bit
// value split into four 64 bit values.
function mul_128x128_256(x1, x2, y1, y2) -> r1, r2, r3, r4 {
	let ah, al := mul_64x64_128(x1, y1)
	let     bh, bl := mul_64x64_128(x1, y2)
	let     ch, cl := mul_64x64_128(x2, y1)
	let         dh, dl := mul_64x64_128(x2, y2)

	r4 := dl

	let carry1, carry2
	let t1, t2

	r3, carry1 := add_carry(bl, cl, 0)
	r3, carry2 := add_carry(r3, dh, 0)

	t1, carry1 := add_carry(bh, ch, carry1)
	r2, carry2 := add_carry(t1, al, carry2)

	r1 := i64.add(i64.add(ah, carry1), carry2)
}
function mul(x1, x2, x3, x4, y1, y2, y3, y4) -> r1, r2, r3, r4 {
	// TODO it would actually suffice to have mul_128x128_128 for the first two.
	let b1, b2, b3, b4 := mul_128x128_256(x3, x4, y1, y2)
	let c1, c2, c3, c4 := mul_128x128_256(x1, x2, y3, y4)
	let         d1, d2, d3, d4 := mul_128x128_256(x3, x4, y3, y4)
	r4 := d4
	r3 := d3
	let t1, t2
	t1, t2, r1, r2 := add(0, 0, b3, b4, 0, 0, c3, c4)
	t1, t2, r1, r2 := add(0, 0, r1, r2, 0, 0, d1, d2)
}
function div(x1, x2, x3, x4, y1, y2, y3, y4) -> r1, r2, r3, r4 {
	// TODO implement properly
	r4 := i64.div_u(x4, y4)
}
function mod(x1, x2, x3, x4, y1, y2, y3, y4) -> r1, r2, r3, r4 {
	// TODO implement properly
	r4 := i64.rem_u(x4, y4)
}
function smod(x1, x2, x3, x4, y1, y2, y3, y4) -> r1, r2, r3, r4 {
	// TODO implement properly
	r4 := i64.rem_u(x4, y4)
}
function exp(x1, x2, x3, x4, y1, y2, y3, y4) -> r1, r2, r3, r4 {
	// TODO implement properly
	unreachable()
}

function byte(x1, x2, x3, x4, y1, y2, y3, y4) -> r1, r2, r3, r4 {
	if i64.eqz(i64.or(i64.or(x1, x2), x3)) {
		let component
		switch i64.div_u(x4, 8)
		case 0 { component := y1 }
		case 1 { component := y2 }
		case 2 { component := y3 }
		case 3 { component := y4 }
		x4 := i64.mul(i64.rem_u(x4, 8), 8)
		r4 := i64.shr_u(component, i64.sub(56, x4))
		r4 := i64.and(0xff, r4)
	}
}
function xor(x1, x2, x3, x4, y1, y2, y3, y4) -> r1, r2, r3, r4 {
	r1 := i64.xor(x1, y1)
	r2 := i64.xor(x2, y2)
	r3 := i64.xor(x3, y3)
	r4 := i64.xor(x4, y4)
}
function or(x1, x2, x3, x4, y1, y2, y3, y4) -> r1, r2, r3, r4 {
	r1 := i64.or(x1, y1)
	r2 := i64.or(x2, y2)
	r3 := i64.or(x3, y3)
	r4 := i64.or(x4, y4)
}
function and(x1, x2, x3, x4, y1, y2, y3, y4) -> r1, r2, r3, r4 {
	r1 := i64.and(x1, y1)
	r2 := i64.and(x2, y2)
	r3 := i64.and(x3, y3)
	r4 := i64.and(x4, y4)
}
function not(x1, x2, x3, x4) -> r1, r2, r3, r4 {
	let mask := 0xffffffffffffffff
	r1, r2, r3, r4 := xor(x1, x2, x3, x4, mask, mask, mask, mask)
}
function iszero(x1, x2, x3, x4) -> r1, r2, r3, r4 {
	r4 := i64.eqz(i64.or(i64.or(x1, x2), i64.or(x3, x4)))
}
function eq(x1, x2, x3, x4, y1, y2, y3, y4) -> r1, r2, r3, r4 {
	if i64.eq(x1, y1) {
		if i64.eq(x2, y2) {
			if i64.eq(x3, y3) {
				if i64.eq(x4, y4) {
					r4 := 1
				}
			}
		}
	}
}

// returns 0 if a == b, -1 if a < b and 1 if a > b
function cmp(a, b) -> r {
	switch i64.lt_u(a, b)
	case 1 { r := 0xffffffffffffffff }
	default {
		r := i64.ne(a, b)
	}
}

function lt(x1, x2, x3, x4, y1, y2, y3, y4) -> z1, z2, z3, z4 {
	switch cmp(x1, y1)
	case 0 {
		switch cmp(x2, y2)
		case 0 {
			switch cmp(x3, y3)
			case 0 {
				z4 := i64.lt_u(x4, y4)
			}
			case 1 { z4 := 0 }
			default { z4 := 1 }
		}
		case 1 { z4 := 0 }
		default { z4 := 1 }
	}
	case 1 { z4 := 0 }
	default { z4 := 1 }
}
function gt(x1, x2, x3, x4, y1, y2, y3, y4) -> z1, z2, z3, z4 {
	z1, z2, z3, z4 := lt(y1, y2, y3, y4, x1, x2, x3, x4)
}
function slt(x1, x2, x3, x4, y1, y2, y3, y4) -> z1, z2, z3, z4 {
	// TODO correct?
	x1 := i64.add(x1, 0x8000000000000000)
	y1 := i64.add(y1, 0x8000000000000000)
	z1, z2, z3, z4 := lt(x1, x2, x3, x4, y1, y2, y3, y4)
}
function sgt(x1, x2, x3, x4, y1, y2, y3, y4) -> z1, z2, z3, z4 {
	z1, z2, z3, z4 := slt(y1, y2, y3, y4, x1, x2, x3, x4)
}

function shl_single(a, amount) -> x, y {
	// amount < 64
	x := i64.shr_u(a, i64.sub(64, amount))
	y := i64.shl(a, amount)
}

function shl(x1, x2, x3, x4, y1, y2, y3, y4) -> z1, z2, z3, z4 {
	if i64.and(i64.eqz(x1), i64.eqz(x2)) {
		if i64.eqz(x3) {
			if i64.lt_u(x4, 256) {
				if i64.ge_u(x4, 128) {
					y1 := y3
					y2 := y4
					y3 := 0
					y4 := 0
					x4 := i64.sub(x4, 128)
				}
				if i64.ge_u(x4, 64) {
					y1 := y2
					y2 := y3
					y3 := y4
					y4 := 0
					x4 := i64.sub(x4, 64)
				}
				let t, r
				t, z4 := shl_single(y4, x4)
				r, z3 := shl_single(y3, x4)
				z3 := i64.or(z3, t)
				t, z2 := shl_single(y2, x4)
				z2 := i64.or(z2, r)
				r, z1 := shl_single(y1, x4)
				z1 := i64.or(z1, t)
			}
		}
	}
}

function shr_single(a, amount) -> x, y {
	// amount < 64
	y := i64.shl(a, i64.sub(64, amount))
	x := i64.shr_u(a, amount)
}

function shr(x1, x2, x3, x4, y1, y2, y3, y4) -> z1, z2, z3, z4 {
	if i64.and(i64.eqz(x1), i64.eqz(x2)) {
		if i64.eqz(x3) {
			if i64.lt_u(x4, 256) {
				if i64.ge_u(x4, 128) {
					y4 := y2
					y3 := y1
					y2 := 0
					y1 := 0
					x4 := i64.sub(x4, 128)
				}
				if i64.ge_u(x4, 64) {
					y4 := y3
					y3 := y2
					y2 := y1
					y1 := 0
					x4 := i64.sub(x4, 64)
				}
				let t
				z4, t := shr_single(y4, x4)
				z3, t := shr_single(y3, x4)
				z4 := i64.or(z4, t)
				z2, t := shr_single(y2, x4)
				z3 := i64.or(z3, t)
				z1, t := shr_single(y1, x4)
				z2 := i64.or(z2, t)
			}
		}
	}
}
function sar(x1, x2, x3, x4, y1, y2, y3, y4) -> z1, z2, z3, z4 {
	// TODO implement
	unreachable()
}
function addmod(x1, x2, x3, x4, y1, y2, y3, y4) -> z1, z2, z3, z4 {
	// TODO implement
	unreachable()
}
function mulmod(x1, x2, x3, x4, y1, y2, y3, y4) -> z1, z2, z3, z4 {
	// TODO implement
	unreachable()
}
function signextend(x1, x2, x3, x4, y1, y2, y3, y4) -> z1, z2, z3, z4 {
	// TODO implement
	unreachable()
}

function u256_to_i64(x1, x2, x3, x4) -> v {
	if i64.ne(0, i64.or(i64.or(x1, x2), x3)) { invalid() }
	v := x4
}

function u256_to_i32(x1, x2, x3, x4) -> v {
	if i64.ne(0, i64.or(i64.or(x1, x2), x3)) { invalid() }
	if i64.ne(0, i64.shr_u(x4, 32)) { invalid() }
	v := x4
}

function u256_to_i32ptr(x1, x2, x3, x4) -> v {
	v := u256_to_i32(x1, x2, x3, x4)
}

function keccak256(x1, x2, x3, x4, y1, y2, y3, y4) -> z1, z2, z3, z4 {
	// TODO implement
	unreachable()
}

function address() -> z1, z2, z3, z4 {
	eth.getAddress(0)
	z1, z2, z3, z4 := mload_internal(0)
}
function balance(x1, x2, x3, x4) -> z1, z2, z3, z4 {
	// TODO implement
	unreachable()
}
function origin() -> z1, z2, z3, z4 {
	eth.getTxOrigin(0)
	z1, z2, z3, z4 := mload_internal(0)
}
function caller() -> z1, z2, z3, z4 {
	eth.getCaller(0)
	z1, z2, z3, z4 := mload_internal(0)
}
function callvalue() -> z1, z2, z3, z4 {
	eth.getCallValue(0)
	z1, z2, z3, z4 := mload_internal(0)
}
function calldataload(x1, x2, x3, x4) -> z1, z2, z3, z4 {
	eth.callDataCopy(0, u256_to_i32(x1, x2, x3, x4), 32)
	z1, z2, z3, z4 := mload_internal(0)
}
function calldatasize() -> z1, z2, z3, z4 {
	z4 := eth.getCallDataSize()
}
function calldatacopy(x1, x2, x3, x4, y1, y2, y3, y4, z1, z2, z3, z4) {
	eth.callDataCopy(
		// scratch - TODO: overflow check
		i64.add(u256_to_i32ptr(x1, x2, x3, x4), 64),
		u256_to_i32(y1, y2, y3, y4),
		u256_to_i32(z1, z2, z3, z4)
	)
}

// Needed?
function codesize() -> z1, z2, z3, z4 {
	eth.getCodeSize(0)
	z1, z2, z3, z4 := mload_internal(0)
}
function codecopy(x1, x2, x3, x4, y1, y2, y3, y4, z1, z2, z3, z4) {
	eth.codeCopy(
		// scratch - TODO: overflow check
		i64.add(u256_to_i32ptr(x1, x2, x3, x4), 64),
		u256_to_i32(y1, y2, y3, y4),
		u256_to_i32(z1, z2, z3, z4)
	)
}
function datacopy(x1, x2, x3, x4, y1, y2, y3, y4, z1, z2, z3, z4) {
	// TODO correct?
	codecopy(x1, x2, x3, x4, y1, y2, y3, y4, z1, z2, z3, z4)
}

function gasprice() -> z1, z2, z3, z4 {
	eth.getTxGasPrice(0)
	z1, z2, z3, z4 := mload_internal(0)
}
function extcodesize(x1, x2, x3, x4) -> z1, z2, z3, z4 {
	// TODO implement
	unreachable()
}
function extcodehash(x1, x2, x3, x4) -> z1, z2, z3, z4 {
	// TODO implement
	unreachable()
}
function extcodecopy(x1, x2, x3, x4, y1, y2, y3, y4, z1, z2, z3, z4) {
	// TODO implement
	unreachable()
}

function returndatasize() -> z1, z2, z3, z4 {
	z4 := eth.getReturnDataSize()
}
function returndatacopy(x1, x2, x3, x4, y1, y2, y3, y4, z1, z2, z3, z4) {
	eth.returnDataCopy(
		// scratch - TODO: overflow check
		i64.add(u256_to_i32ptr(x1, x2, x3, x4), 64),
		u256_to_i32(y1, y2, y3, y4),
		u256_to_i32(z1, z2, z3, z4)
	)
}

function blockhash(x1, x2, x3, x4) -> z1, z2, z3, z4 {
	// TODO implement
	unreachable()
}
function coinbase() -> z1, z2, z3, z4 {
	// TODO implement
	unreachable()
}
function timestamp() -> z1, z2, z3, z4 {
	z4 := eth.getBlockTimestamp()
}
function number() -> z1, z2, z3, z4 {
	z4 := eth.getBlockNumber()
}
function difficulty() -> z1, z2, z3, z4 {
	eth.getBlockDifficulty(0)
	z1, z2, z3, z4 := mload_internal(0)
}
function gaslimit() -> z1, z2, z3, z4 {
	z4 := eth.getBlockGasLimit()
}

function pop(x1, x2, x3, x4) {
}


function endian_swap_16(x) -> y {
	let hi := i64.and(i64.shl(x, 8), 0xff00)
	let lo := i64.and(i64.shr_u(x, 8), 0xff)
	y := i64.or(hi, lo)
}

function endian_swap_32(x) -> y {
	let hi := i64.shl(endian_swap_16(x), 16)
	let lo := endian_swap_16(i64.shr_u(x, 16))
	y := i64.or(hi, lo)
}

function endian_swap(x) -> y {
	let hi := i64.shl(endian_swap_32(x), 32)
	let lo := endian_swap_32(i64.shr_u(x, 32))
	y := i64.or(hi, lo)
}
function save_temp_mem_32() -> t1, t2, t3, t4 {
	t1 := i64.load(0)
	t2 := i64.load(8)
	t3 := i64.load(16)
	t4 := i64.load(24)
}
function restore_temp_mem_32(t1, t2, t3, t4) {
	i64.store(0, t1)
	i64.store(8, t2)
	i64.store(16, t3)
	i64.store(24, t4)
}
function save_temp_mem_64() -> t1, t2, t3, t4, t5, t6, t7, t8 {
	t1 := i64.load(0)
	t2 := i64.load(8)
	t3 := i64.load(16)
	t4 := i64.load(24)
	t5 := i64.load(32)
	t6 := i64.load(40)
	t7 := i64.load(48)
	t8 := i64.load(54)
}
function restore_temp_mem_64(t1, t2, t3, t4, t5, t6, t7, t8) {
	i64.store(0, t1)
	i64.store(8, t2)
	i64.store(16, t3)
	i64.store(24, t4)
	i64.store(32, t5)
	i64.store(40, t6)
	i64.store(48, t7)
	i64.store(54, t8)
}
function mload(x1, x2, x3, x4) -> z1, z2, z3, z4 {
	let pos := u256_to_i32ptr(x1, x2, x3, x4)
	// Make room for the scratch space
	// TODO do we need to check for overflow?
	pos := i64.add(pos, 64)
	z1, z2, z3, z4 := mload_internal(pos)
}
function mload_internal(pos) -> z1, z2, z3, z4 {
	z1 := endian_swap(i64.load(pos))
	z2 := endian_swap(i64.load(i64.add(pos, 8)))
	z3 := endian_swap(i64.load(i64.add(pos, 16)))
	z4 := endian_swap(i64.load(i64.add(pos, 24)))
}
function mstore(x1, x2, x3, x4, y1, y2, y3, y4) {
	let pos := u256_to_i32ptr(x1, x2, x3, x4)
	// Make room for the scratch space
	// TODO do we need to check for overflow?
	pos := i64.add(pos, 64)
	mstore_internal(pos, y1, y2, y3, y4)
}
function mstore_internal(pos, y1, y2, y3, y4) {
	i64.store(pos, endian_swap(y1))
	i64.store(i64.add(pos, 8), endian_swap(y2))
	i64.store(i64.add(pos, 16), endian_swap(y3))
	i64.store(i64.add(pos, 24), endian_swap(y4))
}
function mstore8(x1, x2, x3, x4, y1, y2, y3, y4) {
	// TODO implement
	unreachable()
}
// Needed?
function msize() -> z1, z2, z3, z4 {
	// TODO implement
	unreachable()
}
function sload(x1, x2, x3, x4) -> z1, z2, z3, z4 {
	mstore_internal(0, x1, x2, x3, x4)
	eth.storageLoad(0, 32)
	z1, z2, z3, z4 := mload_internal(32)
}

function sstore(x1, x2, x3, x4, y1, y2, y3, y4) {
	mstore_internal(0, x1, x2, x3, x4)
	mstore_internal(32, y1, y2, y3, y4)
	eth.storageStore(0, 32)
}

// Needed?
function pc() -> z1, z2, z3, z4 {
	// TODO implement
	unreachable()
}
function gas() -> z1, z2, z3, z4 {
	z4 := eth.getGasLeft()
}

function log0(p1, p2, p3, p4, s1, s2, s3, s4) {
	// TODO implement
	unreachable()
}
function log1(
	p1, p2, p3, p4, s1, s2, s3, s4,
	t11, t12, t13, t14
) {
	// TODO implement
	unreachable()
}
function log2(
	p1, p2, p3, p4, s1, s2, s3, s4,
	t11, t12, t13, t14,
	t21, t22, t23, t24
) {
	// TODO implement
	unreachable()
}
function log3(
	p1, p2, p3, p4, s1, s2, s3, s4,
	t11, t12, t13, t14,
	t21, t22, t23, t24,
	t31, t32, t33, t34
) {
	// TODO implement
	unreachable()
}
function log4(
	p1, p2, p3, p4, s1, s2, s3, s4,
	t11, t12, t13, t14,
	t21, t22, t23, t24,
	t31, t32, t33, t34,
	t41, t42, t43, t44,
) {
	// TODO implement
	unreachable()
}

function create(x1, x2, x3, x4, y1, y2, y3, y4, z1, z2, z3, z4) -> a1, a2, a3, a4 {
	// TODO implement
	unreachable()
}
function call(
	a1, a2, a3, a4,
	b1, b2, b3, b4,
	c1, c2, c3, c4,
	d1, d2, d3, d4,
	e1, e2, e3, e4,
	f1, f2, f3, f4,
	g1, g2, g3, g4
) -> x1, x2, x3, x4 {
	// TODO implement
	unreachable()
}
function callcode(
	a1, a2, a3, a4,
	b1, b2, b3, b4,
	c1, c2, c3, c4,
	d1, d2, d3, d4,
	e1, e2, e3, e4,
	f1, f2, f3, f4,
	g1, g2, g3, g4
) -> x1, x2, x3, x4 {
	// TODO implement
	unreachable()
}
function delegatecall(
	a1, a2, a3, a4,
	b1, b2, b3, b4,
	c1, c2, c3, c4,
	d1, d2, d3, d4,
	e1, e2, e3, e4,
	f1, f2, f3, f4
) -> x1, x2, x3, x4 {
	// TODO implement
	unreachable()
}
function staticcall(
	a1, a2, a3, a4,
	b1, b2, b3, b4,
	c1, c2, c3, c4,
	d1, d2, d3, d4,
	e1, e2, e3, e4,
	f1, f2, f3, f4
) -> x1, x2, x3, x4 {
	// TODO implement
	unreachable()
}
function create2(
	a1, a2, a3, a4,
	b1, b2, b3, b4,
	c1, c2, c3, c4,
	d1, d2, d3, d4
) -> x1, x2, x3, x4 {
	// TODO implement
	unreachable()
}
function selfdestruct(a1, a2, a3, a4) {
	mstore(0, 0, 0, 0, a1, a2, a3, a4)
	// In EVM, addresses are padded to 32 bytes, so discard the first 12.
	eth.selfDestruct(12)
}

function return(x1, x2, x3, x4, y1, y2, y3, y4) {
	eth.finish(
		// scratch - TODO: overflow check
		i64.add(u256_to_i32ptr(x1, x2, x3, x4), 64),
		u256_to_i32(y1, y2, y3, y4)
	)
}
function revert(x1, x2, x3, x4, y1, y2, y3, y4) {
	eth.revert(
		// scratch - TODO: overflow check
		i64.add(u256_to_i32ptr(x1, x2, x3, x4), 64),
		u256_to_i32(y1, y2, y3, y4)
	)
}
function invalid() {
	unreachable()
}
})"};

}

Object EVMToEWasmTranslator::run(Object const& _object)
{
	if (!m_polyfill)
		parsePolyfill();

	Block ast = boost::get<Block>(Disambiguator(m_dialect, *_object.analysisInfo)(*_object.code));
	set<YulString> reservedIdentifiers;
	NameDispenser nameDispenser{m_dialect, ast, reservedIdentifiers};
	OptimiserStepContext context{m_dialect, nameDispenser, reservedIdentifiers};

	FunctionHoister::run(context, ast);
	FunctionGrouper::run(context, ast);
	MainFunction{}(ast);
	ExpressionSplitter::run(context, ast);
	WordSizeTransform::run(m_dialect, ast, nameDispenser);

	NameDisplacer{nameDispenser, m_polyfillFunctions}(ast);
	for (auto const& st: m_polyfill->statements)
		ast.statements.emplace_back(ASTCopier{}.translate(st));

	Object ret;
	ret.name = _object.name;
	ret.code = make_shared<Block>(move(ast));
	ret.analysisInfo = make_shared<AsmAnalysisInfo>();

	ErrorList errors;
	ErrorReporter errorReporter(errors);
	AsmAnalyzer analyzer(*ret.analysisInfo, errorReporter, std::nullopt, WasmDialect::instance(), {}, _object.dataNames());
	if (!analyzer.analyze(*ret.code))
	{
		// TODO the errors here are "wrong" because they have invalid source references!
		string message;
		for (auto const& err: errors)
			message += langutil::SourceReferenceFormatter::formatErrorInformation(*err);
		yulAssert(false, message);
	}

	for (auto const& subObjectNode: _object.subObjects)
		if (Object const* subObject = dynamic_cast<Object const*>(subObjectNode.get()))
			ret.subObjects.push_back(make_shared<Object>(run(*subObject)));
		else
			ret.subObjects.push_back(make_shared<Data>(dynamic_cast<Data const&>(*subObjectNode)));
	ret.subIndexByName = _object.subIndexByName;

	return ret;
}

void EVMToEWasmTranslator::parsePolyfill()
{
	ErrorList errors;
	ErrorReporter errorReporter(errors);
	shared_ptr<Scanner> scanner{make_shared<Scanner>(CharStream(polyfill, ""))};
	m_polyfill = Parser(errorReporter, WasmDialect::instance()).parse(scanner, false);
	if (!errors.empty())
	{
		string message;
		for (auto const& err: errors)
			message += langutil::SourceReferenceFormatter::formatErrorInformation(*err);
		yulAssert(false, message);
	}

	m_polyfillFunctions.clear();
	for (auto const& statement: m_polyfill->statements)
		m_polyfillFunctions.insert(boost::get<FunctionDefinition>(statement).name);
}

