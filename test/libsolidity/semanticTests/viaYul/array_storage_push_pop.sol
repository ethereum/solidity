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
// gas irOptimized: 162373
// gas legacy: 141922
// gas legacyOptimized: 139708
// set_get_length(uint256): 0xFF -> 0
// gas irOptimized: 1787868
// gas legacy: 1524427
// gas legacyOptimized: 1500358
// set_get_length(uint256): 0xFFF -> 0
// gas irOptimized: 28349160
// gas legacy: 24115159
// gas legacyOptimized: 23733970
// set_get_length(uint256): 0xFFFF -> FAILURE # Out-of-gas #
