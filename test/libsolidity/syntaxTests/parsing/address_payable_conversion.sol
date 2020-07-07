contract C {
    function f() public pure {
        address payable a = address payable(this);
    }
}
// ----
// ParserError 2314: (80-87): Expected ';' but got 'payable'
