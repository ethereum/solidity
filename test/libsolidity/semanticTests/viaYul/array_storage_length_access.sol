contract C {
    uint[] storageArray;
    function set_get_length(uint256 len) public returns (uint256)
    {
        storageArray = new uint[](len);
        return storageArray.length;
    }
}
// ====
// compileViaYul: true
// ----
// set_get_length(uint256): 0 -> 0
// set_get_length(uint256): 1 -> 1
// set_get_length(uint256): 10 -> 10
// set_get_length(uint256): 20 -> 20
// set_get_length(uint256): 0 -> 0
// set_get_length(uint256): 0xFF -> 0xFF
// set_get_length(uint256): 0xFFF -> 0xFFF
// set_get_length(uint256): 0xFFFF -> FAILURE # Out-of-gas #
