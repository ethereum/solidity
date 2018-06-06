pragma experimental "v0.5.0";
contract C {
    struct S { uint a; }
    S x;
    function f() view public {
        S y = x;
        y;
    }
}
// ----
// TypeError: (116-119): Data location must be specified as either "memory" or "storage".
