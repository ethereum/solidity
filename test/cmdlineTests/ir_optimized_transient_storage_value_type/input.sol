// SPDX-License-Identifier: GPL-3.0
pragma solidity >=0.0.0;

contract C {
    uint transient x;
    function f() public {
        x = x + 1;
    }
}
