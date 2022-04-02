contract C {
    function f(bool b) public pure
    {
        if â€¬(b) { return; }
    }
}
// ----
// ParserError 2314: (65-66='â'): Expected '(' but got 'ILLEGAL'
