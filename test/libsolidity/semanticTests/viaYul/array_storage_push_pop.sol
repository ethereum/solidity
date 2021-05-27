contract C {
    uint[] storageArray;
    function set_get_length(uint256 len) public returns (uint256) {
        while(storageArray.length < len)
            storageArray.push();
        while(storageArray.length > 0)
            storageArray.pop();
        return storageArray.length;
    }
}
// ====
// compileViaYul: also
// ----
// set_get_length(uint256): 0 -> 0
// set_get_length(uint256): 1 -> 0
// set_get_length(uint256): 10 -> 0
// set_get_length(uint256): 20 -> 0
// gas irOptimized: 88845
// gas legacy: 85817
// gas legacyOptimized: 83603
// set_get_length(uint256): 0xFF -> 0
// gas irOptimized: 852360
// gas legacy: 810322
// gas legacyOptimized: 786253
// set_get_length(uint256): 0xFFF -> 0
// gas irOptimized: 13328532
// gas legacy: 12649054
// gas legacyOptimized: 12267865
// set_get_length(uint256): 0xFFFF -> FAILURE # Out-of-gas #
