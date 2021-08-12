contract c {
    uint256[] data;

    function test()
        public
        returns (uint256 x, uint256 y, uint256 z, uint256 l)
    {
        data.push(5);
        x = data[0];
        data.push(4);
        y = data[1];
        data.push(3);
        l = data.length;
        z = data[2];
    }
}
// ====
// compileViaYul: also
// ----
// test() -> 5, 4, 3, 3
// gas irOptimized: 111317
// gas legacy: 111838
// gas legacyOptimized: 111128
