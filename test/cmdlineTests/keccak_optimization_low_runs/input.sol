// SPDX-License-Identifier: GPL-v3
pragma solidity >= 0.0.0;

contract C {
    fallback() external {
        assembly {
            mstore(0, 100)
            // because of the low runs value, the constant will not be optimized
            sstore(0, keccak256(0, 32))
        }
    }
}
