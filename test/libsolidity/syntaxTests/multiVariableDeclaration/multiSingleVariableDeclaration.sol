contract C {
  function f() internal returns (uint) {
    (uint a) = f();
    a;
  }
} 
