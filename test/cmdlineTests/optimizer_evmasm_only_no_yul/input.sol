// SPDX-License-Identifier: GPL-3.0
pragma solidity *;

// Test case for issue #16306: --optimize --no-optimize-yul should not cause ICE
contract C {
    struct A { string a; }
    function f() public pure returns (A memory) {
        A memory t;
        return t;
    }
}
