contract c {
    uint16[] data;
    function test() public returns (uint16 x, uint16 y, uint16 z) {
        for (uint i = 1; i <= 48; i++)
            data.push(uint16(i));
        for (uint j = 1; j <= 10; j++)
            data.pop();
        x = data[data.length - 1];
        for (uint k = 1; k <= 10; k++)
            data.pop();
        y = data[data.length - 1];
        for (uint l = 1; l <= 10; l++)
            data.pop();
        z = data[data.length - 1];
        for (uint m = 1; m <= 18; m++)
            data.pop();
    }
}
// ====
// compileViaYul: also
// ----
// test() -> 38, 28, 18
// gas irOptimized: 188649
// gas legacy: 189780
// gas legacyOptimized: 178267
// storageEmpty -> 1
