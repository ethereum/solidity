contract C {
    function f(uint256 a, uint8 b) public returns (uint256) {
        a <<= b;
        return a;
    }
}
// ----
// f(uint256,uint8): 0x4266, 0x0 -> 0x4266
// f(uint256,uint8): 0x4266, 0x8 -> 0x426600
// f(uint256,uint8): 0x4266, 0x10 -> 0x42660000
// f(uint256,uint8): 0x4266, 0x11 -> 0x84cc0000
// f(uint256,uint8): 0x4266, 0xf0 -> 0x4266000000000000000000000000000000000000000000000000000000000000
