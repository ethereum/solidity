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
// ====
// compileViaYul: also
// ----
// storageEmpty -> 1
// fill() ->
// gas irOptimized: 519848
// gas legacy: 521773
// gas legacyOptimized: 516733
// storageEmpty -> 0
// halfClear() ->
// storageEmpty -> 0
// fullClear() ->
// storageEmpty -> 1
