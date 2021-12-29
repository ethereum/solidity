contract C {
    uint256 public x;
    modifier m() {
        x ++;
        _;
    }

    modifier mdouble() {
        _;
        _;
    }

    function f() public m m m m m m m m m m mdouble m m m m m m m m m m m m m m m m returns (uint) {
        return x;
    }
}

// ====
// compileToEwasm: also
// compileViaYul: also
// ----
// x() -> 0
// f() -> 0x2a
// x() -> 0x2a
