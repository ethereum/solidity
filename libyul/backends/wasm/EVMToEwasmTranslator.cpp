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
 * Translates Yul code from EVM dialect to Ewasm dialect.
 */

#include <libyul/backends/wasm/EVMToEwasmTranslator.h>

#include <libyul/backends/wasm/WordSizeTransform.h>
#include <libyul/backends/wasm/WasmDialect.h>
#include <libyul/optimiser/ExpressionSplitter.h>
#include <libyul/optimiser/FunctionGrouper.h>
#include <libyul/optimiser/MainFunction.h>
#include <libyul/optimiser/FunctionHoister.h>
#include <libyul/optimiser/Disambiguator.h>
#include <libyul/optimiser/NameDisplacer.h>
#include <libyul/optimiser/OptimiserStep.h>
#include <libyul/optimiser/ForLoopConditionIntoBody.h>

#include <libyul/AsmParser.h>
#include <libyul/AsmAnalysis.h>
#include <libyul/AsmAnalysisInfo.h>
#include <libyul/Object.h>

#include <liblangutil/ErrorReporter.h>
#include <liblangutil/Scanner.h>
#include <liblangutil/SourceReferenceFormatter.h>

using namespace std;
using namespace solidity;
using namespace solidity::yul;
using namespace solidity::util;
using namespace solidity::langutil;

