// SPDX-License-Identifier: UNLICENSED
pragma solidity >=0.8.0;

/// Some Error type E.
error E(uint, uint);

enum Weather {
    Sunny,
    Cloudy,
    Rainy
}

contract MyContract
{
    Weather lastWheather = Weather.Rainy;
    uint constant someFixedValue = 1234;

    function sum(uint a, uint b) public pure returns (uint result)
    {
        Weather weather = Weather.Sunny;
        uint foo = 12345 + someFixedValue;
        if (a == b)
            revert E(a, b);
        weather = Weather.Cloudy;
        result = a + b + foo;
    }

    function main() public pure returns (uint)
    {
        return sum(2, 3 - 123 + 456);
    }
}
