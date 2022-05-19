contract C {
  int[] data;
  function f() public returns (uint) {
    data.push(1);
    (data.pop)();
    return data.length;
  }
}
// ----
// f() -> 0
