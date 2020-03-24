contract C {
    uint256 public x;
    modifier run() {
        for (uint256 i = 0; i < 10; i++) {
            if (i % 2 == 1) continue;
            _;
        }
    }

    function f() public run {
        uint256 k = x;
        uint256 t = k + 1;
        x = t;
    }
}

// ----
// x() -> 0
// f() ->
// x() -> 5
