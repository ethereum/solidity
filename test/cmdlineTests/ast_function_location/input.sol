// SPDX-License-Identifier: GPL-3.0
pragma solidity *;

function checkPattern(int _x, int _y,int _xStep, int _yStep) returns (int startX, int startY, int endX, int endY)
{
     return (_x, _x, _x, _x);
}

function checkPattern(int _x, int _y) returns (bool)
{
    return true;
}

function checkStealing(uint _x, uint _y)
{

    for (uint i = 0; i < 3; i++)
    {
        checkPattern(_x, _y, i, i);
    }
}

contract C {

    uint a = 1;
    uint b = 1;
    function test() public {
        checkStealing(a,b);
    }
}


