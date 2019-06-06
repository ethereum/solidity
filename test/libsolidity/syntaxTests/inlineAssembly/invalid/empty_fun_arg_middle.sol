contract C {
  function f() public pure {
    assembly {
      function f(a, b, c) {}
      f(1,,1)
    }
  }
}
// ----
// ParserError: (96-97): Literal, identifier or instruction expected.
