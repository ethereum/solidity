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
// compileViaYul: also
// ----
// pushEmpty(uint256): 128
// gas irOptimized: 412570
// gas legacy: 417287
// gas legacyOptimized: 397885
// pushEmpty(uint256): 256
// gas irOptimized: 702558
// gas legacy: 715083
// gas legacyOptimized: 686593
// pushEmpty(uint256): 38869 -> FAILURE # out-of-gas #
// gas irOptimized: 100000000
// gas legacy: 100000000
// gas legacyOptimized: 100000000
