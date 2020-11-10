contract B {
    uint immutable x = 1;
    function f() public pure returns (uint) {
        return x;
    }
}
// ----
