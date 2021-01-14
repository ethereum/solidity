// SPDX-License-Identifier: GPL-3.0
pragma solidity >=0.0;

function unsafe_add(uint x, uint y) pure returns (uint)
{
    unchecked {
        return x + y;
    }
}

contract C {
    function f() public pure {
        for(uint x = 0; x < 10; x = unsafe_add(x, unsafe_add(x, 1)))
        {
        }
    }
}
