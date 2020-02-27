contract test {
    mapping(uint256 => uint256) public m1;
    mapping(uint256 => mapping(uint256 => uint256)) public m2;
    function set(uint256 k, uint256 v) public {
        m1[k] = v;
    }
    function set(uint256 k1, uint256 k2, uint256 v) public {
        m2[k1][k2] = v;
    }
}
// ====
// compileViaYul: also
// ----
// m1(uint256): 0 -> 0
// m1(uint256): 0x01 -> 0
// m1(uint256): 0xa7 -> 0
// set(uint256,uint256): 0x01, 0xa1 ->
// m1(uint256): 0 -> 0
// m1(uint256): 0x01 -> 0xa1
// m1(uint256): 0xa7 -> 0
// set(uint256,uint256): 0x00, 0xef ->
// m1(uint256): 0 -> 0xef
// m1(uint256): 0x01 -> 0xa1
// m1(uint256): 0xa7 -> 0
// set(uint256,uint256): 0x01, 0x05 ->
// m1(uint256): 0 -> 0xef
// m1(uint256): 0x01 -> 0x05
// m1(uint256): 0xa7 -> 0
// m2(uint256,uint256): 0, 0 -> 0
// m2(uint256,uint256): 0, 0x01 -> 0
// m2(uint256,uint256): 0xa7, 0 -> 0
// m2(uint256,uint256): 0xa7, 0x01 -> 0
// set(uint256,uint256,uint256): 0xa7, 0x01, 0x23
// m2(uint256,uint256): 0, 0x01 -> 0
// m2(uint256,uint256): 0xa7, 0 -> 0
// m2(uint256,uint256): 0xa7, 0x01 -> 0x23
// set(uint256,uint256,uint256): 0, 0x01, 0xef
// m2(uint256,uint256): 0, 0x01 -> 0xef
// m2(uint256,uint256): 0xa7, 0 -> 0
// m2(uint256,uint256): 0xa7, 0x01 -> 0x23
