contract C {
    function f() pure public {
        (1 type).max;
    }
}
// ----
// ParserError 2314: (55-59): Expected ',' but got 'type'
