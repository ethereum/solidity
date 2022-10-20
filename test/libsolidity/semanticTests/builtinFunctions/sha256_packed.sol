contract C {
    function f(int256 _input) public returns (bytes32 hash) {
        uint24 b = 65536;
        uint c = 256;
        bytes32 input = bytes32(uint256(_input));
        return sha256(abi.encodePacked(uint8(8), input, b, input, c));
    }
}
// ----
// f(int256): 4 -> 0x804e0d7003cfd70fc925dc103174d9f898ebb142ecc2a286da1abd22ac2ce3ac
// f(int256): 5 -> 0xe94921945f9068726c529a290a954f412bcac53184bb41224208a31edbf63cf0
// f(int256): -1 -> 0xf14def4d07cd185ddd8b10a81b2238326196a38867e6e6adbcc956dc913488c7
