contract C {
  function f() public pure {
    assembly {
      function g() -> a,b, c {}
      let x, y ,z : = g()
    }
  }
}
// ----
// ParserError: (107-108): Literal, identifier or instruction expected.
// ParserError: (107-108): Expected primary expression.
