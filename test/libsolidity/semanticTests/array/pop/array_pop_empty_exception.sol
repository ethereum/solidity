contract c {
    uint256[] data;

    function test() public returns (bool) {
        data.pop();
        return true;
    }
}
// ====
// compileViaYul: also
// compileToEwasm: also
// ----
// test() -> FAILURE
