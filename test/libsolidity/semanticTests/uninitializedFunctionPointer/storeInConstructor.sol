contract InvalidTest {

  function() internal storedFn;

  bool flag;

  constructor() public {
    function() internal invalid;
    storedFn = invalid;
  }
  function f() public returns (uint) {
    if (flag) return 2;
    flag = true;
    storedFn();
  }
}
// ----
// f() -> FAILURE
// f() -> FAILURE
