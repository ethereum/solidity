// SPDX-License-Identifier: GPL-3.0
pragma solidity >=0.0;
pragma experimental SMTChecker;
contract test {
    function f(uint x) public pure {
		assert(x > 0);
		assert(x > 2);
		assert(x > 4);
    }
}