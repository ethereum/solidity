contract C {
  function f() public {}
  struct S {f x;}
  function g(function(S memory) external) public {}
}
// ----
// TypeError 5172: (50-51): Name has to refer to a user-defined value type, struct, enum or contract.
