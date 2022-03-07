// SPDX-License-Identifier: GPL-3.0
pragma solidity >=0.0.0;
pragma abicoder v2;

contract D {
    constructor() { assembly { mstore(0,0) } }
    function f() public pure {}
}
