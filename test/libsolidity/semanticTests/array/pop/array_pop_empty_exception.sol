contract c {
    uint256[] data;

    function test() public returns (bool) {
        data.pop();
        return true;
    }
}
// ----
// test() -> FAILURE, hex"4e487b71", 0x31
