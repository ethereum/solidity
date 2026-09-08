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
// ----
// set_get_length(uint256): 0 -> 0
// set_get_length(uint256): 1 -> 0
// set_get_length(uint256): 10 -> 0
// set_get_length(uint256): 20 -> 0
// gas irOptimized: 87223
// gas legacy: 85609
// gas legacyOptimized: 83462
// set_get_length(uint256): 0xFF -> 0
// gas irOptimized: 833583
// gas legacy: 807764
// gas legacyOptimized: 784467
// gas ssaCFGOptimized: 835115
// set_get_length(uint256): 0xFFF -> 0
// gas irOptimized: 13029435
// gas legacy: 12608096
// gas legacyOptimized: 12239199
// gas ssaCFGOptimized: 13054007
// set_get_length(uint256): 0xFFFF -> FAILURE # Out-of-gas #
