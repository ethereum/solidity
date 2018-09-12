contract C {
  function f() public pure returns (uint, bytes32, C) {
    return abi.decode("abc", (uint, bytes32, C));
  }
}
