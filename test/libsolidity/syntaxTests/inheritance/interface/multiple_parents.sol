interface SuperA {
    function test() external returns (uint256);
    function testA() external returns (int128);
}

interface SuperB {
    function test() external returns (uint256);
    function testB() external returns (int256);
}

interface Sub is SuperA, SuperB {
}

// ----
// TypeError 6480: (236-271): Derived contract must override function "test". Two or more base classes define function with same name and parameter types.
