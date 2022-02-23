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

// NOTE: This file is used to generate `ewasmPolyfills/Logical.h`.

function or_bool(a, b, c, d) -> r:i32 {
	r := i32.eqz(i64.eqz(i64.or(i64.or(a, b), i64.or(c, d))))
}

function or_bool_320(a, b, c, d, e) -> r:i32 {
	r := i32.or(or_bool(a, b, c, 0), or_bool(d, e, 0, 0))
}

function or_bool_512(a, b, c, d, e, f, g, h) -> r:i32 {
	r := i32.or(or_bool(a, b, c, d), or_bool(e, f, g, h))
}
