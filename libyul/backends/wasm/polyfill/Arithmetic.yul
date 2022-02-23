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

// NOTE: This file is used to generate `ewasmPolyfills/Arithmetic.h`.

// returns a + y + c plus carry value on 64 bit values.
// c should be at most 1
function add_carry(x, y, c) -> r, r_c {
	let t := i64.add(x, y)
	r := i64.add(t, c)
	r_c := i64.extend_i32_u(i32.or(
		i64.lt_u(t, x),
		i64.lt_u(r, t)
	))
}

function add(x1, x2, x3, x4, y1, y2, y3, y4) -> r1, r2, r3, r4 {
	let carry
	r4, carry := add_carry(x4, y4, 0)
	r3, carry := add_carry(x3, y3, carry)
	r2, carry := add_carry(x2, y2, carry)
	r1, carry := add_carry(x1, y1, carry)
}

function sub(x1, x2, x3, x4, y1, y2, y3, y4) -> r1, r2, r3, r4 {
	// x - y = x + (~y + 1)
	let carry
	r4, carry := add_carry(x4, bit_negate(y4), 1)
	r3, carry := add_carry(x3, bit_negate(y3), carry)
	r2, carry := add_carry(x2, bit_negate(y2), carry)
	r1, carry := add_carry(x1, bit_negate(y1), carry)
}

function sub320(x1, x2, x3, x4, x5, y1, y2, y3, y4, y5) -> r1, r2, r3, r4, r5 {
	// x - y = x + (~y + 1)
	let carry
	r5, carry := add_carry(x5, bit_negate(y5), 1)
	r4, carry := add_carry(x4, bit_negate(y4), carry)
	r3, carry := add_carry(x3, bit_negate(y3), carry)
	r2, carry := add_carry(x2, bit_negate(y2), carry)
	r1, carry := add_carry(x1, bit_negate(y1), carry)
}

function sub512(x1, x2, x3, x4, x5, x6, x7, x8, y1, y2, y3, y4, y5, y6, y7, y8) -> r1, r2, r3, r4, r5, r6, r7, r8 {
	// x - y = x + (~y + 1)
	let carry
	r8, carry := add_carry(x8, bit_negate(y8), 1)
	r7, carry := add_carry(x7, bit_negate(y7), carry)
	r6, carry := add_carry(x6, bit_negate(y6), carry)
	r5, carry := add_carry(x5, bit_negate(y5), carry)
	r4, carry := add_carry(x4, bit_negate(y4), carry)
	r3, carry := add_carry(x3, bit_negate(y3), carry)
	r2, carry := add_carry(x2, bit_negate(y2), carry)
	r1, carry := add_carry(x1, bit_negate(y1), carry)
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
	let bh, bl := mul_64x64_128(x1, y2)
	let ch, cl := mul_64x64_128(x2, y1)
	let dh, dl := mul_64x64_128(x2, y2)
	r4 := dl
	let carry1, carry2
	let t1, t2
	r3, carry1 := add_carry(bl, cl, 0)
	r3, carry2 := add_carry(r3, dh, 0)
	t1, carry1 := add_carry(bh, ch, carry1)
	r2, carry2 := add_carry(t1, al, carry2)
	r1 := i64.add(i64.add(ah, carry1), carry2)
}

// Multiplies two 256 bit values resulting in a 512 bit
// value split into eight 64 bit values.
function mul_256x256_512(x1, x2, x3, x4, y1, y2, y3, y4) -> r1, r2, r3, r4, r5, r6, r7, r8 {
	let a1, a2, a3, a4 := mul_128x128_256(x1, x2, y1, y2)
	let b1, b2, b3, b4 := mul_128x128_256(x1, x2, y3, y4)
	let c1, c2, c3, c4 := mul_128x128_256(x3, x4, y1, y2)
	let d1, d2, d3, d4 := mul_128x128_256(x3, x4, y3, y4)
	r8 := d4
	r7 := d3
	let carry1, carry2
	let t1, t2
	r6, carry1 := add_carry(b4, c4, 0)
	r6, carry2 := add_carry(r6, d2, 0)
	r5, carry1 := add_carry(b3, c3, carry1)
	r5, carry2 := add_carry(r5, d1, carry2)
	r4, carry1 := add_carry(a4, b2, carry1)
	r4, carry2 := add_carry(r4, c2, carry2)
	r3, carry1 := add_carry(a3, b1, carry1)
	r3, carry2 := add_carry(r3, c1, carry2)
	r2, carry1 := add_carry(a2, carry1, carry2)
	r1 := i64.add(a1, carry1)
}

