contract C {
  function f() public pure {
    assembly {
      let mod := 2
    }
  }
}
// ----
// ParserError 5568: (67-70): Cannot use builtin function name "mod" as identifier name.
