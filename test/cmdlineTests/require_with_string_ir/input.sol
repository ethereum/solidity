// SPDX-License-Identifier: GPL-3.0
pragma solidity >=0.0;

contract C {
    function f(bool condition) external pure {
        string memory message = "Condition must be satisfied";
        require(condition, message);
    }

    function g(bool condition) external pure {
        require(condition, "Condition must be satisfied");
    }
}
