contract test {
    function f() pure public { "abc\
// ----
// ParserError: (47-53): Expected string end-quote.
