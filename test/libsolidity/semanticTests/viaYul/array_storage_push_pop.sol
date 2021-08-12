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
// gas irOptimized: 86331
// gas legacy: 85822
// gas legacyOptimized: 83597
// set_get_length(uint256): 0xFF -> 0
// gas irOptimized: 821881
// gas legacy: 810327
// gas legacyOptimized: 786247
// set_get_length(uint256): 0xFFF -> 0
// gas irOptimized: 12841093
// gas legacy: 12649059
// gas legacyOptimized: 12267859
// set_get_length(uint256): 0xFFFF -> FAILURE # Out-of-gas #
