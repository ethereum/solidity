contract C {
    function f() public pure {
        bin"11001100_";
    }
}
// ----
// ParserError 8936: (52-65): Invalid use of number separator '_'.
