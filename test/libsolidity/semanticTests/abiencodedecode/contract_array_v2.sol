pragma experimental ABIEncoderV2;
contract C {
  function f(bytes calldata x) public returns (C[] memory) {
    return abi.decode(x, (C[]));
  }
}
// ====
// compileViaYul: also
// ----
// f(bytes): 0x20, 0xA0, 0x20, 3, 0x01, 0x02, 0x03 -> 0x20, 3, 0x01, 0x02, 0x03
// f(bytes): 0x20, 0x60, 0x20, 1, 0x0102030405060708090a0b0c0d0e0f1011121314 -> 0x20, 1, 0x0102030405060708090a0b0c0d0e0f1011121314
// f(bytes): 0x20, 0x60, 0x20, 1, 0x0102030405060708090a0b0c0d0e0f101112131415 -> FAILURE
