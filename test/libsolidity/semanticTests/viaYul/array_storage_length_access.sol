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
// gas irOptimized: 434427
// gas legacy: 619622
// gas legacyOptimized: 600718
// set_get_length(uint256): 0xFFF -> 0xFFF
// gas irOptimized: 6743189
// gas legacy: 9765519
// gas legacyOptimized: 9461820
// set_get_length(uint256): 0xFFFFF -> FAILURE # Out-of-gas #
