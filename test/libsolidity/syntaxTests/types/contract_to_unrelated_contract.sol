contract A {}
contract B {}
contract C {
  function f() public pure {
    B b = B(new A());
  }
}
// ----
// TypeError 9640: (80-90='B(new A())'): Explicit type conversion not allowed from "contract A" to "contract B".
