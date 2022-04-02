function f() public pure {
    revert 1;
}
// ----
// ParserError 2314: (38-39='1'): Expected ';' but got 'Number'
