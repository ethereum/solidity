contract C {
  function f() public pure returns (uint) {
    return abi.decode("abc", (uint));
  }
}
// ----
