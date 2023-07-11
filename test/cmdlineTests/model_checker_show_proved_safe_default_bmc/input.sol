// SPDX-License-Identifier: GPL-3.0
pragma solidity >=0.0;
contract C {
    function f(uint8 x) public {
        assert(x >= 0);
        assert(x < 1000);
    }
}
