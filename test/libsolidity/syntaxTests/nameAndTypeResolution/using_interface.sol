interface I {
    function f();
}
contract C is I {
    function f() public {
    }
}
// ----
// Warning: (18-31): Functions in interfaces should be declared external.
// Warning: (18-31): No visibility specified. Defaulting to "public". In interfaces it defaults to external.
