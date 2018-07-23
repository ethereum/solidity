contract C {
  function f() public {
    bytes memory a;
    a[];
  }
}
// ----
// TypeError: (61-64): Index expression cannot be omitted.
