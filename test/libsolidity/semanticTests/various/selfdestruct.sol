contract C {
  event Terminated();

  constructor() payable {
  }

  function terminate() external {
    emit Terminated();
    selfdestruct(payable(msg.sender));
    // Execution stops here, so the second one is not executed.
    selfdestruct(payable(msg.sender));
    emit Terminated();
  }
}

contract D {
  C public c;

  constructor() payable {
      c = new C{value: 1 ether}();
  }

  function f() external {
      c.terminate();
  }

  function exists() external returns (bool) {
      return address(c).code.length != 0;
  }
}
// ----
// constructor(), 1 ether ->
// gas irOptimized: 186970
// gas legacy: 255973
// gas legacyOptimized: 178919
// c() -> 0x137aa4dfc0911524504fcd4d98501f179bc13b4a
// balance: 0x137aa4dfc0911524504fcd4d98501f179bc13b4a -> 1000000000000000000
// balance -> 0
// exists() -> true
// f() ->
// ~ emit Terminated() from 0x137aa4dfc0911524504fcd4d98501f179bc13b4a
// balance: 0x137aa4dfc0911524504fcd4d98501f179bc13b4a -> 0
// ~ emit Terminated() from 0x137aa4dfc0911524504fcd4d98501f179bc13b4a
// balance -> 1000000000000000000
// ~ emit Terminated() from 0x137aa4dfc0911524504fcd4d98501f179bc13b4a
// exists() -> false
