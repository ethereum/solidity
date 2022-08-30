contract X {
    function g(string calldata str) external {}

    function foo(string calldata code) external {
        this.g(code);
    }
}
// ----
// ParserError 3548: (95-99): Location already specified.
// ParserError 6933: (127-131): Expected primary expression.