namespace
{
static string const polyfill{R"(
{
function or_bool(a, b, c, d) -> r:i32 {
	r := i32.eqz(i64.eqz(i64.or(i64.or(a, b), i64.or(c, d))))
}
function or_bool_320(a, b, c, d, e) -> r:i32 {
	r := i32.or(or_bool(a, b, c, 0), or_bool(d, e, 0, 0))
}
function or_bool_512(a, b, c, d, e, f, g, h) -> r:i32 {
	r := i32.or(or_bool(a, b, c, d), or_bool(e, f, g, h))
}
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
	r4 := i64.extend_i32_u(iszero256(x1, x2, x3, x4))
}
function iszero256(x1, x2, x3, x4) -> r:i32 {
	r := i64.eqz(i64.or(i64.or(x1, x2), i64.or(x3, x4)))
}
function iszero320(x1, x2, x3, x4, x5) -> r:i32 {
	r := i64.eqz(i64.or(i64.or(i64.or(x1, x2), i64.or(x3, x4)), x5))
}
function iszero512(x1, x2, x3, x4, x5, x6, x7, x8) -> r:i32 {
	r := i32.and(iszero256(x1, x2, x3, x4), iszero256(x5, x6, x7, x8))
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
function cmp(a, b) -> r:i32 {
	switch i64.lt_u(a, b)
	case 1:i32 { r := 0xffffffff:i32 }
	default {
		r := i64.ne(a, b)
	}
}
function lt_320x320_64(x1, x2, x3, x4, x5, y1, y2, y3, y4, y5) -> z:i32 {
	switch cmp(x1, y1)
	case 0:i32 {
		switch cmp(x2, y2)
		case 0:i32 {
			switch cmp(x3, y3)
			case 0:i32 {
				switch cmp(x4, y4)
				case 0:i32 {
					z := i64.lt_u(x5, y5)
				}
				case 1:i32 { z := 0:i32 }
				default { z := 1:i32 }
			}
			case 1:i32 { z := 0:i32 }
			default { z := 1:i32 }
		}
		case 1:i32 { z := 0:i32 }
		default { z := 1:i32 }
	}
	case 1:i32 { z := 0:i32 }
	default { z := 1:i32 }
}
function lt_512x512_64(x1, x2, x3, x4, x5, x6, x7, x8, y1, y2, y3, y4, y5, y6, y7, y8) -> z:i32 {
	switch cmp(x1, y1)
	case 0:i32 {
		switch cmp(x2, y2)
		case 0:i32 {
			switch cmp(x3, y3)
			case 0:i32 {
				switch cmp(x4, y4)
				case 0:i32 {
					switch cmp(x5, y5)
					case 0:i32 {
						switch cmp(x6, y6)
						case 0:i32 {
							switch cmp(x7, y7)
							case 0:i32 {
								z := i64.lt_u(x8, y8)
							}
							case 1:i32 { z := 0:i32 }
							default { z := 1:i32 }
						}
						case 1:i32 { z := 0:i32 }
						default { z := 1:i32 }
					}
					case 1:i32 { z := 0:i32 }
					default { z := 1:i32 }
				}
				case 1:i32 { z := 0:i32 }
				default { z := 1:i32 }
			}
			case 1:i32 { z := 0:i32 }
			default { z := 1:i32 }
		}
		case 1:i32 { z := 0:i32 }
		default { z := 1:i32 }
	}
	case 1:i32 { z := 0:i32 }
	default { z := 1:i32 }
}/*
)"
// Split long string to make it compilable on msvc
// https://docs.microsoft.com/en-us/cpp/error-messages/compiler-errors-1/compiler-error-c2026?view=vs-2019
R"(
*/
function lt_256x256_64(x1, x2, x3, x4, y1, y2, y3, y4) -> z:i32 {
	switch cmp(x1, y1)
	case 0:i32 {
		switch cmp(x2, y2)
		case 0:i32 {
			switch cmp(x3, y3)
			case 0:i32 {
				z := i64.lt_u(x4, y4)
			}
			case 1:i32 { z := 0:i32 }
			default { z := 1:i32 }
		}
		case 1:i32 { z := 0:i32 }
		default { z := 1:i32 }
	}
	case 1:i32 { z := 0:i32 }
	default { z := 1:i32 }
}
function lt(x1, x2, x3, x4, y1, y2, y3, y4) -> z1, z2, z3, z4 {
	z4 := i64.extend_i32_u(lt_256x256_64(x1, x2, x3, x4, y1, y2, y3, y4))
}
function gte_256x256_64(x1, x2, x3, x4, y1, y2, y3, y4) -> z:i32 {
	z := i32.eqz(lt_256x256_64(x1, x2, x3, x4, y1, y2, y3, y4))
}
function gte_320x320_64(x1, x2, x3, x4, x5, y1, y2, y3, y4, y5) -> z:i32 {
	z := i32.eqz(lt_320x320_64(x1, x2, x3, x4, x5, y1, y2, y3, y4, y5))
}
function gte_512x512_64(x1, x2, x3, x4, x5, x6, x7, x8, y1, y2, y3, y4, y5, y6, y7, y8) -> z:i32 {
	z := i32.eqz(lt_512x512_64(x1, x2, x3, x4, x5, x6, x7, x8, y1, y2, y3, y4, y5, y6, y7, y8))
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

function keccak256(x1, x2, x3, x4, y1, y2, y3, y4) -> z1, z2, z3, z4 {
	// TODO implement
	unreachable()
}

function address() -> z1, z2, z3, z4 {
	eth.getAddress(0:i32)
	z1, z2, z3, z4 := mload_internal(0:i32)
}
function balance(x1, x2, x3, x4) -> z1, z2, z3, z4 {
	mstore_address(0:i32, x1, x2, x3, x4)
	eth.getExternalBalance(12:i32, 48:i32)
	z1, z2, z3, z4 := mload_internal(32:i32)
}
function selfbalance() -> z1, z2, z3, z4 {
	// TODO: not part of current Ewasm spec
	unreachable()
}
function chainid() -> z1, z2, z3, z4 {
	// TODO: not part of current Ewasm spec
	unreachable()
}
function origin() -> z1, z2, z3, z4 {
	eth.getTxOrigin(0:i32)
	z1, z2, z3, z4 := mload_internal(0:i32)
}
function caller() -> z1, z2, z3, z4 {
	eth.getCaller(0:i32)
	z1, z2, z3, z4 := mload_internal(0:i32)
}
function callvalue() -> z1, z2, z3, z4 {
	eth.getCallValue(0:i32)
	z1, z2, z3, z4 := mload_internal(0:i32)
}
function calldataload(x1, x2, x3, x4) -> z1, z2, z3, z4 {
	eth.callDataCopy(0:i32, u256_to_i32(x1, x2, x3, x4), 32:i32)
	z1, z2, z3, z4 := mload_internal(0:i32)
}
function calldatasize() -> z1, z2, z3, z4 {
	z4 := i64.extend_i32_u(eth.getCallDataSize())
}
function calldatacopy(x1, x2, x3, x4, y1, y2, y3, y4, z1, z2, z3, z4) {
	eth.callDataCopy(
		to_internal_i32ptr(x1, x2, x3, x4),
		u256_to_i32(y1, y2, y3, y4),
		u256_to_i32(z1, z2, z3, z4)
	)
}

// Needed?
function codesize() -> z1, z2, z3, z4 {
	z4 := i64.extend_i32_u(eth.getCodeSize())
}
function codecopy(x1, x2, x3, x4, y1, y2, y3, y4, z1, z2, z3, z4) {
	eth.codeCopy(
		to_internal_i32ptr(x1, x2, x3, x4),
		u256_to_i32(y1, y2, y3, y4),
		u256_to_i32(z1, z2, z3, z4)
	)
}
function datacopy(x1, x2, x3, x4, y1, y2, y3, y4, z1, z2, z3, z4) {
	// TODO correct?
	codecopy(x1, x2, x3, x4, y1, y2, y3, y4, z1, z2, z3, z4)
}

function gasprice() -> z1, z2, z3, z4 {
	eth.getTxGasPrice(0:i32)
	z1, z2, z3, z4 := mload_internal(0:i32)
}
function extcodesize_internal(x1, x2, x3, x4) -> r:i32 {
	mstore_address(0:i32, x1, x2, x3, x4)
	r := eth.getExternalCodeSize(12:i32)
}
function extcodesize(x1, x2, x3, x4) -> z1, z2, z3, z4 {
	z4 := i64.extend_i32_u(extcodesize_internal(x1, x2, x3, x4))
}
function extcodehash(x1, x2, x3, x4) -> z1, z2, z3, z4 {
	// TODO: not part of current Ewasm spec
	unreachable()
}
function extcodecopy(a1, a2, a3, a4, p1, p2, p3, p4, o1, o2, o3, o4, l1, l2, l3, l4) {
	mstore_address(0:i32, a1, a2, a3, a4)
	let codeOffset:i32 := u256_to_i32(o1, o2, o3, o4)
	let codeLength:i32 := u256_to_i32(l1, l2, l3, l4)
	eth.externalCodeCopy(12:i32, to_internal_i32ptr(p1, p2, p3, p4), codeOffset, codeLength)
}

function returndatasize() -> z1, z2, z3, z4 {
	z4 := i64.extend_i32_u(eth.getReturnDataSize())
}
function returndatacopy(x1, x2, x3, x4, y1, y2, y3, y4, z1, z2, z3, z4) {
	eth.returnDataCopy(
		to_internal_i32ptr(x1, x2, x3, x4),
		u256_to_i32(y1, y2, y3, y4),
		u256_to_i32(z1, z2, z3, z4)
	)
}

function blockhash(x1, x2, x3, x4) -> z1, z2, z3, z4 {
	let r:i32 := eth.getBlockHash(u256_to_i64(x1, x2, x3, x4), 0:i32)
	if i32.eqz(r) {
		z1, z2, z3, z4 := mload_internal(0:i32)
	}
}
function coinbase() -> z1, z2, z3, z4 {
	eth.getBlockCoinbase(0:i32)
	z1, z2, z3, z4 := mload_internal(0:i32)
}
function timestamp() -> z1, z2, z3, z4 {
	z4 := eth.getBlockTimestamp()
}
function number() -> z1, z2, z3, z4 {
	z4 := eth.getBlockNumber()
}
function difficulty() -> z1, z2, z3, z4 {
	eth.getBlockDifficulty(0:i32)
	z1, z2, z3, z4 := mload_internal(0:i32)
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
function mload(x1, x2, x3, x4) -> z1, z2, z3, z4 {
	z1, z2, z3, z4 := mload_internal(to_internal_i32ptr(x1, x2, x3, x4))
}
function mload_internal(pos:i32) -> z1, z2, z3, z4 {
	z1 := endian_swap(i64.load(pos))
	z2 := endian_swap(i64.load(i32.add(pos, 8:i32)))
	z3 := endian_swap(i64.load(i32.add(pos, 16:i32)))
	z4 := endian_swap(i64.load(i32.add(pos, 24:i32)))
}
function mstore(x1, x2, x3, x4, y1, y2, y3, y4) {
	mstore_internal(to_internal_i32ptr(x1, x2, x3, x4), y1, y2, y3, y4)
}
function mstore_internal(pos:i32, y1, y2, y3, y4) {
	i64.store(pos, endian_swap(y1))
	i64.store(i32.add(pos, 8:i32), endian_swap(y2))
	i64.store(i32.add(pos, 16:i32), endian_swap(y3))
	i64.store(i32.add(pos, 24:i32), endian_swap(y4))
}
function mstore_address(pos:i32, a1, a2, a3, a4) {
	a1, a2, a3 := u256_to_address(a1, a2, a3, a4)
	mstore_internal(pos, 0, a1, a2, a3)
}
function mstore8(x1, x2, x3, x4, y1, y2, y3, y4) {
	let v := u256_to_byte(y1, y2, y3, y4)
	i64.store8(to_internal_i32ptr(x1, x2, x3, x4), v)
}
// Needed?
function msize() -> z1, z2, z3, z4 {
	// TODO implement
	unreachable()
}
function sload(x1, x2, x3, x4) -> z1, z2, z3, z4 {
	mstore_internal(0:i32, x1, x2, x3, x4)
	eth.storageLoad(0:i32, 32:i32)
	z1, z2, z3, z4 := mload_internal(32:i32)
}

function sstore(x1, x2, x3, x4, y1, y2, y3, y4) {
	mstore_internal(0:i32, x1, x2, x3, x4)
	mstore_internal(32:i32, y1, y2, y3, y4)
	eth.storageStore(0:i32, 32:i32)
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
	eth.log(
		to_internal_i32ptr(p1, p2, p3, p4),
		u256_to_i32(s1, s2, s3, s4),
		0:i32, 0:i32, 0:i32, 0:i32, 0:i32
	)
}
function log1(
	p1, p2, p3, p4, s1, s2, s3, s4,
	t1_1, t1_2, t1_3, t1_4
) {
	eth.log(
		to_internal_i32ptr(p1, p2, p3, p4),
		u256_to_i32(s1, s2, s3, s4),
		1:i32,
		to_internal_i32ptr(t1_1, t1_2, t1_3, t1_4),
		0:i32, 0:i32, 0:i32
	)
}
function log2(
	p1, p2, p3, p4, s1, s2, s3, s4,
	t1_1, t1_2, t1_3, t1_4,
	t2_1, t2_2, t2_3, t2_4
) {
	eth.log(
		to_internal_i32ptr(p1, p2, p3, p4),
		u256_to_i32(s1, s2, s3, s4),
		2:i32,
		to_internal_i32ptr(t1_1, t1_2, t1_3, t1_4),
		to_internal_i32ptr(t2_1, t2_2, t2_3, t2_4),
		0:i32, 0:i32
	)
}
function log3(
	p1, p2, p3, p4, s1, s2, s3, s4,
	t1_1, t1_2, t1_3, t1_4,
	t2_1, t2_2, t2_3, t2_4,
	t3_1, t3_2, t3_3, t3_4
) {
	eth.log(
		to_internal_i32ptr(p1, p2, p3, p4),
		u256_to_i32(s1, s2, s3, s4),
		3:i32,
		to_internal_i32ptr(t1_1, t1_2, t1_3, t1_4),
		to_internal_i32ptr(t2_1, t2_2, t2_3, t2_4),
		to_internal_i32ptr(t3_1, t3_2, t3_3, t3_4),
		0:i32
	)
}
function log4(
	p1, p2, p3, p4, s1, s2, s3, s4,
	t1_1, t1_2, t1_3, t1_4,
	t2_1, t2_2, t2_3, t2_4,
	t3_1, t3_2, t3_3, t3_4,
	t4_1, t4_2, t4_3, t4_4,
) {
	eth.log(
		to_internal_i32ptr(p1, p2, p3, p4),
		u256_to_i32(s1, s2, s3, s4),
		4:i32,
		to_internal_i32ptr(t1_1, t1_2, t1_3, t1_4),
		to_internal_i32ptr(t2_1, t2_2, t2_3, t2_4),
		to_internal_i32ptr(t3_1, t3_2, t3_3, t3_4),
		to_internal_i32ptr(t4_1, t4_2, t4_3, t4_4)
	)
}

function create(
	x1, x2, x3, x4,
	y1, y2, y3, y4,
	z1, z2, z3, z4
) -> a1, a2, a3, a4 {
	let v1, v2 := u256_to_u128(x1, x2, x3, x4)
	mstore_internal(0:i32, 0, 0, v1, v2)

	let r:i32 := eth.create(0:i32, to_internal_i32ptr(y1, y2, y3, y4), u256_to_i32(z1, z2, z3, z4), 32:i32)
	if i32.eqz(r) {
		a1, a2, a3, a4 := mload_internal(32:i32)
	}
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
	let g := u256_to_i64(a1, a2, a3, a4)
	mstore_address(0:i32, b1, b2, b3, b4)

	let v1, v2 := u256_to_u128(c1, c2, c3, c4)
	mstore_internal(32:i32, 0, 0, v1, v2)

	x4 := i64.extend_i32_u(eth.call(g, 12:i32, 32:i32, to_internal_i32ptr(d1, d2, d3, d4), u256_to_i32(e1, e2, e3, e4)))
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
	mstore_address(0:i32, b1, b2, b3, b4)

	let v1, v2 := u256_to_u128(c1, c2, c3, c4)
	mstore_internal(32:i32, 0, 0, v1, v2)

	x4 := i64.extend_i32_u(eth.callCode(
		u256_to_i64(a1, a2, a3, a4),
		12:i32,
		32:i32,
		to_internal_i32ptr(d1, d2, d3, d4),
		u256_to_i32(e1, e2, e3, e4)
	))
}
function delegatecall(
	a1, a2, a3, a4,
	b1, b2, b3, b4,
	c1, c2, c3, c4,
	d1, d2, d3, d4,
	e1, e2, e3, e4,
	f1, f2, f3, f4
) -> x1, x2, x3, x4 {
	mstore_address(0:i32, b1, b2, b3, b4)

	x4 := i64.extend_i32_u(eth.callDelegate(
		u256_to_i64(a1, a2, a3, a4),
		12:i32,
		to_internal_i32ptr(c1, c2, c3, c4),
		u256_to_i32(d1, d2, d3, d4)
	))
}
function staticcall(
	a1, a2, a3, a4,
	b1, b2, b3, b4,
	c1, c2, c3, c4,
	d1, d2, d3, d4,
	e1, e2, e3, e4,
	f1, f2, f3, f4
) -> x1, x2, x3, x4 {
	mstore_address(0:i32, b1, b2, b3, b4)

	x4 := i64.extend_i32_u(eth.callStatic(
		u256_to_i64(a1, a2, a3, a4),
		12:i32,
		to_internal_i32ptr(c1, c2, c3, c4),
		u256_to_i32(d1, d2, d3, d4)
	))
}
function create2(
	a1, a2, a3, a4,
	b1, b2, b3, b4,
	c1, c2, c3, c4,
	d1, d2, d3, d4
) -> x1, x2, x3, x4 {
	// TODO: not part of current Ewasm spec
	unreachable()
}
function selfdestruct(a1, a2, a3, a4) {
	mstore_address(0:i32, a1, a2, a3, a4)
	// In EVM, addresses are padded to 32 bytes, so discard the first 12.
	eth.selfDestruct(12:i32)
}

function return(x1, x2, x3, x4, y1, y2, y3, y4) {
	eth.finish(
		to_internal_i32ptr(x1, x2, x3, x4),
		u256_to_i32(y1, y2, y3, y4)
	)
}
function revert(x1, x2, x3, x4, y1, y2, y3, y4) {
	eth.revert(
		to_internal_i32ptr(x1, x2, x3, x4),
		u256_to_i32(y1, y2, y3, y4)
	)
}
function invalid() {
	unreachable()
}
}
)"};

}

Object EVMToEwasmTranslator::run(Object const& _object)
{
	if (!m_polyfill)
		parsePolyfill();

	Block ast = std::get<Block>(Disambiguator(m_dialect, *_object.analysisInfo)(*_object.code));
	set<YulString> reservedIdentifiers;
	NameDispenser nameDispenser{m_dialect, ast, reservedIdentifiers};
	OptimiserStepContext context{m_dialect, nameDispenser, reservedIdentifiers};

	FunctionHoister::run(context, ast);
	FunctionGrouper::run(context, ast);
	MainFunction::run(context, ast);
	ForLoopConditionIntoBody::run(context, ast);
	ExpressionSplitter::run(context, ast);
	WordSizeTransform::run(m_dialect, WasmDialect::instance(), ast, nameDispenser);

	NameDisplacer{nameDispenser, m_polyfillFunctions}(ast);
	for (auto const& st: m_polyfill->statements)
		ast.statements.emplace_back(ASTCopier{}.translate(st));

	Object ret;
	ret.name = _object.name;
	ret.code = make_shared<Block>(move(ast));
	ret.analysisInfo = make_shared<AsmAnalysisInfo>();

	ErrorList errors;
	ErrorReporter errorReporter(errors);
	AsmAnalyzer analyzer(*ret.analysisInfo, errorReporter, WasmDialect::instance(), {}, _object.dataNames());
	if (!analyzer.analyze(*ret.code))
	{
		string message = "Invalid code generated after EVM to wasm translation.\n";
		message += "Note that the source locations in the errors below will reference the original, not the translated code.\n";
		message += "Translated code:\n";
		message += "----------------------------------\n";
		message += ret.toString(&WasmDialect::instance());
		message += "----------------------------------\n";
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

void EVMToEwasmTranslator::parsePolyfill()
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
		m_polyfillFunctions.insert(std::get<FunctionDefinition>(statement).name);
}
