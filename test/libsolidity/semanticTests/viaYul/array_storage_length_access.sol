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
// gas irOptimized: 96690
// gas legacy: 126722
// gas legacyOptimized: 107807
// set_get_length(uint256): 0xFFF -> 0xFFF
// gas irOptimized: 1217857
// gas legacy: 1702119
// gas legacyOptimized: 1398409
// set_get_length(uint256): 0xFFFFF -> FAILURE # Out-of-gas #
