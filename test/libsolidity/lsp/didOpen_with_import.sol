// SPDX-License-Identifier: UNLICENSED
pragma solidity >=0.8.0;

import './lib.sol';

contract C
{
    function f(uint a, uint b) public pure returns (uint)
    {
        return Lib.add(2 * a, b);
        //     ^^^^^^^ @diagnostics
    }
}
