{
    function f() -> x {}
    add := f()
}
// ----
// ParserError 6272: (35-37): Cannot assign to builtin function "add".
