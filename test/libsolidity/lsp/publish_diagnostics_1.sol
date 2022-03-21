// SPDX-License-Identifier: UNLICENSED
pragma solidity >=0.8.0;

contract MyContract
{
    constructor()
    {
        uint unused; // [Warning 2072] Unused local variable.
   //   ^^^^^^^^^^^ @unusedVariable
    }
}

contract D
{
    function main() public payable returns (uint)
                                       //   ^^^^ @unusedReturnVariable
    {
        MyContract c = new MyContract();
   //   ^^^^^^^^^^^^ @unusedContractVariable
    }
}
