contract C {
  struct S {t t;}
  function f(function(S memory) external) public {}
}
// ----
// TypeError: (25-26): Name has to refer to a struct, enum or contract.
