contract c {
    bytes data;

    function test() public returns (uint256 x, uint256 y, uint256 l) {
        data.push(0x07);
        data.push(0x03);
        x = data.length;
        data.pop();
        data.pop();
        data.push(0x02);
        y = data.length;
        l = data.length;
    }
}
// ====
// compileToEwasm: also
// compileViaYul: also
// ----
// test() -> 2, 1, 1
