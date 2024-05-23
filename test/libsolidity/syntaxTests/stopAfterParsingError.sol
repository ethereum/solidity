contract C {
    uint storage x;
}
// ====
// stopAfter:parsing
// ----
// ParserError 2314: (22-29): Expected identifier but got 'storage'
