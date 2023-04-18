contract test {
    function f(uint x) pure public {
        uint y = +x;
        y;
    }
}
// ----
// ParserError 9636: (70-71): Use of unary + is disallowed.
