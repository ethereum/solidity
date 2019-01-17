contract C {
  function f() public pure {
    assembly {
      function (a, b) {}
    }
  }
}
// ----
// ParserError: (72-73): Expected identifier but got '('
// ParserError: (79-80): Expected ';' but got '{'
