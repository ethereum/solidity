contract C {
    function(uint256) returns (uint256) internal x;

    constructor() {
        x = double;
    }

    function test() public returns (bool) {
        return x == double;
    }

    function double(uint256 _arg) public returns (uint256 _ret) {
        _ret = _arg * 2;
    }
}

// ====
// compileToEwasm: also
// ----
// test() -> true
