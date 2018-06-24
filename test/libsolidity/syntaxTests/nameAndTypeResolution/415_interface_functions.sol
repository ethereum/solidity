interface I {
    function();
    function f();
}
// ----
// Warning: (18-29): Functions in interfaces should be declared external.
// Warning: (34-47): Functions in interfaces should be declared external.
// Warning: (18-29): No visibility specified. Defaulting to "public". In interfaces it defaults to external.
// Warning: (34-47): No visibility specified. Defaulting to "public". In interfaces it defaults to external.
