contract I {
  function f() external {}
}
contract A is I {}
contract B is I {}
contract C is A, B {}
