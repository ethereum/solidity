contract C {
    function f() public returns (uint256 r) {
        uint256;
        uint256;
        uint256;
        uint256;
        int256 x = -7;
        return uint256(x);
    }
}

// ====
// compileViaYul: also
// compileToEwasm: also
// ----
// f() -> -7
