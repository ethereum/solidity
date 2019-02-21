contract C {
  function f() public pure {
    assembly {
      function (a, b) {}
    }
  }
}
// ----
// ParserError: (72-73): Expected identifier but got '('
