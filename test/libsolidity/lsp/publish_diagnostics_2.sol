// SPDX-License-Identifier: UNLICENSED
pragma solidity >=0.8.0;

contract C
{
    function makeSomeError() public pure returns (uint res)
    {
        uint x = "hi";
    //  ^^^^^^^^^^^^^ @conversionError
        return;
     // ^^^^^^^ @argumentsRequired
        res = 2;
    }
}

contract D
{
    function main() public payable returns (uint)
    {
        C c = new C();
        return c.makeSomeError(2, 3);
          //   ^^^^^^^^^^^^^^^^^^^^^ @wrongArgumentsCount
    }
}
