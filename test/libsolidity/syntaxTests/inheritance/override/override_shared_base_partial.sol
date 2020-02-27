contract I {
  function f() external virtual {}
}
contract A is I {
  function f() external virtual override {}
}
contract B is I {}
contract C is A, B {
  function f() external override(A, I) {}
}
// ----
// Warning: (178-192): Override specifier list order differs from inheritance order.
