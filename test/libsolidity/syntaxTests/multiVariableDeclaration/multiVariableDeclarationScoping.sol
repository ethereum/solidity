pragma experimental "v0.5.0";

contract C {
  function f() internal {
    {
      (uint a, uint b, uint c) = (1, 2, 3);
    }
    a;
  }
} 
// ----
// DeclarationError: (130-131): Undeclared identifier.
