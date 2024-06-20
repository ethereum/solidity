contract C {
    uint transient storage x;
}
// ----
// ParserError 2314: (32-39): Expected identifier but got 'storage'
