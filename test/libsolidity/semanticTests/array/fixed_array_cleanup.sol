contract c {
    uint spacer1;
    uint spacer2;
    uint[20] data;
    function fill() public {
        for (uint i = 0; i < data.length; ++i) data[i] = i+1;
    }
    function clear() public { delete data; }
}
// ====
// compileToEwasm: also
// ----
// storageEmpty -> 1
// fill() ->
// gas irOptimized: 465440
// gas legacy: 471400
// gas legacyOptimized: 467400
// storageEmpty -> 0
// clear() ->
// storageEmpty -> 1
