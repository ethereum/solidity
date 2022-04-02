contract C {
  function f() public {
    bytes memory a;
    a[];
  }
}
// ----
// TypeError 9689: (61-64='a[]'): Index expression cannot be omitted.
