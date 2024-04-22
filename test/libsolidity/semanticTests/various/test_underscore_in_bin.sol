contract test {
    function f(bool cond) public pure returns (uint256) {
        uint32 x = 0b0001001000110100_10101011;
        uint256 y = 0b0001001000110100_1010101111001101_0001001000110100;
        return cond ? x : y;
    }
}
// ----
// f(bool): true -> 0x1234ab
// f(bool): false -> 0x1234abcd1234
