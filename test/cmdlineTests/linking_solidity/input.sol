// SPDX-License-Identifier: GPL-3.0
pragma solidity >=0.0;

library L {
    function f() external {}
}

contract C {
    function foo() public {
        L.f();
    }
}
