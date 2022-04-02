contract test {
    function f() pure public {
        { uint x; }
        uint x;
    }
}
// ----
// Warning 2519: (57-63='uint x'): This declaration shadows an existing declaration.
// Warning 2072: (57-63='uint x'): Unused local variable.
// Warning 2072: (75-81='uint x'): Unused local variable.
