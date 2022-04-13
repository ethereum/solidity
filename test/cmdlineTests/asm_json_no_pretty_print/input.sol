// SPDX-License-Identifier: GPL-3.0
pragma solidity >=0.0;

contract C {
    function f(uint x) public pure {
        x += 42;
        require(x > 100);
    }
}
