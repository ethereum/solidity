contract C {
  function f() public pure {
    assembly {
      function f(a., x.b) -> t.b, b.. {}
    }
  }
}
// ----
// DeclarationError 3927: (74-76='a.'): User-defined identifiers in inline assembly cannot contain '.'.
// DeclarationError 3927: (78-81='x.b'): User-defined identifiers in inline assembly cannot contain '.'.
// DeclarationError 3927: (86-89='t.b'): User-defined identifiers in inline assembly cannot contain '.'.
// DeclarationError 3927: (91-94='b..'): User-defined identifiers in inline assembly cannot contain '.'.
