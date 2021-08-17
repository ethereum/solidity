contract C {
  function g() public pure {
    assembly {
      function f(a., x.b) -> t.b, b.. {}
    }
  }
}
// ----
// DeclarationError 3927: (74-76): User-defined identifiers in inline assembly cannot contain '.'.
// DeclarationError 3927: (78-81): User-defined identifiers in inline assembly cannot contain '.'.
// DeclarationError 3927: (86-89): User-defined identifiers in inline assembly cannot contain '.'.
// DeclarationError 3927: (91-94): User-defined identifiers in inline assembly cannot contain '.'.
