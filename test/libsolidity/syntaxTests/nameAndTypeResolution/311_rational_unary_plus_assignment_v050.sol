pragma experimental "v0.5.0";
contract test {
    function f(uint x) pure public {
        uint y = +x;
        y;
    }
}
// ----
// SyntaxError: (100-102): Use of unary + is deprecated.
