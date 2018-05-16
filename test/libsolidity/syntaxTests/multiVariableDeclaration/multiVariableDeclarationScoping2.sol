pragma experimental "v0.5.0";

contract C {
  function f() internal {
    {
      (uint a, uint b, uint c) = (a, b, c);
    }
  }
} 
// ----
// DeclarationError: (110-111): Undeclared identifier. Did you mean "a"?
// DeclarationError: (113-114): Undeclared identifier. Did you mean "b"?
// DeclarationError: (116-117): Undeclared identifier. Did you mean "c"?
