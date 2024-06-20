contract C {
    uint storage transient x;
}
// ----
// ParserError 2314: (22-29): Expected identifier but got 'storage'
