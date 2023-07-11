// SPDX-License-Identifier: GPL-3.0
pragma solidity >=0.0.0;

contract C {
    constructor() payable {
        uint256 x = block.prevrandao;
        assembly {
            sstore(0, x)
            stop()
        }
    }

    fallback() external payable {
        assembly {
            stop()
        }
    }
}
