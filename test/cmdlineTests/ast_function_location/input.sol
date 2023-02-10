// SPDX-License-Identifier: GPL-3.0
pragma solidity ^0.8.17;

function checkPattern(int _x, int _y,int _xStep, int _yStep)  returns(int startX, int startY, int endX, int endY)
{ 
   
    return (_x, _x, _x, _x);
}

function checkPattern(int _x, int _y)  returns(bool)
{
    return true;
    
}
function checkStealing(uint _x, uint _y) 
{

    for (uint i = 0; i < 3; i++)
    {
        (int xS, int yS, int xE, int yE) = checkPattern(_x, _y,i, i); // error here
    }
}

contract C {

        uint a = 1;
        uint b = 1;
        function callFuncWithKeyValue() public {
          checkStealing(a,b);
        }
}


