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
// gas irOptimized: 3004881
// gas legacy: 3037923
// gas legacyOptimized: 2994071
// test_indices(uint256): 5 ->
// gas irOptimized: 372005
// gas legacy: 367951
// gas legacyOptimized: 365459
// test_indices(uint256): 10 ->
// test_indices(uint256): 15 ->
// gas irOptimized: 72860
// test_indices(uint256): 0xFF ->
// gas irOptimized: 3382967
// gas legacy: 3448377
// gas legacyOptimized: 3362917
// test_indices(uint256): 1000 ->
// gas irOptimized: 18099119
// gas legacy: 18359999
// gas legacyOptimized: 18040944
// test_indices(uint256): 129 ->
// gas irOptimized: 2743149
// gas legacy: 2739453
// gas legacyOptimized: 2698745
// test_indices(uint256): 128 ->
// gas irOptimized: 398204
// gas legacy: 434248
// gas legacyOptimized: 400760
// test_indices(uint256): 1 ->
// gas irOptimized: 368461
// gas legacy: 363149
// gas legacyOptimized: 361673
