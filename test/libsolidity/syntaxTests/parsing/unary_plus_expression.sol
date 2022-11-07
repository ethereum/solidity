contract test {
    function f(uint x) pure public {
        uint y = +x;
        y;
    }
}
// ----
// SyntaxError 9636: (70-72): Use of unary + is disallowed.
// TypeError 4907: (70-72): Built-in unary operator + cannot be applied to type uint256.
