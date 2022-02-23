// SPDX-License-Identifier: GPL-3.0
pragma solidity >=0.0;

contract C {
    uint x;
    constructor() { x = a(); }
    function a() public pure returns (uint) { return f(); } // this should be inlined
    function f() internal pure returns (uint) { return 6; }
}
