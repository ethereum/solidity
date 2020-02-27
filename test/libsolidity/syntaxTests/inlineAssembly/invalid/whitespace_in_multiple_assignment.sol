contract C {
  function f() public pure {
    assembly {
      function g() -> a,b, c {}
      let x, y ,z : = g()
    }
  }
}
// ----
// ParserError: (109-110): Expected identifier but got '='
