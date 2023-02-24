contract C {
    function f() pure public {
        1 0x1234567890123456789012345678901234567890.send;
    }
}
// ----
// ParserError 2314: (54-96): Expected ';' but got 'Number'
