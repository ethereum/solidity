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
// gas irOptimized: 410742
// gas legacy: 400519
// gas legacyOptimized: 388804
// gas ssaCFGOptimized: 410739
// pushEmpty(uint256): 256
// gas irOptimized: 698282
// gas legacy: 684859
// gas legacyOptimized: 671480
// gas ssaCFGOptimized: 698279
// pushEmpty(uint256): 38869 -> FAILURE # out-of-gas #
// gas irOptimized: 100000000
// gas legacy: 100000000
// gas legacyOptimized: 100000000
