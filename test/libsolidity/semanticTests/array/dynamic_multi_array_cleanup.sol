contract c {
    struct s { uint[][] d; }
    s[] data;
    function fill() public returns (uint) {
        while (data.length < 3)
            data.push();
        while (data[2].d.length < 4)
            data[2].d.push();
        while (data[2].d[3].length < 5)
            data[2].d[3].push();
        data[2].d[3][4] = 8;
        return data[2].d[3][4];
    }
    function clear() public { delete data; }
}
// ----
// storageEmpty -> 1
// fill() -> 8
// gas irOptimized: 122742
// gas legacy: 121602
// gas legacyOptimized: 120589
// storageEmpty -> 0
// clear() ->
// storageEmpty -> 1
