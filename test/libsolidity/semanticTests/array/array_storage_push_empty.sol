contract C {
    uint256[] storageArray;
    function pushEmpty(uint256 len) public {
        while(storageArray.length < len)
            storageArray.push();

        for (uint i = 0; i < len; i++)
            require(storageArray[i] == 0);
    }
}
// ====
// EVMVersion: >=petersburg
// ----
// pushEmpty(uint256): 128
// gas irOptimized: 406413
// gas legacy: 415744
// gas legacyOptimized: 397380
// pushEmpty(uint256): 256
// gas irOptimized: 693185
// gas legacy: 715316
// gas legacyOptimized: 688632
// pushEmpty(uint256): 38869 -> FAILURE # out-of-gas #
// gas irOptimized: 100000000
// gas legacy: 100000000
// gas legacyOptimized: 100000000
