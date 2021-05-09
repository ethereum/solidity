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

// NOTE: This file is used to generate `ewasmPolyfills/Comparison.h`.

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
	r4 := i64.extend_i32_u(
		i32.and(
			i64.eq(x1, y1),
			i32.and(
				i64.eq(x2, y2),
				i32.and(
					i64.eq(x3, y3),
					i64.eq(x4, y4)
				)
			)
		)
	)
}

// returns 0 if a == b, -1 if a < b and 1 if a > b
function cmp(a, b) -> r:i32 {
	r := i32.select(0xffffffff:i32, i64.ne(a, b), i64.lt_u(a, b))
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
}

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
