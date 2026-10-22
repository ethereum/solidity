contract Base {
    uint256 value = 10;
}
contract Child is Base {
    function f() public pure returns (uint256 value) {
        return 42;
    }
}
// ----
// Warning 2519: (105-118): This declaration shadows an existing declaration.
