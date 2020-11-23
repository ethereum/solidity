contract C {
    function() internal returns (uint)[20] x;
    int256 mutex;

    function one() public returns (uint256) {
        function() internal returns (uint)[20] memory xmem;
        x = xmem;
        return 3;
    }

    function two() public returns (uint256) {
        if (mutex > 0) return 7;
        mutex = 1;
        // If this test fails, it might re-execute this function.
        x[0]();
        return 2;
    }
}

// ====
// compileViaYul: also
// compileToEwasm: also
// ----
// one() -> 3
// two() -> FAILURE, hex"4e487b71", 0x51
