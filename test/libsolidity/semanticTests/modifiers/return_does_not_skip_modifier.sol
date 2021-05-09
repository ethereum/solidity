contract C {
    uint256 public x;
    modifier setsx {
        _;
        x = 9;
    }

    function f() public setsx returns (uint256) {
        return 2;
    }
}

// ====
// compileViaYul: also
// compileToEwasm: also
// ----
// x() -> 0
// f() -> 2
// x() -> 9
