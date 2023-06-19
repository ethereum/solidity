// SPDX-License-Identifier: GPL-3.0
pragma solidity *;

contract C {
    // This will trigger a non-fatal error at the analysis stage.
    // With this kind of error we still run subsequent analysis stages.
    uint x;
    string y = x;
}
