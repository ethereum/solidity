// SPDX-License-Identifier: UNLICENSED
pragma solidity >=0.8.0;

contract MyContract
{
    constructor()
    {
        uint unused; // [Warning 2072] Unused local variable.
    }
}

contract D
{
    function main() public payable returns (uint)
    {
        MyContract c = new MyContract();
    }
}
