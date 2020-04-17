contract C {
    function f() public returns (uint256) {
        uint256 x = 1;
        (x) = 2;
        return x;
    }
}
// ====
// compileViaYul: also
// ----
// f() -> 2
