contract c {
    uint256[] data;

    function test() public returns (uint256 x, uint256 l) {
        data.push(7);
        data.push(3);
        x = data.length;
        data.pop();
        x = data.length;
        data.pop();
        l = data.length;
    }
}
// ----
// test() -> 1, 0
