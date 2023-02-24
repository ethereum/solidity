contract C {
    modifier m(uint) suffix {}
}
// ----
// ParserError 2314: (34-40): Expected '{' but got identifier
