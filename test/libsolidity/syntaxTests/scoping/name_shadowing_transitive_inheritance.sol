contract A {}
contract B is A {
    int256 value = 10;
}
contract C is B {
    function f() public pure returns (uint256) {
        uint256 value = 99;
        return value;
    }
}
// ----
// Warning 2519: (132-145): This declaration shadows an existing declaration.
