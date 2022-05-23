contract c {
    uint256 a;
    uint256 b;
    uint256 c;
    uint16[] inner = [1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15, 16];
    uint16[][] data;
    function test() public returns (uint x, uint y, uint z) {
        for (uint i = 1; i <= 48; i++)
            data.push(inner);
        for (uint j = 1; j <= 10; j++)
            data.pop();
        x = data[data.length - 1][0];
        for (uint k = 1; k <= 10; k++)
            data.pop();
        y = data[data.length - 1][1];
        for (uint l = 1; l <= 10; l++)
            data.pop();
        z = data[data.length - 1][2];
        for (uint m = 1; m <= 18; m++)
            data.pop();
        delete inner;
    }
}
// ----
// test() -> 1, 2, 3
// gas irOptimized: 2280132
// gas legacy: 2273434
// gas legacyOptimized: 2261820
// storageEmpty -> 1
