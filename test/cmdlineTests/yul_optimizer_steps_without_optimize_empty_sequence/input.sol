// SPDX-License-Identifier: GPL-3.0
pragma solidity >=0.0;

contract C
{
    constructor() {}

    function foo() public pure returns (bool)
    {
        // given the empty optimizer sequence ``:``, ``b`` and ``c`` should not be removed in the
        // optimized IR as the ``UnusedPruner`` step will not be run.
        uint a = 100;
        uint b = a;
        uint c = a;

        return a == 100;
    }
}
