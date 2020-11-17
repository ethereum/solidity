// SPDX-License-Identifier: GPL-3.0
pragma solidity >=0.6.0;

contract C {}
contract D {
    function f() public {
        C c = new C();
    }
}
