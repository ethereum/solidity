contract C {
    function test() public returns (uint256) {
        // Note that this only works because computation on literals is done using
        // unbounded integers.
        if ((2**255 + 2**255) % 7 != addmod(2**255, 2**255, 7)) return 1;
        if ((2**255 + 2**255) % 7 != addmod(2**255, 2**255, 7)) return 2;
        return 0;
    }
}

// ====
// compileToEwasm: also
// compileViaYul: also
// ----
// test() -> 0
