contract c {
    uint256[] data;

    function test() public returns (bool) {
        data.pop();
        return true;
    }
}
// ====
// compileToEwasm: also
// compileViaYul: also
// ----
// test() -> FAILURE
