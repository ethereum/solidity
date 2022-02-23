// SPDX-License-Identifier: GPL-v3
pragma solidity >= 0.0.0;

contract C {
    constructor() {
        assembly {
            mstore(0, 100)
            // because this is part of deploy code, the keccak will not be evaluated
            sstore(0, keccak256(0, 32))
        }
    }

    fallback() external {
        assembly {
            mstore(0, 100)
            // The keccak here would be evaluated
            sstore(0, keccak256(0, 32))
        }

    }
}
