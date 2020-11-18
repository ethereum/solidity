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

// NOTE: This file is used to generate `ewasmPolyfills/Conversion.h`.

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

function bswap16(x) -> y {
	let hi := i64.and(i64.shl(x, 8), 0xff00)
	let lo := i64.and(i64.shr_u(x, 8), 0xff)
	y := i64.or(hi, lo)
}

function bswap32(x) -> y {
	let hi := i64.shl(bswap16(x), 16)
	let lo := bswap16(i64.shr_u(x, 16))
	y := i64.or(hi, lo)
}

function bswap64(x) -> y {
	let hi := i64.shl(bswap32(x), 32)
	let lo := bswap32(i64.shr_u(x, 32))
	y := i64.or(hi, lo)
}
