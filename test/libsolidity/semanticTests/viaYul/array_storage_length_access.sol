contract C {
    uint[] storageArray;
    function set_get_length(uint256 len) public returns (uint256) {
        while(storageArray.length < len)
            storageArray.push();
        return storageArray.length;
    }
}
// ====
// compileViaYul: also
// ----
// set_get_length(uint256): 0 -> 0
// set_get_length(uint256): 1 -> 1
// set_get_length(uint256): 10 -> 10
// set_get_length(uint256): 20 -> 20
// set_get_length(uint256): 0xFF -> 0xFF
// gas ir: 917394
// gas irOptimized: 434473
// gas legacy: 619622
// gas legacyOptimized: 600751
// set_get_length(uint256): 0xFFF -> 0xFFF
// gas ir: 14623616
// gas irOptimized: 6743235
// gas legacy: 9765519
// gas legacyOptimized: 9461853
// set_get_length(uint256): 0xFFFFF -> FAILURE # Out-of-gas #
// gas ir: 100000000
// gas irOptimized: 100000000
// gas legacy: 100000000
// gas legacyOptimized: 100000000
