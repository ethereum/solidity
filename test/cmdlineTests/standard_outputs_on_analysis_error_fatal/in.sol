// SPDX-License-Identifier: GPL-3.0
pragma solidity *;

contract C {
    // This will trigger a fatal error at the analysis stage, of the kind that terminates analysis
    // immediately without letting the current step finish.
    constructor(uint[] storage) {}
}
