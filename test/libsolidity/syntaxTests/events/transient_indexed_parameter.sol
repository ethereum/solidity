contract C {
    event e(string indexed transient a);
}
// ----
// ParserError 2314: (50-51): Expected ',' but got identifier
