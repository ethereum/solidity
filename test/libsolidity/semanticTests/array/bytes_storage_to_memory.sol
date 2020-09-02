contract C {
  bytes s = "abcd";
  function f() external returns (byte) {
    bytes memory data = s;
    return data[0];
  }
}
// ----
// f() -> "a"
