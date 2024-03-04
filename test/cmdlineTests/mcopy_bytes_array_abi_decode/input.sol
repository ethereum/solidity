// SPDX-License-Identifier: GPL-3.0
pragma solidity >=0.0.0;

contract C {
    function foo() external pure
    {
        abi.decode("abcd", (bytes));
    }
}
