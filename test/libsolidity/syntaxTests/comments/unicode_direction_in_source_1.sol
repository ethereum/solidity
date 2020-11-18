contract C {
    function f(bool b) public pure
    {
        if ‬(b) { return; }
    }
}
// ----
// ParserError 2314: (65-66): Expected '(' but got 'ILLEGAL'
