contract test {
    function f() pure public {
        { uint x; }
        { uint x; }
    }
}
// ----
// Warning: (57-63): Unused local variable.
// Warning: (77-83): Unused local variable.
