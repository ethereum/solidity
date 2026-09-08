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
// gas irOptimized: 519491
// gas legacy: 518943
// gas legacyOptimized: 515555
// gas ssaCFGOptimized: 519427
// storageEmpty -> 0
// halfClear() ->
// gas irOptimized: 91469
// gas legacy: 90567
// gas legacyOptimized: 90457
// storageEmpty -> 0
// fullClear() ->
// storageEmpty -> 1
