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

// NOTE: This file is used to generate `ewasmPolyfills/Bitwise.h`.

function bit_negate(x) -> y {
	y := i64.xor(x, 0xffffffffffffffff)
}

function split(x) -> hi, lo {
	hi := i64.shr_u(x, 32)
	lo := i64.and(x, 0xffffffff)
}

function shl_internal(amount, x1, x2, x3, x4) -> r1, r2, r3, r4 {
	// amount < 64
	r1 := i64.add(i64.shl(x1, amount), i64.shr_u(x2, i64.sub(64, amount)))
	r2 := i64.add(i64.shl(x2, amount), i64.shr_u(x3, i64.sub(64, amount)))
	r3 := i64.add(i64.shl(x3, amount), i64.shr_u(x4, i64.sub(64, amount)))
	r4 := i64.shl(x4, amount)
}

function shr_internal(amount, x1, x2, x3, x4) -> r1, r2, r3, r4 {
	// amount < 64
	r4 := i64.add(i64.shr_u(x4, amount), i64.shl(x3, i64.sub(64, amount)))
	r3 := i64.add(i64.shr_u(x3, amount), i64.shl(x2, i64.sub(64, amount)))
	r2 := i64.add(i64.shr_u(x2, amount), i64.shl(x1, i64.sub(64, amount)))
	r1 := i64.shr_u(x1, amount)
}

function shl320_internal(amount, x1, x2, x3, x4, x5) -> r1, r2, r3, r4, r5 {
	// amount < 64
	r1 := i64.add(i64.shl(x1, amount), i64.shr_u(x2, i64.sub(64, amount)))
	r2 := i64.add(i64.shl(x2, amount), i64.shr_u(x3, i64.sub(64, amount)))
	r3 := i64.add(i64.shl(x3, amount), i64.shr_u(x4, i64.sub(64, amount)))
	r4 := i64.add(i64.shl(x4, amount), i64.shr_u(x5, i64.sub(64, amount)))
	r5 := i64.shl(x5, 1)
}

function shr320_internal(amount, x1, x2, x3, x4, x5) -> r1, r2, r3, r4, r5 {
	// amount < 64
	r5 := i64.add(i64.shr_u(x5, amount), i64.shl(x4, i64.sub(64, amount)))
	r4 := i64.add(i64.shr_u(x4, amount), i64.shl(x3, i64.sub(64, amount)))
	r3 := i64.add(i64.shr_u(x3, amount), i64.shl(x2, i64.sub(64, amount)))
	r2 := i64.add(i64.shr_u(x2, amount), i64.shl(x1, i64.sub(64, amount)))
	r1 := i64.shr_u(x1, 1)
}

function shl512_internal(amount, x1, x2, x3, x4, x5, x6, x7, x8) -> r1, r2, r3, r4, r5, r6, r7, r8 {
	// amount < 64
	r1 := i64.add(i64.shl(x1, amount), i64.shr_u(x2, i64.sub(64, amount)))
	r2 := i64.add(i64.shl(x2, amount), i64.shr_u(x3, i64.sub(64, amount)))
	r3 := i64.add(i64.shl(x3, amount), i64.shr_u(x4, i64.sub(64, amount)))
	r4 := i64.add(i64.shl(x4, amount), i64.shr_u(x5, i64.sub(64, amount)))
	r5 := i64.add(i64.shl(x5, amount), i64.shr_u(x6, i64.sub(64, amount)))
	r6 := i64.add(i64.shl(x6, amount), i64.shr_u(x7, i64.sub(64, amount)))
	r7 := i64.add(i64.shl(x7, amount), i64.shr_u(x8, i64.sub(64, amount)))
	r8 := i64.shl(x8, amount)
}

function shr512_internal(amount, x1, x2, x3, x4, x5, x6, x7, x8) -> r1, r2, r3, r4, r5, r6, r7, r8 {
	// amount < 64
	r8 := i64.add(i64.shr_u(x8, amount), i64.shl(x7, i64.sub(64, amount)))
	r7 := i64.add(i64.shr_u(x7, amount), i64.shl(x6, i64.sub(64, amount)))
	r6 := i64.add(i64.shr_u(x6, amount), i64.shl(x5, i64.sub(64, amount)))
	r5 := i64.add(i64.shr_u(x5, amount), i64.shl(x4, i64.sub(64, amount)))
	r4 := i64.add(i64.shr_u(x4, amount), i64.shl(x3, i64.sub(64, amount)))
	r3 := i64.add(i64.shr_u(x3, amount), i64.shl(x2, i64.sub(64, amount)))
	r2 := i64.add(i64.shr_u(x2, amount), i64.shl(x1, i64.sub(64, amount)))
	r1 := i64.shr_u(x1, amount)
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

function shl_single(a, amount) -> x, y {
	// amount < 64
	x := i64.shr_u(a, i64.sub(64, amount))
	y := i64.shl(a, amount)
}

function shl(x1, x2, x3, x4, y1, y2, y3, y4) -> z1, z2, z3, z4 {
	if i32.and(i64.eqz(x1), i64.eqz(x2)) {
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
	if i32.and(i64.eqz(x1), i64.eqz(x2)) {
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
	if i64.gt_u(i64.clz(y1), 0) {
		z1, z2, z3, z4 := shr(x1, x2, x3, x4, y1, y2, y3, y4)
		leave
	}

	if gte_256x256_64(x1, x2, x3, x4, 0, 0, 0, 256) {
		z1 := 0xffffffffffffffff
		z2 := 0xffffffffffffffff
		z3 := 0xffffffffffffffff
		z4 := 0xffffffffffffffff
	}
	if lt_256x256_64(x1, x2, x3, x4, 0, 0, 0, 256) {
		y1, y2, y3, y4 := shr(0, 0, 0, x4, y1, y2, y3, y4)
		z1, z2, z3, z4 := shl(
			0, 0, 0, i64.sub(256, x4),
			0xffffffffffffffff, 0xffffffffffffffff, 0xffffffffffffffff, 0xffffffffffffffff
		)
		z1, z2, z3, z4 := or(y1, y2, y3, y4, z1, z2, z3, z4)
	}
}
