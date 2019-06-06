contract C {
  function f() public pure {
    assembly {
      function f(a, b) {}
      f()
      f(,1)
    }
  }
}
// ----
// ParserError: (101-102): Literal, identifier or instruction expected.
