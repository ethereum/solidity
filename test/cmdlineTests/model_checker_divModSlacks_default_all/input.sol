// SPDX-License-Identifier: GPL-3.0
pragma solidity >=0.0;
contract C {
	function f(uint a, uint b) public pure returns (uint, uint) {
		require(b != 0);
		return (a / b, a % b);
	}
}
