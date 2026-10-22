// SPDX-License-Identifier: GPL-3.0
pragma solidity >=0.0;

contract C {
    constructor() payable {}

    function f() public {}
    function g() public pure {}
    function h() public view {}
    function i() public payable {}

    receive() external payable {}
    fallback() external {}
}

