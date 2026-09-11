// SPDX-License-Identifier: GPL-3.0
pragma solidity >=0.0;

contract C {
    uint256 value;

    function f(uint256 argument) public {
        value = argument;
    }
}
