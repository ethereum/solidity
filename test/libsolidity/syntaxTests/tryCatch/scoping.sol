contract Test {
  // This checks a scoping error,
  // the variable "a" was not visible
  // at the assignment.
  function test(address _ext) external {
    try Test(_ext).test(_ext) {} catch {}
    uint a = 1;
    a = 3;
  }
}
// ----
