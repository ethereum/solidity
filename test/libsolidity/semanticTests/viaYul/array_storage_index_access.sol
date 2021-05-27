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
// ====
// compileViaYul: also
// ----
// test_indices(uint256): 1 ->
// test_indices(uint256): 129 ->
// gas irOptimized: 3070168
// gas legacy: 3071200
// gas legacyOptimized: 3011868
// test_indices(uint256): 5 ->
// gas irOptimized: 373956
// gas legacy: 369236
// gas legacyOptimized: 366144
// test_indices(uint256): 10 ->
// test_indices(uint256): 15 ->
// gas irOptimized: 76670
// test_indices(uint256): 0xFF ->
// gas irOptimized: 3511240
// gas legacy: 3514162
// gas legacyOptimized: 3398102
// test_indices(uint256): 1000 ->
// gas irOptimized: 18591162
// gas legacy: 18617994
// gas legacyOptimized: 18178939
// test_indices(uint256): 129 ->
// gas irOptimized: 2798783
// gas legacy: 2772730
// gas legacyOptimized: 2716542
// test_indices(uint256): 128 ->
// gas irOptimized: 455997
// gas legacy: 467267
// gas legacyOptimized: 418419
// test_indices(uint256): 1 ->
// gas irOptimized: 368599
// gas legacy: 363402
// gas legacyOptimized: 361806
