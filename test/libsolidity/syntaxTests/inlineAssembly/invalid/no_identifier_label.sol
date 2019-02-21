contract C {
  function f() public pure {
    assembly {
      return : 1
    }
  }
}
// ----
// ParserError: (70-71): Label name must precede ":".
// ParserError: (70-71): Expected primary expression.
