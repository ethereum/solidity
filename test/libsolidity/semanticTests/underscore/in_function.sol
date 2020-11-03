contract C {
    function f() public pure returns (uint) {
        uint _;
        return _;
    }

    function g() public pure returns (uint) {
        uint _ = 1;
        return _;
    }
}
// ====
// compileViaYul: also
// ----
// f() -> 0
// g() -> 1
