// SPDX-License-Identifier: GPL-3.0
pragma solidity >=0.0;

library L1 {
    function f() external {}
}

library L2 {
    function f() external {}
}

contract C {
    function foo() public {
        L1.f();
        L2.f();
        L1.f();
        L2.f();
        L1.f();
    }
}
