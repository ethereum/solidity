contract test {
  function f() public pure returns (bytes memory) {
    return bytes("abc");
  }
}
