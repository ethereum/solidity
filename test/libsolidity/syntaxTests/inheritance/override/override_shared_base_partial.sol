contract I {
  function f() external {}
}
contract A is I {
  function f() external override {}
}
contract B is I {}
contract C is A, B {
  function f() external override(A, I) {}
}
