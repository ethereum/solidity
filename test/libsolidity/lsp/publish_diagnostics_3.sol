// SPDX-License-Identifier: UNLICENSED
pragma solidity >=0.8.0;

abstract contract A {
    function a() public virtual;
}

contract B is A
// ^( @notAbstract
{
}
// ^) @notAbstract
