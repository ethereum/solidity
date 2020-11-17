// SPDX-License-Identifier: GPL-3.0
pragma solidity >=0.0;
library L { function f(uint) public {} }
contract test {
    using L for uint;
    function f() public {
        uint x;
        x.f();
    }
}