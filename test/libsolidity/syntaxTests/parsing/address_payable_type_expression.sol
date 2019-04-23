contract C {
    function f() public pure {
        address payable;
    }
}
// The second error message below is a result of
// Parser::expectIdentifierToken() advancing over ';'
// even when expectToken() fails. We could remove this message
// by not having it advance on error but instead substituting in
// a bogus identifier value.
// ----
// ParserError: (67-68): Expected identifier but got ';'
// ParserError: (73-74): Expected ';' but got '}'
