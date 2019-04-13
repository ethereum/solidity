contract C {
    function f() public pure {
        address payable a = address payable(this);
    }
}
// ----
// ParserError: (93-94): Expected ';' but got 'payable'; deleted tokens to the next expected token.
