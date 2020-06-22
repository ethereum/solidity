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
// ParserError 1856: (103-104): Literal or identifier expected.
