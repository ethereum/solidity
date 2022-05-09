// SPDX-License-Identifier: UNLICENSED
pragma solidity >=0.8.0;

/// Some Error type E.
error E(uint, uint);

enum Weather {
//   ^^^^^^^ @whetherEnum
    Sunny,
    Cloudy,
    Rainy
}

/// Some custom Color enum type holding 3 colors.
enum Color {
//   ^^^^^ @ColorEnum
    /// Red color.
    Red,
//  ^^^ @EnumMemberRed
    /// Green color.
    Green,
    /// Blue color.
    Blue
}

library Lib
//   @  ^^^ @LibLibrary
{
    function add(uint a, uint b) public pure returns (uint result)
// ^( @addFunction
//           ^^^ @addSymbol
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
//     ^^^^^^^^ @RGBColorStruct
{
    uint8 red;
    uint8 green;
    uint8 blue;
}
// ----
// lib: @diagnostics 2072
