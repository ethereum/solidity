contract InvalidTest {

  function() internal storedFn;

  bool flag;

  constructor() {
    function() internal invalid;
    storedFn = invalid;
  }
  function f() public returns (uint) {
    if (flag) return 2;
    flag = true;
    storedFn();
  }
}
// ====
// compileViaYul: also
// compileToEwasm: also
// ----
// f() -> FAILURE
// f() -> FAILURE
