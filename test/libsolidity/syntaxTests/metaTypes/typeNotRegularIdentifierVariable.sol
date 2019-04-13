contract Test {
    function f() public pure {
        uint type;
    }
}
// ----
// ParserError: (64-65): Expected ';' but got 'type'; deleted tokens to the next expected token.
