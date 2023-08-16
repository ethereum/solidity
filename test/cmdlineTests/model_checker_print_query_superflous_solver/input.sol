// SPDX-License-Identifier: GPL-3.0
pragma solidity >=0.0;
contract C
{
	function f() public pure {
		uint x = 0;
		assert(x == 0);
	}
}
