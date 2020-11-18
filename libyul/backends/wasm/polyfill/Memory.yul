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

function memset(ptr:i32, value:i32, length:i32) {
	for { let i:i32 := 0:i32 } i32.lt_u(i, length) { i := i32.add(i, 1:i32) }
	{
		i32.store8(i32.add(ptr, i), value)
	}
}