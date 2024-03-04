contract c {
    uint spacer1;
    uint spacer2;
    uint[20] data;
    function fill() public {
        for (uint i = 0; i < data.length; ++i) data[i] = i+1;
    }
    function clear() public { delete data; }
}
// ----
// storageEmpty -> 1
// fill() ->
// gas irOptimized: 464834
// gas legacy: 468818
// gas legacyOptimized: 466238
// storageEmpty -> 0
// clear() ->
// gas irOptimized: 122148
// gas legacy: 122440
// gas legacyOptimized: 122259
// storageEmpty -> 1
