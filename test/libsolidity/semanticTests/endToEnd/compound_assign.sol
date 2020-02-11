contract test {
    uint value1;
    uint value2;

    function f(uint x, uint y) public returns(uint w) {
        uint value3 = y;
        value1 += x;
        value3 *= x;
        value2 *= value3 + value1;
        return value2 += 7;
    }
}

// ====
// compileViaYul: also
// ----
// f(uint256,uint256): 0x0, 0x6 -> 0x7
// f(uint256,uint256): 0x1, 0x3 -> 0x23
// f(uint256,uint256): 0x2, 0x19 -> 0x746
// f(uint256,uint256): 0x3, 0x45 -> 0x60d45
// f(uint256,uint256): 0x4, 0x54 -> 0x82def49
// f(uint256,uint256): 0x5, 0x2 -> 0xcc7c5e28
// f(uint256,uint256): 0x6, 0x33 -> 0x10532dc451f
// f(uint256,uint256): 0x7, 0x30 -> 0x173645132481b
