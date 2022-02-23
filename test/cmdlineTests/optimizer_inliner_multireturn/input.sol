// SPDX-License-Identifier: GPL-3.0
pragma solidity >=0.0;

function test(uint x) pure returns (uint, uint)
{
    unchecked {
        return (x + 1, x);
    }
}

contract C {
    function f() public pure {
        for((uint x, uint y) = (0, 1); x < 10; (x, y) = test(x))
        {
        }
    }
}
