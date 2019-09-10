contract C {
  function f() public pure {
    assembly {
      let x := mload(0)
      := 1
    }
  }
}
// ----
// ParserError: (87-89): Literal, identifier or instruction expected.
