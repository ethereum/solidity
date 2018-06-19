contract A {}
contract B {}
contract C {
  function f() public pure {
    B b = B(new A());
  }
}
// ----
// TypeError: (80-90): Explicit type conversion not allowed from "contract A" to "contract B".
