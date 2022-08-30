contract test {
    function f() public pure {
        fixed a = 0.42578125 % -0.4271087646484375;
        fixed b = .5 % a;
        fixed c = a % b;
        a; b; c;
    }
}
// ----
// TypeError 2271: (117-123): Binary operator % not compatible with types rational_const 1 / 2 and fixed128x18. Fractional literals not supported.
