contract test {
    function f() pure public {
        { uint x; }
        uint x;
    }
}
// ----
// Warning: (57-63): Unused local variable.
// Warning: (75-81): Unused local variable.
