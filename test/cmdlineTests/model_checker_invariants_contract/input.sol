// SPDX-License-Identifier: GPL-3.0
pragma solidity >=0.0;
contract test {
	uint x;
    function f() public view {
		assert(x < 10);
    }
}