contract test {
    mapping(uint => mapping(uint => uint)) table;

    function f(uint x, uint y, uint z) public returns(uint w) {
        if (z == 0) return table[x][y];
        else return table[x][y] = z;
    }
}

// ====
// compileViaYul: also
// ----
// f(uint256,uint256,uint256): 0x4, 0x5, 0x0 -> 0x0
// f(uint256,uint256,uint256): 0x5, 0x4, 0x0 -> 0x0
// f(uint256,uint256,uint256): 0x4, 0x5, 0x9 -> 0x9
// f(uint256,uint256,uint256): 0x4, 0x5, 0x0 -> 0x9
// f(uint256,uint256,uint256): 0x5, 0x4, 0x0 -> 0x0
// f(uint256,uint256,uint256): 0x5, 0x4, 0x7 -> 0x7
// f(uint256,uint256,uint256): 0x4, 0x5, 0x0 -> 0x9
// f(uint256,uint256,uint256): 0x5, 0x4, 0x0 -> 0x7
