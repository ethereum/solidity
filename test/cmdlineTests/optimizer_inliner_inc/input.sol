// SPDX-License-Identifier: GPL-3.0
pragma solidity >=0.0;

function unsafe_inc(uint x) pure returns (uint)
{
    unchecked {
        return x + 1;
    }
}

contract C {
    function f() public pure {
        for(uint x = 0; x < 10; x = unsafe_inc(x))
        {
        }
    }
}
