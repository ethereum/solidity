// This restriction might be lifted in the future
contract C {
  function f() public pure {
    abi.decode("abc", (bytes calldata));
  }
}
// ----
// ParserError: (121-129): Expected ',' but got 'calldata'