function mul(x1, x2, x3, x4, y1, y2, y3, y4) -> r1, r2, r3, r4 {
	// TODO it would actually suffice to have mul_128x128_128 for the first two.
	let b1, b2, b3, b4 := mul_128x128_256(x3, x4, y1, y2)
	let c1, c2, c3, c4 := mul_128x128_256(x1, x2, y3, y4)
	let d1, d2, d3, d4 := mul_128x128_256(x3, x4, y3, y4)
	r4 := d4
	r3 := d3
	let t1, t2
	t1, t2, r1, r2 := add(0, 0, b3, b4, 0, 0, c3, c4)
	t1, t2, r1, r2 := add(0, 0, r1, r2, 0, 0, d1, d2)
}

function div(x1, x2, x3, x4, y1, y2, y3, y4) -> r1, r2, r3, r4 {
	// Based on https://github.com/ewasm/evm2wasm/blob/master/wasm/DIV.wast
	if iszero256(y1, y2, y3, y4) {
		leave
	}

	let m1 := 0
	let m2 := 0
	let m3 := 0
	let m4 := 1

	for {} true {} {
		if i32.or(i64.eqz(i64.clz(y1)), gte_256x256_64(y1, y2, y3, y4, x1, x2, x3, x4)) {
			break
		}
		y1, y2, y3, y4 := shl_internal(1, y1, y2, y3, y4)
		m1, m2, m3, m4 := shl_internal(1, m1, m2, m3, m4)
	}

	for {} or_bool(m1, m2, m3, m4) {} {
		if gte_256x256_64(x1, x2, x3, x4, y1, y2, y3, y4) {
			x1, x2, x3, x4 := sub(x1, x2, x3, x4, y1, y2, y3, y4)
			r1, r2, r3, r4 := add(r1, r2, r3, r4, m1, m2, m3, m4)
		}
		y1, y2, y3, y4 := shr_internal(1, y1, y2, y3, y4)
		m1, m2, m3, m4 := shr_internal(1, m1, m2, m3, m4)
	}
}

function sdiv(x1, x2, x3, x4, y1, y2, y3, y4) -> r1, r2, r3, r4 {
	// Based on https://github.com/ewasm/evm2wasm/blob/master/wasm/SDIV.wast

	let sign:i32 := i32.wrap_i64(i64.shr_u(i64.xor(x1, y1), 63))

	if i64.eqz(i64.clz(x1)) {
		x1, x2, x3, x4 := xor(
			x1, x2, x3, x4,
			0xffffffffffffffff, 0xffffffffffffffff, 0xffffffffffffffff, 0xffffffffffffffff
		)
		x1, x2, x3, x4 := add(x1, x2, x3, x4, 0, 0, 0, 1)
	}

	if i64.eqz(i64.clz(y1)) {
		y1, y2, y3, y4 := xor(
			y1, y2, y3, y4,
			0xffffffffffffffff, 0xffffffffffffffff, 0xffffffffffffffff, 0xffffffffffffffff
		)
		y1, y2, y3, y4 := add(y1, y2, y3, y4, 0, 0, 0, 1)
	}

	r1, r2, r3, r4 := div(x1, x2, x3, x4, y1, y2, y3, y4)

	if sign {
		r1, r2, r3, r4 := xor(
			r1, r2, r3, r4,
			0xffffffffffffffff, 0xffffffffffffffff, 0xffffffffffffffff, 0xffffffffffffffff
		)
		r1, r2, r3, r4 := add(r1, r2, r3, r4, 0, 0, 0, 1)
	}
}

