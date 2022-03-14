// SPDX-License-Identifier: UNLICENSED
pragma solidity >=0.8.0;

import {Weather as Wetter} from "./lib.sol";
import "./lib.sol" as That;

contract C
{
    function test_symbol_alias() public pure returns (Wetter result)
    {
        result = Wetter.Sunny;
    }

    function test_library_alias() public pure returns (That.Color result)
    {
        That.Color color = That.Color.Red;
        result = color;
    }
}
