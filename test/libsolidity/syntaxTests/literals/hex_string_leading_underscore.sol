contract C {
    function f() public pure {
        hex"_1234";
    }
}
// ----
// ParserError 8936: (52-57='hex"_'): Invalid use of number separator '_'.
