contract C {
    function f() public pure {
        hex"1234_";
    }
}
// ----
// ParserError 8936: (52-61='hex"1234_'): Invalid use of number separator '_'.
