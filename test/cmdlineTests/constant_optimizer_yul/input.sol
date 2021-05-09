// SPDX-License-Identifier: GPL-v3
pragma solidity >= 0.0.0;
contract C {
    constructor () {
        assembly {
            // This shl should not be evaluated for all values of optimize-runs
            sstore(0, shl(180, 1))
        }
    }
    fallback() external {
        assembly {
            // This shl would be evaluated for high values of optimize-runs
            sstore(0, shl(180, 1))
        }
    }
}
