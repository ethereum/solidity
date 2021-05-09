contract C {
  bytes s;
  function f() external returns (bytes1) {
    bytes memory data = "abcd";
    s = data;
    return s[0];
  }
}
// ====
// compileToEwasm: also
// compileViaYul: also
// ----
// f() -> "a"
