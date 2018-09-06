contract C {
  function f() public {
    bytes32 b;
    b[];
  }
}
// ----
// TypeError: (56-59): Index expression cannot be omitted.
