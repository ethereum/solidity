pragma solidity >=0.0;
// SPDX-License-Identifier: GPL-3.0
contract C {
    function f(uint[] calldata x) pure external returns (uint) { return x[0]; }
    // This will be optimized out
    function g() pure internal {}
    mapping(uint => uint) public t;
    constructor(uint x) {
        t[0] = x;
    }
}
