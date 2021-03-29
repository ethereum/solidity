contract C {
    address[] addressArray;
    function set_get_length(uint256 len) public returns (uint256)
    {
        while(addressArray.length < len)
            addressArray.push();
        while(addressArray.length > len)
            addressArray.pop();
        return addressArray.length;
    }
}
// ====
// EVMVersion: >=petersburg
// compileViaYul: also
// ----
// set_get_length(uint256): 0 -> 0
// set_get_length(uint256): 1 -> 1
// set_get_length(uint256): 10 -> 10
// set_get_length(uint256): 20 -> 20
// set_get_length(uint256): 0 -> 0
// gas irOptimized: 110050
// gas legacy: 107830
// gas legacyOptimized: 107262
// set_get_length(uint256): 0xFF -> 0xFF
// gas irOptimized: 700302
// gas legacy: 882337
// gas legacyOptimized: 650704
// set_get_length(uint256): 0xFFF -> 0xFFF
// gas irOptimized: 10207734
// gas legacy: 12945874
// gas legacyOptimized: 9462646
// set_get_length(uint256): 0xFFFF -> FAILURE # Out-of-gas #
