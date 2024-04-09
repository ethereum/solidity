contract C {
    function f() public pure {
        bin"101_01010";
    }
}
// ----
// ParserError 8936: (52-56): Expected numbers of bits multiple of 8.
