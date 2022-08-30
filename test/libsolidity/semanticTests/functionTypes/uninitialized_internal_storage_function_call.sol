contract Test {
    function() internal x;

    function f() public returns (uint256 r) {
        x();
        return 2;
    }
}

// ====
// compileToEwasm: also
// ----
// f() -> FAILURE, hex"4e487b71", 0x51
