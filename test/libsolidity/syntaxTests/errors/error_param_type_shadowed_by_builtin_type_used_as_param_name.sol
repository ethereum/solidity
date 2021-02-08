contract C {
    error E(int bytes, bytes x);
}
// ----
// ParserError 2314: (29-34): Expected ',' but got 'bytes'
