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
// gas irOptimized: 2989821
// gas legacy: 3071205
// gas legacyOptimized: 3011873
// test_indices(uint256): 5 ->
// gas irOptimized: 371949
// gas legacy: 369241
// gas legacyOptimized: 366149
// test_indices(uint256): 10 ->
// test_indices(uint256): 15 ->
// gas irOptimized: 67954
// test_indices(uint256): 0xFF ->
// gas irOptimized: 3353039
// gas legacy: 3514167
// gas legacyOptimized: 3398107
// test_indices(uint256): 1000 ->
// gas irOptimized: 17981281
// gas legacy: 18617999
// gas legacyOptimized: 18178944
// test_indices(uint256): 129 ->
// gas irOptimized: 2730702
// gas legacy: 2772735
// gas legacyOptimized: 2716547
// test_indices(uint256): 128 ->
// gas irOptimized: 383265
// gas legacy: 467272
// gas legacyOptimized: 418424
// test_indices(uint256): 1 ->
// gas irOptimized: 368886
// gas legacy: 363407
// gas legacyOptimized: 361811
