contract C {
    function f() public pure {
        uint transient x = 0;
    }
}
// ----
// ParserError 2314: (67-68): Expected ';' but got identifier
