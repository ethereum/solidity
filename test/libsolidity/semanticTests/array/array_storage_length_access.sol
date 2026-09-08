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
// gas irOptimized: 98761
// gas legacy: 128571
// gas legacyOptimized: 110143
// set_get_length(uint256): 0xFFF -> 0xFFF
// gas irOptimized: 1209113
// gas legacy: 1689548
// gas legacyOptimized: 1393535
// gas ssaCFGOptimized: 1197595
// set_get_length(uint256): 0xFFFFF -> FAILURE # Out-of-gas #
