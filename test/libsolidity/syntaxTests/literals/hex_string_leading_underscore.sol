contract C {
    function f() public pure {
        hex"_1234";
    }
}
// ----
// ParserError: (52-57): Invalid use of number separator '_'.
