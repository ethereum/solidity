// SPDX-License-Identifier: GPL-3.0
pragma solidity > 0.7.1;
pragma abicoder v2;

contract C {
	function f(uint a, uint b, uint c, uint d) public pure returns (uint, int, uint, uint) {
		uint w = 2**a;
		int x = (-2)**b;
		uint y = 10**c;
		uint z = (2**256 -1 )**d;

		// Special cases: 0, 1, and -1
		w = (0)**a;
		x = (-1)**b;
		y = 1**c;

		return (w, x, y, z);
	}
}
