// SPDX-License-Identifier: UNLICENSED
pragma solidity >=0.8.0;

import "my-module/test.sol";

contract MyContract
{
    function f(uint a, uint b) public pure returns (uint)
    {
        return MyModule.add(a, b);
    }
}
// ----
// test:
