contract C {
  function f() {
    uint x = 1 finney;
  }
}
// ----
// ParserError 2314: (45-51): Expected ';' but got identifier
