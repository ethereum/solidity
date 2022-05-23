contract C {
    uint[] storageArray;
    function test_indices(uint256 len) public
    {
        while (storageArray.length < len)
            storageArray.push();
        while (storageArray.length > len)
            storageArray.pop();
        for (uint i = 0; i < len; i++)
            storageArray[i] = i + 1;

        for (uint i = 0; i < len; i++)
            require(storageArray[i] == i + 1);
    }
}
// ----
// test_indices(uint256): 1 ->
// test_indices(uint256): 129 ->
// gas irOptimized: 3018687
// gas legacy: 3070431
// gas legacyOptimized: 3010325
// test_indices(uint256): 5 ->
// gas irOptimized: 372543
// gas legacy: 369211
// gas legacyOptimized: 366089
// test_indices(uint256): 10 ->
// test_indices(uint256): 15 ->
// gas irOptimized: 72860
// test_indices(uint256): 0xFF ->
// gas irOptimized: 3410255
// gas legacy: 3512637
// gas legacyOptimized: 3395047
// test_indices(uint256): 1000 ->
// gas irOptimized: 18206122
// gas legacy: 18611999
// gas legacyOptimized: 18166944
// test_indices(uint256): 129 ->
// gas irOptimized: 2756955
// gas legacy: 2771961
// gas legacyOptimized: 2714999
// test_indices(uint256): 128 ->
// gas irOptimized: 411903
// gas legacy: 466504
// gas legacyOptimized: 416888
// test_indices(uint256): 1 ->
// gas irOptimized: 368571
// gas legacy: 363401
// gas legacyOptimized: 361799
