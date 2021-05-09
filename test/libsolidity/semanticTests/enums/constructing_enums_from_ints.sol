contract c {
    enum Truth {False, True}

    function test() public returns (uint256) {
        return uint256(Truth(uint8(0x1)));
    }
}

// ====
// compileToEwasm: also
// compileViaYul: also
// ----
// test() -> 1
