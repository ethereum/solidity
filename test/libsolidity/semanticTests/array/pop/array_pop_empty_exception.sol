contract c {
    uint256[] data;

    function test() public returns (bool) {
        data.pop();
        return true;
    }
}
// ====
// compileToEwasm: also
// ----
// test() -> FAILURE, hex"4e487b71", 0x31
