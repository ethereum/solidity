contract C {
    uint256 public x;
    modifier run() {
        for (uint256 i = 1; i < 10; i++) {
            if (i == 5) return;
            _;
        }
    }

    function f() public run {
        uint256 k = x;
        uint256 t = k + 1;
        x = t;
    }
}

// ====
// compileViaYul: also
// compileToEwasm: also
// ----
// x() -> 0
// f() ->
// x() -> 4
