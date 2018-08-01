contract A {}
contract B is A {}
contract C {
  function f() public {
    A a = new B();
    a;
  }
}
// ----
