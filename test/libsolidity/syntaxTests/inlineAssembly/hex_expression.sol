contract C {
  function f() public pure {
    assembly {
      pop(hex"2233")
    }
  }
}
// ----
// ParserError 1856: (67-76): Literal or identifier expected.
