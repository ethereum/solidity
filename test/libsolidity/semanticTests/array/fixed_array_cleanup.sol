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
// compileViaYul: also
// ----
// storageEmpty -> 1
// fill() ->
// gas irOptimized: 465585
// gas legacy: 471460
// gas legacyOptimized: 467220
// storageEmpty -> 0
// clear() ->
// storageEmpty -> 1
