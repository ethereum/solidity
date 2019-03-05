contract C {
  function f() public pure {
    assembly {
      let x : = mload(0)
    }
  }
}
// ----
// ParserError: (69-70): Literal, identifier or instruction expected.
