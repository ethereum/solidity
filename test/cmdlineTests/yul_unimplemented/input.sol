// SPDX-License-Identifier: GPL-3.0
pragma solidity >=0.0;
contract test {
    bytes a;
    bytes b;
    function f() public {
        a = b;
    }
}