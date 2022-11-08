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
// gas irOptimized: 3021484
// gas legacy: 3071683
// gas legacyOptimized: 3014415
// test_indices(uint256): 5 ->
// gas irOptimized: 722540
// gas legacy: 719151
// gas legacyOptimized: 716139
// test_indices(uint256): 10 ->
// gas irOptimized: 158407
// gas legacy: 162657
// gas legacyOptimized: 158422
// test_indices(uint256): 15 ->
// gas irOptimized: 173467
// gas legacy: 179782
// gas legacyOptimized: 173727
// test_indices(uint256): 0xFF ->
// gas irOptimized: 5681652
// gas legacy: 5780977
// gas legacyOptimized: 5668997
// test_indices(uint256): 1000 ->
// gas irOptimized: 18208919
// gas legacy: 18602799
// gas legacyOptimized: 18179744
// test_indices(uint256): 129 ->
// gas irOptimized: 5198552
// gas legacy: 5212013
// gas legacyOptimized: 5157889
// test_indices(uint256): 128 ->
// gas irOptimized: 417500
// gas legacy: 470568
// gas legacyOptimized: 423768
// test_indices(uint256): 1 ->
// gas irOptimized: 726968
// gas legacy: 721789
// gas legacyOptimized: 720209
