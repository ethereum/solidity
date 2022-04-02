function g() pure {
    assembly { let hex := 1 }
}
// ----
// ParserError 2314: (39-42='hex'): Expected identifier but got 'ILLEGAL'
