contract C {
  bytes s;
  function f(bytes calldata data) external returns (bytes1) {
    s = data;
    return s[0];
  }
}
// ----
// f(bytes): 0x20, 0x08, "abcdefgh" -> "a"
