contract test {
    function f() pure public {
        ufixed16x2 a = +3.25;
        fixed16x2 b = -3.25;
        a; b;
    }
}
// ----
// SyntaxError 9636: (70-75): Use of unary + is disallowed.
// TypeError 4907: (70-75): Unary operator + cannot be applied to type rational_const 13 / 4.
