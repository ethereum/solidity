contract C {
  bytes s;
  function f(bytes calldata data) external returns (byte) {
    s = data;
    return s[0];
  }
}
// ====
// compileViaYul: also
// ----
// f(bytes): 0x20, 0x08, "abcdefgh" -> "a"
