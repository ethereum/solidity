// SPDX-License-Identifier: GPL-3.0
pragma solidity >=0.0;
pragma abicoder v2;

contract C {
    constructor() payable {
        assembly ("memory-safe") {
            let a := 0
            revert(0, a)
        }
    }
}
