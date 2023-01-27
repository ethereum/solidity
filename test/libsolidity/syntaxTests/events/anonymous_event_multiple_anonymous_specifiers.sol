contract c {
    event e(uint a) anonymous anonymous;
}
// ----
// ParserError 2314: (43-52): Expected ';' but got 'anonymous'