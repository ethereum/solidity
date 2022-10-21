contract C {
    function f(int256 input) public returns (bytes32 sha256hash) {
        return ripemd160(abi.encodePacked(bytes32(uint256(input))));
    }
}
// ----
// f(int256): 4 -> 0x1b0f3c404d12075c68c938f9f60ebea4f74941a0000000000000000000000000
// f(int256): 5 -> 0xee54aa84fc32d8fed5a5fe160442ae84626829d9000000000000000000000000
// f(int256): -1 -> 0x1cf4e77f5966e13e109703cd8a0df7ceda7f3dc3000000000000000000000000
