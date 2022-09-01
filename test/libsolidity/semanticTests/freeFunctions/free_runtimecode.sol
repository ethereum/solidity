contract C {
  uint public x = 2;
}

function test() returns (bool) {
  return type(C).runtimeCode.length > 20;
}

contract D {
  function f() public returns (bool) {
    return test();
  }
}
// ----
// f() -> true