function mod(x1, x2, x3, x4, y1, y2, y3, y4) -> r1, r2, r3, r4 {
 	// Based on https://github.com/ewasm/evm2wasm/blob/master/wasm/MOD.wast
	if iszero256(y1, y2, y3, y4) {
		leave
	}

	r1 := x1
	r2 := x2
	r3 := x3
	r4 := x4

	let m1 := 0
	let m2 := 0
	let m3 := 0
	let m4 := 1

	for {} true {} {
		if i32.or(i64.eqz(i64.clz(y1)), gte_256x256_64(y1, y2, y3, y4, r1, r2, r3, r4)) {
			break
		}

		y1, y2, y3, y4 := shl_internal(1, y1, y2, y3, y4)
		m1, m2, m3, m4 := shl_internal(1, m1, m2, m3, m4)
	}

	for {} or_bool(m1, m2, m3, m4) {} {
		if gte_256x256_64(r1, r2, r3, r4, y1, y2, y3, y4) {
			r1, r2, r3, r4 := sub(r1, r2, r3, r4, y1, y2, y3, y4)
		}

		y1, y2, y3, y4 := shr_internal(1, y1, y2, y3, y4)
		m1, m2, m3, m4 := shr_internal(1, m1, m2, m3, m4)
	}
}

function mod320(x1, x2, x3, x4, x5, y1, y2, y3, y4, y5) -> r1, r2, r3, r4, r5 {
	// Based on https://github.com/ewasm/evm2wasm/blob/master/wasm/mod_320.wast
	if iszero320(y1, y2, y3, y4, y5) {
		leave
	}

	let m1 := 0
	let m2 := 0
	let m3 := 0
	let m4 := 0
	let m5 := 1

	r1 := x1
	r2 := x2
	r3 := x3
	r4 := x4
	r5 := x5

	for {} true {} {
		if i32.or(i64.eqz(i64.clz(y1)), gte_320x320_64(y1, y2, y3, y4, y5, r1, r2, r3, r4, r5)) {
			break
		}
		y1, y2, y3, y4, y5 := shl320_internal(1, y1, y2, y3, y4, y5)
		m1, m2, m3, m4, m5 := shl320_internal(1, m1, m2, m3, m4, m5)
	}

	for {} or_bool_320(m1, m2, m3, m4, m5) {} {
		if gte_320x320_64(r1, r2, r3, r4, r5, y1, y2, y3, y4, y5) {
			r1, r2, r3, r4, r5 := sub320(r1, r2, r3, r4, r5, y1, y2, y3, y4, y5)
		}

		y1, y2, y3, y4, y5 := shr320_internal(1, y1, y2, y3, y4, y5)
		m1, m2, m3, m4, m5 := shr320_internal(1, m1, m2, m3, m4, m5)
	}
}

function mod512(x1, x2, x3, x4, x5, x6, x7, x8, y1, y2, y3, y4, y5, y6, y7, y8) -> r1, r2, r3, r4, r5, r6, r7, r8 {
	// Based on https://github.com/ewasm/evm2wasm/blob/master/wasm/mod_512.wast
	if iszero512(y1, y2, y3, y4, y5, y6, y7, y8) {
		leave
	}

	let m1 := 0
	let m2 := 0
	let m3 := 0
	let m4 := 0
	let m5 := 0
	let m6 := 0
	let m7 := 0
	let m8 := 1

	r1 := x1
	r2 := x2
	r3 := x3
	r4 := x4
	r5 := x5
	r6 := x6
	r7 := x7
	r8 := x8

	for {} true {} {
		if i32.or(
				i64.eqz(i64.clz(y1)),
				gte_512x512_64(y1, y2, y3, y4, y5, y6, y7, y8, r1, r2, r3, r4, r5, r6, r7, r8)
			)
		{
			break
		}
		y1, y2, y3, y4, y5, y6, y7, y8 := shl512_internal(1, y1, y2, y3, y4, y5, y6, y7, y8)
		m1, m2, m3, m4, m5, m6, m7, m8 := shl512_internal(1, m1, m2, m3, m4, m5, m6, m7, m8)
	}

	for {} or_bool_512(m1, m2, m3, m4, m5, m6, m7, m8) {} {
		if gte_512x512_64(r1, r2, r3, r4, r5, r6, r7, r8, y1, y2, y3, y4, y5, y6, y7, y8) {
			r1, r2, r3, r4, r5, r6, r7, r8 := sub512(r1, r2, r3, r4, r5, r6, r7, r8, y1, y2, y3, y4, y5, y6, y7, y8)
		}

		y1, y2, y3, y4, y5, y6, y7, y8 := shr512_internal(1, y1, y2, y3, y4, y5, y6, y7, y8)
		m1, m2, m3, m4, m5, m6, m7, m8 := shr512_internal(1, m1, m2, m3, m4, m5, m6, m7, m8)
	}
}

