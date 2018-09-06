contract B {}
contract A is B {}
contract C {
  function f() public pure {
    A a = A(new B());
  }
}
// ----
// TypeError: (85-95): Explicit type conversion not allowed from "contract B" to "contract A".
