// SPDX-License-Identifier: GPL-3.0
pragma solidity >=0.0;

library L {
    function f() external {}
}

library L1 {
    function f() external {}
}

library L2 {
    function f() external {}
}

library L3 {
    function f() external {}
}

library L4 {
    function f() external {}
}

library L5 {
    function f() external {}
}

library L6 {
    function f() external {}
}

contract C {
    function foo() public {
        L.f();
        L1.f();
        L2.f();
        L3.f();
        L4.f();
        L5.f();
        L6.f();
    }
}
