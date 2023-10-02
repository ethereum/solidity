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
// gas irOptimized: 3016570
// gas legacy: 3069098
// gas legacyOptimized: 3013250
// test_indices(uint256): 5 ->
// gas irOptimized: 576716
// gas legacy: 574754
// gas legacyOptimized: 572383
// test_indices(uint256): 10 ->
// gas irOptimized: 158059
// gas legacy: 162468
// gas legacyOptimized: 158336
// test_indices(uint256): 15 ->
// gas irOptimized: 172984
// gas legacy: 179513
// gas legacyOptimized: 173606
// test_indices(uint256): 0xFF ->
// gas irOptimized: 5672104
// gas legacy: 5775928
// gas legacyOptimized: 5666726
// test_indices(uint256): 1000 ->
// gas irOptimized: 18173701
// gas legacy: 18583810
// gas legacyOptimized: 18171248
// test_indices(uint256): 129 ->
// gas irOptimized: 4147676
// gas legacy: 4164468
// gas legacyOptimized: 4122100
// test_indices(uint256): 128 ->
// gas irOptimized: 409209
// gas legacy: 463706
// gas legacyOptimized: 418061
// test_indices(uint256): 1 ->
// gas irOptimized: 580316
// gas legacy: 576904
// gas legacyOptimized: 575649
