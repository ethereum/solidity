contract C {
  function f() public pure {
    assembly {
      let x := 0100
    }
  }
}
// ----
// ParserError 1465: (72-73): Illegal token: Octal numbers not allowed.
