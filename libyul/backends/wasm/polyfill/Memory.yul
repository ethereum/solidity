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

// NOTE: This file is used to generate `ewasmPolyfills/Memory.h`.

function save_temp_mem_32() -> t1, t2, t3, t4 {
	t1 := i64.load(0:i32)
	t2 := i64.load(8:i32)
	t3 := i64.load(16:i32)
	t4 := i64.load(24:i32)
}

function restore_temp_mem_32(t1, t2, t3, t4) {
	i64.store(0:i32, t1)
	i64.store(8:i32, t2)
	i64.store(16:i32, t3)
	i64.store(24:i32, t4)
}

function save_temp_mem_64() -> t1, t2, t3, t4, t5, t6, t7, t8 {
	t1 := i64.load(0:i32)
	t2 := i64.load(8:i32)
	t3 := i64.load(16:i32)
	t4 := i64.load(24:i32)
	t5 := i64.load(32:i32)
	t6 := i64.load(40:i32)
	t7 := i64.load(48:i32)
	t8 := i64.load(54:i32)
}

function restore_temp_mem_64(t1, t2, t3, t4, t5, t6, t7, t8) {
	i64.store(0:i32, t1)
	i64.store(8:i32, t2)
	i64.store(16:i32, t3)
	i64.store(24:i32, t4)
	i64.store(32:i32, t5)
	i64.store(40:i32, t6)
	i64.store(48:i32, t7)
	i64.store(54:i32, t8)
}

function pop(x1, x2, x3, x4) {
}

function memoryguard(x:i64) -> y1, y2, y3, y4 {
	y4 := x
}

// Fill `length` bytes starting from `ptr` with `value`.
function memset(ptr:i32, value:i32, length:i32) {
	for { let i:i32 := 0:i32 } i32.lt_u(i, length) { i := i32.add(i, 1:i32) }
	{
		i32.store8(i32.add(ptr, i), value)
	}
}

// Writes 256-bits from `pos`, but only set the bottom 160-bits.
function mstore_address(pos:i32, a1, a2, a3, a4) {
	a1, a2, a3 := u256_to_address(a1, a2, a3, a4)
	i64.store(pos, 0:i64)
	i32.store(i32.add(pos, 8:i32), 0:i32)
	i32.store(i32.add(pos, 12:i32), bswap32(i32.wrap_i64(a1)))
	i64.store(i32.add(pos, 16:i32), bswap64(a2))
	i64.store(i32.add(pos, 24:i32), bswap64(a3))
}

// Reads 256-bits from `pos`, but only returns the bottom 160-bits.
function mload_address(pos:i32) -> z1, z2, z3, z4 {
	z2 := i64.extend_i32_u(bswap32(i32.load(i32.add(pos, 12:i32))))
	z3 := bswap64(i64.load(i32.add(pos, 16:i32)))
	z4 := bswap64(i64.load(i32.add(pos, 24:i32)))
}

// Writes 256-bits from `pos`.
function mstore_internal(pos:i32, y1, y2, y3, y4) {
	i64.store(pos, bswap64(y1))
	i64.store(i32.add(pos, 8:i32), bswap64(y2))
	i64.store(i32.add(pos, 16:i32), bswap64(y3))
	i64.store(i32.add(pos, 24:i32), bswap64(y4))
}

// Reads 256-bits from `pos`.
function mload_internal(pos:i32) -> z1, z2, z3, z4 {
	z1 := bswap64(i64.load(pos))
	z2 := bswap64(i64.load(i32.add(pos, 8:i32)))
	z3 := bswap64(i64.load(i32.add(pos, 16:i32)))
	z4 := bswap64(i64.load(i32.add(pos, 24:i32)))
}

// Stores one byte at position `x` of value `y`.
function mstore8(x1, x2, x3, x4, y1, y2, y3, y4) {
	let v := u256_to_byte(y1, y2, y3, y4)
	i64.store8(to_internal_i32ptr(x1, x2, x3, x4), v)
}
