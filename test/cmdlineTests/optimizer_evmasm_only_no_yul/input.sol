// SPDX-License-Identifier: GPL-3.0
pragma solidity *;

contract C {
    struct A { string a; }
    function f() public pure returns (A memory) {
        A memory t;
        return t;
    }
}
