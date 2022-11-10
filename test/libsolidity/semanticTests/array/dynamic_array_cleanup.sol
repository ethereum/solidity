contract c {
    uint[20] spacer;
    uint[] dynamic;
    function fill() public {
        for (uint i = 0; i < 21; ++i)
            dynamic.push(i + 1);
    }
    function halfClear() public {
        while (dynamic.length > 5)
            dynamic.pop();
    }
    function fullClear() public { delete dynamic; }
}
// ----
// storageEmpty -> 1
// fill() ->
// gas irOptimized: 519361
// gas legacy: 521584
// gas legacyOptimized: 517027
// storageEmpty -> 0
// halfClear() ->
// gas irOptimized: 113961
// gas legacy: 113257
// gas legacyOptimized: 113120
// storageEmpty -> 0
// fullClear() ->
// storageEmpty -> 1
