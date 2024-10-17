// SPDX-License-Identifier: GPL-3.0
pragma solidity >=0.0;

interface I {
    function f() external pure returns (uint);
}

contract C is I {
    function f() public pure override returns (uint) {
        return 42;
    }
}

contract D {
    function f() public returns (uint) {
        C c = new C();
        return c.f();
    }
}
