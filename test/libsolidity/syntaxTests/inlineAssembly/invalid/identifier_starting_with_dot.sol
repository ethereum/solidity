contract C {
  function f() public pure {
    assembly {
      let a, .a, aa.b := f()
    }
  }
}
// ----
// ParserError 2314: (70-71): Expected identifier but got '.'
