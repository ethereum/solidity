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
// storage: empty
// fill() ->
// gas ir: 556824
// gas irOptimized: 536238
// gas legacy: 504373
// gas legacyOptimized: 499648
// storage: nonempty
// halfClear() ->
// gas ir: 116943
// storage: nonempty
// fullClear() ->
// storage: empty
