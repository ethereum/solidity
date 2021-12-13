// SPDX-License-Identifier: UNLICENSED
pragma solidity >=0.8.0;

library Lib
{
    function add(uint a, uint b) public pure returns (uint result)
    {
        result = a + b;
    }

    function warningWithUnused() public pure
    {
        uint unused;
    }
}
