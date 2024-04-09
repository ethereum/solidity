contract test {
    function f() public pure returns (bytes32) {
        bytes32 escapeCharacters = bin"0000000000000000"
        bin"1101111010101111"
        bin"1111111011101101"
        bin"10101011";
        return escapeCharacters;
    }
}
// ----
