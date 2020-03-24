contract C {
  function f() public {}
  struct S {f x;}
  function g(function(S memory) external) public {}
}
// ----
// TypeError: (50-51): Name has to refer to a struct, enum or contract.
// TypeError: (78-86): Internal type cannot be used for external function type.
