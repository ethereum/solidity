contract C {
    uint[] storageArray;
    function set_get_length(uint256 len) public returns (uint256) {
        while(storageArray.length < len)
            storageArray.push();
        return storageArray.length;
    }
}
// ----
// set_get_length(uint256): 0 -> 0
// set_get_length(uint256): 1 -> 1
// set_get_length(uint256): 10 -> 10
// set_get_length(uint256): 20 -> 20
// set_get_length(uint256): 0xFF -> 0xFF
// gas irOptimized: 96690
// gas legacy: 129522
// gas legacyOptimized: 110618
// set_get_length(uint256): 0xFFF -> 0xFFF
// gas irOptimized: 1220648
// gas legacy: 1704919
// gas legacyOptimized: 1401220
// set_get_length(uint256): 0xFFFFF -> FAILURE # Out-of-gas #
