pragma experimental "v0.5.0";
contract test {
    function f() pure public {
        ufixed16x2 a = +3.25;
        fixed16x2 b = -3.25;
        a; b;
    }
}
// ----
// SyntaxError: (100-105): Use of unary + is deprecated.
