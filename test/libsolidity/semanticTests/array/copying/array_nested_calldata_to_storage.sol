pragma experimental ABIEncoderV2;

contract c {
    uint256[][] a;

    function test(uint256[][] calldata d) external returns (uint256, uint256) {
        a = d;
        assert(a[0][0] == d[0][0]);
        assert(a[0][1] == d[0][1]);
        return (a.length, a[1][0] + a[1][1]);
    }
}
// ====
// compileViaYul: true
// ----
// test(uint256[][]): 0x20, 2, 0x40, 0x40, 2, 23, 42 -> 2, 65
