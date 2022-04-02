contract test {
    function f() pure public {
        { uint x; }
        { uint x; }
    }
}
// ----
// Warning 2072: (57-63='uint x'): Unused local variable.
// Warning 2072: (77-83='uint x'): Unused local variable.
