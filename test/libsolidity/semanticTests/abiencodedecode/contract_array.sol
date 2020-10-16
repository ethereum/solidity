contract C {
  function f(bytes calldata x) public returns (C[] memory) {
    return abi.decode(x, (C[]));
  }
}
// ====
// compileViaYul: also
// ----
// f(bytes): 0x20, 0xA0, 0x20, 3, 0x01, 0x02, 0x03 -> 0x20, 3, 0x01, 0x02, 0x03
