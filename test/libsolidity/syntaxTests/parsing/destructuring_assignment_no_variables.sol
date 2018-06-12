contract C {
    // ensures solc fails on multi-assignment without named variables.
    function g() pure public returns (int, int, int) {}
    function f() pure public {
        (,,) = g();
    }
}
// ----
// ParserError: (182-183): Expected primary expression.
