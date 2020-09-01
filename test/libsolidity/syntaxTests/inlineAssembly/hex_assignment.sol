contract C {
  function f() public pure {
    assembly {
      let x := hex"0011"
    }
  }
}
// ----
// ParserError 3772: (72-81): Hex literals are not valid in this context.
