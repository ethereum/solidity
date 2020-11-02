contract C {
  bytes s;
  function f() external returns (byte) {
    bytes memory data = "abcd";
    s = data;
    return s[0];
  }
}
// ====
// compileViaYul: also
// ----
// f() -> "a"
