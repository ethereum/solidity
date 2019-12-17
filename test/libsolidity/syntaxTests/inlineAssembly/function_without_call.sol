contract C {
  function f() public pure {
    assembly {
      function k() {}

      k
    }
  }
}
// ----
// ParserError: (92-93): Call or assignment expected.
