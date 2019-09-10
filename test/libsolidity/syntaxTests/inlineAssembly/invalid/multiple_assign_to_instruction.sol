contract C {
  function f() public pure {
    assembly {
      function g() -> a,b, c {}
      let a, sub, mov := g()
    }
  }
}
// ----
// ParserError: (102-105): Cannot use builtin function name "sub" as identifier name.
