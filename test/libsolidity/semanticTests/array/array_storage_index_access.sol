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
// gas irOptimized: 3017684
// gas legacy: 3038668
// gas legacyOptimized: 2995964
// gas ssaCFGOptimized: 3017289
// test_indices(uint256): 5 ->
// gas irOptimized: 579668
// gas legacy: 573821
// gas legacyOptimized: 571847
// gas ssaCFGOptimized: 580244
// test_indices(uint256): 10 ->
// gas irOptimized: 157950
// gas legacy: 160122
// gas legacyOptimized: 156996
// gas ssaCFGOptimized: 157912
// test_indices(uint256): 15 ->
// gas irOptimized: 172730
// gas legacy: 175987
// gas legacyOptimized: 171596
// gas ssaCFGOptimized: 172677
// test_indices(uint256): 0xFF ->
// gas irOptimized: 5673820
// gas legacy: 5715762
// gas legacyOptimized: 5632556
// gas ssaCFGOptimized: 5673047
// test_indices(uint256): 1000 ->
// gas irOptimized: 18173002
// gas legacy: 18347824
// gas legacyOptimized: 18037248
// gas ssaCFGOptimized: 18169994
// test_indices(uint256): 129 ->
// gas irOptimized: 4166276
// gas legacy: 4140124
// gas legacyOptimized: 4108272
// gas ssaCFGOptimized: 4170141
// test_indices(uint256): 128 ->
// gas irOptimized: 405519
// gas legacy: 433512
// gas legacyOptimized: 400909
// gas ssaCFGOptimized: 405133
// test_indices(uint256): 1 ->
// gas irOptimized: 583435
// gas legacy: 576726
// gas legacyOptimized: 575542
// gas ssaCFGOptimized: 584036
