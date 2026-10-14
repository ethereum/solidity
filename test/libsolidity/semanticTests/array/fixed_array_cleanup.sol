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
// gas irOptimized: 465010
// gas legacy: 468825
// gas legacyOptimized: 466238
// gas ssaCFGOptimized: 464889
// storageEmpty -> 0
// clear() ->
// gas irOptimized: 97798
// gas legacy: 97944
// gas legacyOptimized: 97880
// storageEmpty -> 1
