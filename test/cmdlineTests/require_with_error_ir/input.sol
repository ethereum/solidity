// SPDX-License-Identifier: GPL-3.0
pragma solidity >=0.0;

error CustomError(uint256, string);

contract C {
    function f(bool condition) external pure {
        require(condition, CustomError(1, "two"));
    }

    function g(bool condition) external pure {
        require(condition, CustomError(2, "three"));
    }
}
