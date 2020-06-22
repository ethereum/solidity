contract C {
  function f() public pure {
    assembly {
      function k() {}

      k
    }
  }
}
// ----
// ParserError 6913: (92-93): Call or assignment expected.
