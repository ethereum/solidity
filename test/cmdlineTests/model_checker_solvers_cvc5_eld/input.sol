// SPDX-License-Identifier: GPL-3.0
pragma solidity >=0.0;
contract test {
	function f() public pure {
		uint y = 1;
		if (y == 0)
			revert();
		assert(y > 0);
	}
}
