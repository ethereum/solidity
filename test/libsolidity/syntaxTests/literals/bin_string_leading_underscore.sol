contract C {
    function f() public pure {
        bin"_11001100";
    }
}
// ----
// ParserError 8936: (52-57): Invalid use of number separator '_'.
