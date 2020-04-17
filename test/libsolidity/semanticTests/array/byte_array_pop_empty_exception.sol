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
// ----
// test() -> FAILURE
