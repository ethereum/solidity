contract test {
    function f() pure public {
        { uint x; }
        uint x;
    }
}
// ----
// Warning 2519: (57-63): This declaration shadows an existing declaration.
// Warning 2072: (57-63): Unused local variable.
// Warning 2072: (75-81): Unused local variable.
