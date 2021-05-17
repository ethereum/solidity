// SPDX-License-Identifier: UNLICENSED
pragma solidity >=0.8.0;

/// Some Error type E.
error E(uint, uint);

enum Weather {
    Sunny,
    Cloudy,
    Rainy
}

/// My contract MyContract.
///
contract MyContract
{
    Weather lastWeather = Weather.Rainy;
    uint constant fixedValue = 1234;

    constructor()
    {
    }

    /// Sum is summing two args and returning the result
    ///
    /// @param a me it is
    /// @param b me it is also
    function sum(uint a, uint b) public pure returns (uint result)
    {
        Weather weather = Weather.Sunny;
        uint foo = 12345 + fixedValue;
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

contract D
{
    function main() public payable returns (uint)
    {
        MyContract c = new MyContract();
        return c.sum(2, 3);
    }
}
