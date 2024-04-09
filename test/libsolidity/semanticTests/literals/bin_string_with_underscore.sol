contract C {
    function f() public pure returns(bytes memory) {
        return bin"00010010_00110100_0101011001111000_10011010";
    }
}
// ----
// f() -> 32, 5, left(0b0001001000110100010101100111100010011010)
