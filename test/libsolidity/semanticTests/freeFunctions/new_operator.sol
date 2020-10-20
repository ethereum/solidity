contract C {
  uint public x = 2;
}

function test() returns (uint) {
  return (new C()).x();
}

contract D {
  function f() public returns (uint) {
    return test();
  }
}
// ====
// compileViaYul: true
// ----
// f() -> 2
