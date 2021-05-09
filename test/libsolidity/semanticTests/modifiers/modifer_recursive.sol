contract C {
    uint public called;
    modifier mod1 {
        called++;
        _;
    }
    function f(uint x) public mod1 returns (uint256 r) {
        return x == 0 ? 2 : f(x - 1)**2;
    }
}

// ====
// compileViaYul: also
// compileToEwasm: also
// ----
// called() -> 0x00
// f(uint256): 5 -> 0x0100000000
// called() -> 6
