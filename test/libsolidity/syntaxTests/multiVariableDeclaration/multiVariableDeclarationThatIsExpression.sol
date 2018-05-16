contract C {
  struct S { function() returns (S storage)[] x; }
  S s;
  function f() internal pure returns (uint, uint, uint, S storage, uint, uint) {
    (,,,s.x[2](),,) = f();
  }
} 
// ----
// TypeError: (160-168): Expression has to be an lvalue.
