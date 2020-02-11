contract C {
    function f(uint a, uint b) public returns(uint) {
        a >>= b;
        return a;
    }
}

// ----
// f(uint256,uint256): 0x4266, 0 -> 0x4266
// f(uint256,uint256): 0x4266, 8 -> 0x42
// f(uint256,uint256): 0x4266, 16 -> 0
// f(uint256,uint256): 0x4266, 17 -> 0
