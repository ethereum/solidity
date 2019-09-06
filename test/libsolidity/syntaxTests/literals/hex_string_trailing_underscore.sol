contract C {
    function f() public pure {
        hex"1234_";
    }
}
// ----
// ParserError: (52-61): Invalid use of number separator '_'.
