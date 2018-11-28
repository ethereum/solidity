contract test {
    function f(uint x) pure public {
        uint y = +x;
        y;
    }
}
// ----
// SyntaxError: (70-72): Use of unary + is disallowed.
// TypeError: (70-72): Unary operator + cannot be applied to type uint256
