contract test {
    function f() pure public {
        { uint x; }
        { uint x; }
    }
}
// ----
// Warning 2072: (57-63): Unused local variable.
// Warning 2072: (77-83): Unused local variable.
