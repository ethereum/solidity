// SPDX-License-Identifier: GPL-3.0
pragma solidity >=0.0;
contract test {
	uint x;
    function f(address _a) public {
		_a.call("");
		assert(x < 10);
    }
	function g() public view {
		assert(x < 10);
	}
}