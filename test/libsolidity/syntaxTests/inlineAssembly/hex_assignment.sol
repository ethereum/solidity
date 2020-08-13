contract C {
  function f() public pure {
    assembly {
      let x := hex"0011"
    }
  }
}
// ----
// ParserError 1856: (72-81): Literal or identifier expected.
