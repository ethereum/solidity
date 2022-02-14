function f() pure {
    assembly "evmasm" () {}
}
// ----
// ParserError 2314: (43-44): Expected 'StringLiteral' but got ')'
