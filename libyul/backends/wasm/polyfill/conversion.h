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
/**
 * This file includes bitwise functions for the wasm polyfill.
 */
#pragma once

#include <string>

namespace solidity::yul::wasm::polyfill
{

static std::string const conversion{R"(
// solidity::yul::wasm::polyfill::conversion

function u256_to_u128(x1, x2, x3, x4) -> v1, v2 {
	if i64.ne(0, i64.or(x1, x2)) { invalid() }
	v2 := x4
	v1 := x3
}

function u256_to_i64(x1, x2, x3, x4) -> v {
	if i64.ne(0, i64.or(i64.or(x1, x2), x3)) { invalid() }
	v := x4
}

function u256_to_i32(x1, x2, x3, x4) -> v:i32 {
	if i64.ne(0, i64.or(i64.or(x1, x2), x3)) { invalid() }
	if i64.ne(0, i64.shr_u(x4, 32)) { invalid() }
	v := i32.wrap_i64(x4)
}

function u256_to_byte(x1, x2, x3, x4) -> v {
	if i64.ne(0, i64.or(i64.or(x1, x2), x3)) { invalid() }
	if i64.gt_u(x4, 255) { invalid() }
	v := x4
}

function u256_to_i32ptr(x1, x2, x3, x4) -> v:i32 {
	v := u256_to_i32(x1, x2, x3, x4)
}

function to_internal_i32ptr(x1, x2, x3, x4) -> r:i32 {
	let p:i32 := u256_to_i32ptr(x1, x2, x3, x4)
	r := i32.add(p, 64:i32)
	if i32.lt_u(r, p) { invalid() }
}

function u256_to_address(x1, x2, x3, x4) -> r1, r2, r3 {
	if i64.ne(0, x1) { invalid() }
	if i64.ne(0, i64.shr_u(x2, 32)) { invalid() }
	r1 := x2
	r2 := x3
	r3 := x4
}

// signextend(i, x): sign extend from (i*8+7)th bit counting from least significant
function signextend(x1, x2, x3, x4, y1, y2, y3, y4) -> z1, z2, z3, z4 {
	z1 := y1
	z2 := y2
	z3 := y3
	z4 := y4
	if lt_256x256_64(x1, x2, x3, x4, 0, 0, 0, 32) {
		let d := i64.mul(i64.sub(31, x4), 8)
		z1, z2, z3, z4 := shl(0, 0, 0, d, z1, z2, z3, z4)
		z1, z2, z3, z4 := sar(0, 0, 0, d, z1, z2, z3, z4)
	}
}

// bswap16(x): reverse order of bytes
function bswap16(x:i32) -> y:i32 {
	let hi:i32 := i32.and(i32.shl(x, 8:i32), 0xff00:i32)
	let lo:i32 := i32.and(i32.shr_u(x, 8:i32), 0xff:i32)
	y := i32.or(hi, lo)
}

// bswap32(x): reverse order of bytes
function bswap32(x:i32) -> y:i32 {
	let hi:i32 := i32.shl(bswap16(x), 16:i32)
	let lo:i32 := bswap16(i32.shr_u(x, 16:i32))
	y := i32.or(hi, lo)
}

// bswap64(x): reverse order of bytes
//   e.g.: bswap64(0x0123456789abcdef) -> 0xefcdab8967452301
function bswap64(x) -> y {
	let hi := i64.shl(i64.extend_i32_u(bswap32(i32.wrap_i64(x))), 32)
	let lo := i64.extend_i32_u(bswap32(i32.wrap_i64(i64.shr_u(x, 32))))
	y := i64.or(hi, lo)
}

// byte(n, x): nth byte of x, where the most significant byte is the 0th byte
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
)"};

} // namespace solidity::yul::wasm::polyfill
