contract C {
    function f() public pure {
        bin"01010101__10101010";
    }
}
// ----
// ParserError 8936: (52-66): Invalid use of number separator '_'.
