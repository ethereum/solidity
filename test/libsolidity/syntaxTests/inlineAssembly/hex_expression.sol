contract C {
  function f() public pure {
    assembly {
      pop(hex"2233")
    }
  }
}
// ----
// ParserError 3772: (67-76): Hex literals are not valid in this context.
