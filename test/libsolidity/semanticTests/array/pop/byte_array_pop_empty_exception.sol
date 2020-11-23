contract c {
    uint256 a;
    uint256 b;
    uint256 c;
    bytes data;

    function test() public returns (bool) {
        data.pop();
        return true;
    }
}
// ====
// compileViaYul: also
// compileToEwasm: also
// ----
// test() -> FAILURE, hex"4e487b71", 0x31
