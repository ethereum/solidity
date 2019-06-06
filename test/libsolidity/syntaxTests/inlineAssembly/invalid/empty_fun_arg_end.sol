contract C {
  function f() public pure {
    assembly {
      function f(a, b) {}
      f()
      f(1,)
    }
  }
}
// ----
// ParserError: (103-104): Literal, identifier or instruction expected.
