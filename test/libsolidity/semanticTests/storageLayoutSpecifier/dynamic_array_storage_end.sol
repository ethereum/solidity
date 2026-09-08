contract C layout at 2**256 - 2 {
    uint[] array;

    function init() public {
        for (uint i = 0; i < 1000; ++i)
            array.push(i + 1);
    }
    function validate() public {
        for (uint i = 0; i < 1000; ++i)
            require(array[i] == i + 1);
    }
    function clear() public {
        for (uint i = 0; i < 1000; ++i)
            array.pop();
    }
}
// ----
// init() ->
// gas irOptimized: 22738148
// gas legacy: 22699167
// gas legacyOptimized: 22541160
// gas ssaCFGOptimized: 22735147
// validate() ->
// gas irOptimized: 2444229
// gas legacy: 2560245
// gas legacyOptimized: 2442238
// gas ssaCFGOptimized: 2444229
// clear() ->
// gas irOptimized: 4449606
// gas legacy: 4300019
// gas legacyOptimized: 4302413
// gas ssaCFGOptimized: 4449606
