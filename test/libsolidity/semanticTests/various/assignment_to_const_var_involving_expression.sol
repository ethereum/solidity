contract C {
    uint256 constant x = 0x123 + 0x456;

    function f() public returns (uint256) {
        return x + 1;
    }
}

// ====
// compileToEwasm: also
// compileViaYul: also
// ----
// f() -> 0x57a
