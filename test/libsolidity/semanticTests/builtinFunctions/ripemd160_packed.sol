contract C {
    function f(int256 _input) public returns (bytes32 hash) {
        uint24 b = 65536;
        uint c = 256;
        bytes32 input = bytes32(uint256(_input));
        return ripemd160(abi.encodePacked(uint8(8), input, b, input, c));
    }
}
// ----
// f(int256): 4 -> 0xf93175303eba2a7b372174fc9330237f5ad202fc000000000000000000000000
// f(int256): 5 -> 0x04f4fc112e2bfbe0d38f896a46629e08e2fcfad5000000000000000000000000
// f(int256): -1 -> 0xc0a2e4b1f3ff766a9a0089e7a410391730872495000000000000000000000000
