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
// compileToEwasm: also
// ----
// f() -> FAILURE, hex"4e487b71", 0x51
// f() -> FAILURE, hex"4e487b71", 0x51
