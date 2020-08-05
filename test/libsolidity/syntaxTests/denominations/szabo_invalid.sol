contract C {
  function f() {
    uint x = 1 szabo;
  }
}
// ----
// ParserError 2314: (45-50): Expected ';' but got identifier
