contract c {
    uint256 a;
    uint256 b;
    uint256 c;
    uint24[] data;
    function test() public returns (uint24 x, uint24 y) {
        for (uint i = 1; i <= 30; i++)
            data.push(uint24(i));
        for (uint j = 1; j <= 10; j++)
            data.pop();
        x = data[data.length - 1];
        for (uint k = 1; k <= 10; k++)
            data.pop();
        y = data[data.length - 1];
        for (uint l = 1; l <= 10; l++)
            data.pop();
    }
}
// ====
// compileViaYul: also
// ----
// test() -> 20, 10
// gas irOptimized: 159175
// gas legacy: 159459
// gas legacyOptimized: 152903
// storageEmpty -> 1
