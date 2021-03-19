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
// gas irOptimized: 629687
// gas legacy: 607287
// gas legacyOptimized: 589048
// pushEmpty(uint256): 256
// gas irOptimized: 859191
// gas legacy: 828983
// gas legacyOptimized: 802808
// pushEmpty(uint256): 32768 -> FAILURE # out-of-gas #
