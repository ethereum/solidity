// SPDX-License-Identifier: GPL-3.0
pragma solidity >=0.0;
contract C
{
	function f(uint x) public pure {
		require(x == 0);
		do {
			++x;
		} while (x < 2);
		assert(x == 3);
	}
}