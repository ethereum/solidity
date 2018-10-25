contract C {
  function f() public pure {
    assembly {
      let x := mload(0)
      := 1
    }
  }
}
// ----
// ParserError: (87-88): Literal, identifier or instruction expected.
// ParserError: (87-88): Expected primary expression.
