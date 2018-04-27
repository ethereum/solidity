contract C {
  function f() internal returns (uint, uint, uint, uint) {
    (uint a, uint b,,) = f();
    a; b;
  }
  function g() internal returns (bytes memory, string storage) {
    (bytes memory a, string storage b) = g();
    a; b;
  }
} 
// ----
// Warning: (163-169): This variable is of storage pointer type and might be returned without assignment. This can cause storage corruption. Assign the variable (potentially from itself) to remove this warning.
