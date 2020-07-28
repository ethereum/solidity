contract C {
  function f(bytes calldata data) external returns (bytes32) {
    return keccak256(bytes(data));
  }
}
// ====
// compileViaYul: also
// ----
// f(bytes): 0x20, 0x08, "abcdefgh" -> 0x48624fa43c68d5c552855a4e2919e74645f683f5384f72b5b051b71ea41d4f2d
