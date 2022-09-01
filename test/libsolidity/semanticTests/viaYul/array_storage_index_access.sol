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
// gas irOptimized: 3018684
// gas legacy: 3068883
// gas legacyOptimized: 3011615
// test_indices(uint256): 5 ->
// gas irOptimized: 372540
// gas legacy: 369151
// gas legacyOptimized: 366139
// test_indices(uint256): 10 ->
// test_indices(uint256): 15 ->
// gas irOptimized: 72860
// test_indices(uint256): 0xFF ->
// gas irOptimized: 3410252
// gas legacy: 3509577
// gas legacyOptimized: 3397597
// test_indices(uint256): 1000 ->
// gas irOptimized: 18206119
// gas legacy: 18599999
// gas legacyOptimized: 18176944
// test_indices(uint256): 129 ->
// gas irOptimized: 2756952
// gas legacy: 2770413
// gas legacyOptimized: 2716289
// test_indices(uint256): 128 ->
// gas irOptimized: 411900
// gas legacy: 464968
// gas legacyOptimized: 418168
// test_indices(uint256): 1 ->
// gas irOptimized: 368568
// gas legacy: 363389
// gas legacyOptimized: 361809
