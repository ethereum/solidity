contract C {
  function f() public pure {
    abi.decode("abc", (bytes memory, uint[][2] memory));
  }
}
// ----
// ParserError: (71-77): Expected ',' but got 'memory'
