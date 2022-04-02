contract C {
  function f() internal {
    {
      (uint a, uint b, uint c) = (a, b, c);
    }
  }
}
// ----
// DeclarationError 7576: (79-80='a'): Undeclared identifier. "a" is not (or not yet) visible at this point.
// DeclarationError 7576: (82-83='b'): Undeclared identifier. "b" is not (or not yet) visible at this point.
// DeclarationError 7576: (85-86='c'): Undeclared identifier. "c" is not (or not yet) visible at this point.