function smod(x1, x2, x3, x4, y1, y2, y3, y4) -> r1, r2, r3, r4 {
	// Based on https://github.com/ewasm/evm2wasm/blob/master/wasm/SMOD.wast
	let m1 := 0
	let m2 := 0
	let m3 := 0
	let m4 := 1

	let sign:i32 := i32.wrap_i64(i64.shr_u(x1, 63))

	if i64.eqz(i64.clz(x1)) {
		x1, x2, x3, x4 := xor(
			x1, x2, x3, x4,
			0xffffffffffffffff, 0xffffffffffffffff, 0xffffffffffffffff, 0xffffffffffffffff
		)
		x1, x2, x3, x4 := add(x1, x2, x3, x4, 0, 0, 0, 1)
	}

	if i64.eqz(i64.clz(y1)) {
		y1, y2, y3, y4 := xor(
			y1, y2, y3, y4,
			0xffffffffffffffff, 0xffffffffffffffff, 0xffffffffffffffff, 0xffffffffffffffff
		)
		y1, y2, y3, y4 := add(y1, y2, y3, y4, 0, 0, 0, 1)
	}

	r1, r2, r3, r4 := mod(x1, x2, x3, x4, y1, y2, y3, y4)

	if sign {
		r1, r2, r3, r4 := xor(
			r1, r2, r3, r4,
			0xffffffffffffffff, 0xffffffffffffffff, 0xffffffffffffffff, 0xffffffffffffffff
		)
		r1, r2, r3, r4 := add(r1, r2, r3, r4, 0, 0, 0, 1)
	}
}

function exp(x1, x2, x3, x4, y1, y2, y3, y4) -> r1, r2, r3, r4 {
	r4 := 1
	for {} or_bool(y1, y2, y3, y4) {} {
		if i32.wrap_i64(i64.and(y4, 1)) {
			r1, r2, r3, r4 := mul(r1, r2, r3, r4, x1, x2, x3, x4)
		}
		x1, x2, x3, x4 := mul(x1, x2, x3, x4, x1, x2, x3, x4)
		y1, y2, y3, y4 := shr_internal(1, y1, y2, y3, y4)
	}
}

function addmod(x1, x2, x3, x4, y1, y2, y3, y4, m1, m2, m3, m4) -> z1, z2, z3, z4 {
	let carry
	z4, carry := add_carry(x4, y4, 0)
	z3, carry := add_carry(x3, y3, carry)
	z2, carry := add_carry(x2, y2, carry)
	z1, carry := add_carry(x1, y1, carry)

	let z0
	z0, z1, z2, z3, z4 := mod320(carry, z1, z2, z3, z4, 0, m1, m2, m3, m4)
}

function mulmod(x1, x2, x3, x4, y1, y2, y3, y4, m1, m2, m3, m4) -> z1, z2, z3, z4 {
	let r1, r2, r3, r4, r5, r6, r7, r8 := mul_256x256_512(x1, x2, x3, x4, y1, y2, y3, y4)
	let t1
	let t2
	let t3
	let t4
	t1, t2, t3, t4, z1, z2, z3, z4 := mod512(r1, r2, r3, r4, r5, r6, r7, r8, 0, 0, 0, 0, m1, m2, m3, m4)
}

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
