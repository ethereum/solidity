contract C {
  function f() public {}
  struct S {f x;}
  function g(function(S memory) external) public {}
}
// ----
// TypeError: (50-51): Name has to refer to a struct, enum or contract.
