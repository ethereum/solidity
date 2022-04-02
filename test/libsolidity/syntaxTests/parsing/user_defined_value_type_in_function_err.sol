function f() {
    type(uint).max;
    type MyInt is int;
}
// ----
// ParserError 2314: (44-49='MyInt'): Expected ';' but got identifier
