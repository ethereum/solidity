// SPDX-License-Identifier: GPL-3.0
pragma solidity >=0.0;
pragma abicoder v2;

contract C {
    constructor() payable {
        assembly ("memory-safe") {
            let a := 0
            // Without the cleanup sequence this will not be simplified to ``revert(a, a)``.
            revert(0, a)
        }
    }
}
