contract C {
    function gasleft() public returns (uint256) {
        return 0;
    }

    function f() public returns (uint256) {
        return gasleft();
    }
}

// ====
// compileToEwasm: also
// ----
// f() -> 0
