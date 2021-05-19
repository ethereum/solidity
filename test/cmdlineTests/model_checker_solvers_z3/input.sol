// SPDX-License-Identifier: GPL-3.0
pragma solidity >=0.0;
contract test {
	function f(uint x) public pure {
		assert(x > 0);
	}
}