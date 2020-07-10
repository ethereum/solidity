contract C {
  function f() public pure {
    assembly {
      pop(hex"2233")
    }
  }
}
// ----
// ParserError 2314: (70-76): Expected ',' but got 'StringLiteral'
