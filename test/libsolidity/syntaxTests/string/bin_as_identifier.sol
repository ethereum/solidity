function g() pure {
    assembly { let bin := 1 }
}
// ----
// ParserError 2314: (39-42): Expected identifier but got 'ILLEGAL'
