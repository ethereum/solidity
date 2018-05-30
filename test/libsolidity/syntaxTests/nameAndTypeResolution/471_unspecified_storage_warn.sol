contract C {
    struct S { uint a; }
    S x;
    function f() view public {
        S y = x;
        y;
    }
}
// ----
// Warning: (86-89): Variable is declared as a storage pointer. Use an explicit "storage" keyword to silence this warning.
