// SPDX-License-Identifier: GPL-3.0
pragma solidity >=0.0;
contract B {
    function f(uint x) public pure {
		assert(x > 0);
    }
}
contract A is B {
    function g(uint y) public pure {
		assert(y > 0);
    }
}