pragma abicoder               v2;
contract C {
  function f(bytes calldata x) public returns (C[] memory) {
    return abi.decode(x, (C[]));
  }
  function g() public returns (bytes memory) {
    C[] memory c = new C[](3);
    c[0] = C(address(0x42));
    c[1] = C(address(0x21));
    c[2] = C(address(0x23));
    return abi.encode(c);
  }
}
// ----
// f(bytes): 0x20, 0xA0, 0x20, 3, 0x01, 0x02, 0x03 -> 0x20, 3, 0x01, 0x02, 0x03
// f(bytes): 0x20, 0x60, 0x20, 1, 0x0102030405060708090a0b0c0d0e0f1011121314 -> 0x20, 1, 0x0102030405060708090a0b0c0d0e0f1011121314
// f(bytes): 0x20, 0x60, 0x20, 1, 0x0102030405060708090a0b0c0d0e0f101112131415 -> FAILURE
// g() -> 0x20, 0xa0, 0x20, 3, 0x42, 0x21, 0x23
