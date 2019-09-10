contract C {
  function f() public pure {
    assembly {
      function f(a, b) {}
      f()
      f(1,)
      f(,1)
    }
  }
}
// ----
// ParserError: (113-114): Literal, identifier or instruction expected.
