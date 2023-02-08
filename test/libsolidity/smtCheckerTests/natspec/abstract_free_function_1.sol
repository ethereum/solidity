// SPDX-License-Identifier: MIT
pragma solidity >=0.8.13;

contract C {
    function f(uint x) external pure {
        uint t = msb(x);
        assert(t == 0); // should fail
    }
}

/// @custom:smtchecker abstract-function-nondet
function msb(uint256 x) pure returns (uint256 result) {}

// ====
// SMTEngine: chc
// ----
// Warning 6328: (144-158): CHC: Assertion violation happens here.
