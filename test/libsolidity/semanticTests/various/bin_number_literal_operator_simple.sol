contract test {
    function add() public pure returns (uint c) {
        c = 0b01 + 0b110;
    }
    function sub() public pure returns (uint c) {
        c = 0b0111110 - 0b110;
    }
    function mul() public pure returns (uint c) {
        c = 0b110 * 0b01011;
    }
    function div() public pure returns (uint c) {
        c = 0b10111 / uint256(0b11);
    }
    function mod() public pure returns (uint c) {
        c = 0b110101010 % 0b1011;
    }
    function xor() public pure returns (uint c) {
        c = 0b110101 ^ 0b001010;
    }
    function and() public pure returns (uint c) {
        c = 0b110010 & 0b010101;
    }
    function or() public pure returns (uint c) {
        c = 0b101010 | 0b110001;
    }
    function shiftleft() public pure returns (uint c) {
        c = 0b11111111 << 0b10000;
    }
    function shiftright() public pure returns (uint c) {
        c = 0b111111110000000000000000 >> 0b10000;
    }
    function compound_add() public pure returns (uint c) {
        c =  0b11110011000;
        c += 0b00001100111;
    }
    function compound_sub() public pure returns (uint c) {
        c =  0b1111111111;
        c -= 0b1010101010;
    }
    function compound_mul() public pure returns (uint c) {
        c =  0b110101011;
        c *= 0b10110101;
    }
}
// ----
// add() -> 0x7
// sub() -> 0x38
// mul() -> 0x42
// div() -> 0x7
// mod() -> 0x8
// xor() -> 0x3f
// and() -> 0x10
// or() -> 0x3b
// shiftleft() -> 0xff0000
// shiftright() -> 0xff
// compound_add() -> 0x7ff
// compound_sub() -> 0x155
// compound_mul() -> 0x12de7
