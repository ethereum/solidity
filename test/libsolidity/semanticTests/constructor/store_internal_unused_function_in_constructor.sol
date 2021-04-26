contract C {
    function() returns (uint256) internal x;

    constructor() {
        x = unused;
    }

    function unused() internal returns (uint256) {
        return 7;
    }

    function t() public returns (uint256) {
        return x();
    }
}

// ====
// compileToEwasm: also
// compileViaYul: also
// ----
// t() -> 7
