contract C {
    function f() public pure {
        hex"1_234";
    }
}
// ----
// ParserError 8936: (52-56='hex"'): Expected even number of hex-nibbles.
