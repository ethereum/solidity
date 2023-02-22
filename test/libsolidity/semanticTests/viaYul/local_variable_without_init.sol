contract C {
    function f() public pure returns (uint) {
        uint x;
        return x;
    }
}
// ----
// f() -> 0
