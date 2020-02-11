contract C {
    function f(uint a, uint8 b) public returns(uint) {
        a <<= b;
        return a;
    }
}

// ----
// f(uint256,uint8): 0x4266, 0 -> 0x4266
// f(uint256,uint8): 0x4266, 8 -> 0x426600
// f(uint256,uint8): 0x4266, 16 -> 0x42660000
// f(uint256,uint8): 0x4266, 17 -> 0x84cc0000
// f(uint256,uint8): 0x4266, 240 -> 0x4266000000000000000000000000000000000000000000000000000000000000
