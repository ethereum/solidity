// SPDX-License-Identifier: UNLICENSED
pragma solidity >=0.8.0;

/// Some Error type E.
error E(uint, uint);

enum Weather {
    Sunny,
    Cloudy,
    Rainy
}

/// Some custom Color enum type holding 3 colors.
enum Color {
    /// Red color.
    Red,
    /// Green color.
    Green,
    /// Blue color.
    Blue
}

library Lib
{
    function add(uint a, uint b) public pure returns (uint result)
// ^( @addFunction
    {
        result = a + b;
    }

// ^) @addFunction
    function warningWithUnused() public pure
    {
        uint unused;
    //  ^^^^^^^^^^^ @diagnostics
    }
}

struct RGBColor
{
    uint8 red;
    uint8 green;
    uint8 blue;
}
