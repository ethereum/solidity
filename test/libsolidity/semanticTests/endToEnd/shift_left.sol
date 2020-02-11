contract C {
    function f(uint a, uint b) public returns(uint) {
        return a << b;
    }
}

// ----
// f(uint256,uint256): 0x4266, 0 -> 0x4266
// f(uint256,uint256): 0x4266, 8 -> 0x426600
// f(uint256,uint256): 0x4266, 16 -> 0x42660000
// f(uint256,uint256): 0x4266, 16 -> 0x42660000
// f(uint256,uint256): 0x4266, 17 -> 0x84CC0000
// f(uint256,uint256): 0x4266, 240 -> 0x4266000000000000000000000000000000000000000000000000000000000000
// f(uint256,uint256): 0x4266, 256 -> 0
