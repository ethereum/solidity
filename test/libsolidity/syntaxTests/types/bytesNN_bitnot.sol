contract C {
  bytes32 b32 = ~bytes32(hex"ff");
  bytes32 b25 = ~bytes25(hex"ff");
  bytes25 b8 = ~bytes8(hex"ff");
}
