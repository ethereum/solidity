interface I {
    event A();
    function f();
    function g();
    function();
}
contract C is I {
    function f() public {
    }
}
// ----
// Warning: (33-46): Functions in interfaces should be declared external.
// Warning: (51-64): Functions in interfaces should be declared external.
// Warning: (69-80): Functions in interfaces should be declared external.
// Warning: (33-46): No visibility specified. Defaulting to "public". In interfaces it defaults to external.
// Warning: (51-64): No visibility specified. Defaulting to "public". In interfaces it defaults to external.
// Warning: (69-80): No visibility specified. Defaulting to "public". In interfaces it defaults to external.
