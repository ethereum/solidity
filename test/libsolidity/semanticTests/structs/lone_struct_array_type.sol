contract C {
    struct s {
        uint256 a;
        uint256 b;
    }

    function f() public returns (uint256) {
        s[7][]; // This is only the type, should not have any effect
        return 3;
    }
}

// ====
// compileToEwasm: also
// compileViaYul: also
// ----
// f() -> 3
