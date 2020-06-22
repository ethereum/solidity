contract C {
  function f() public pure {
    assembly {
      let x : = mload(0)
    }
  }
}
// ----
// ParserError 2314: (71-72): Expected identifier but got '='
