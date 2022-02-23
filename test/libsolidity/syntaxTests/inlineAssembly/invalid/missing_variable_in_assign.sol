contract C {
  function f() public pure {
    assembly {
      let x := mload(0)
      := 1
    }
  }
}
// ----
// ParserError 1856: (87-89): Literal or identifier expected.
