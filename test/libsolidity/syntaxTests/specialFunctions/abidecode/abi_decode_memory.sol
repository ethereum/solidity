contract C {
  function f() public pure {
    abi.decode("abc", (bytes memory, uint[][2] memory));
  }
}
// ----
// ParserError 2314: (71-77='memory'): Expected ',' but got 'memory'
