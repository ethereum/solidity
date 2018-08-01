contract C {
  function f() internal {
    {
      (uint a, uint b, uint c) = (a, b, c);
    }
  }
}
// ----
// DeclarationError: (79-80): Undeclared identifier. "a" is not (or not yet) visible at this point.
// DeclarationError: (82-83): Undeclared identifier. "b" is not (or not yet) visible at this point.
// DeclarationError: (85-86): Undeclared identifier. "c" is not (or not yet) visible at this point.
