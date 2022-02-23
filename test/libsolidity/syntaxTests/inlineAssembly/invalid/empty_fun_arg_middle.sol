contract C {
  function f() public pure {
    assembly {
      function f(a, b, c) {}
      f(1,,1)
    }
  }
}
// ----
// ParserError 1856: (96-97): Literal or identifier expected.
