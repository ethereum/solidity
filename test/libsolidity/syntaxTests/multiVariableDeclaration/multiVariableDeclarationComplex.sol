contract D {
  struct S { uint a; uint b; }
}
contract C {
  function f() internal returns (uint, uint, uint, D.S[20] storage, uint) {
    (,,,D.S[10*2] storage x,) = f();
    x;
  }
} 
// ----
// Warning: (110-117): This variable is of storage pointer type and might be returned without assignment. This can cause storage corruption. Assign the variable (potentially from itself) to remove this warning.
