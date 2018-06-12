contract C {
    function g() pure public returns (int, int, int) {}
    function f() pure public {
        // ensure this test is failing
        (,,) = g();
    }
}
// ----
// ParserError: (150-151): Expected primary expression.
