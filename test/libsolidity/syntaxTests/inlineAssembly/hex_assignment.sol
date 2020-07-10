contract C {
  function f() public pure {
    assembly {
      let x := hex"0011"
    }
  }
}
// ----
// ParserError 6913: (86-87): Call or assignment expected.
