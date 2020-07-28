contract C {
  function f() public pure {
    assembly {
      let a. := 2
      let a.. := 2
      let a.b := 2
      let a..b := 2
    }
  }
}
// ----
// DeclarationError 3927: (67-69): User-defined identifiers in inline assembly cannot contain '.'.
// DeclarationError 3927: (85-88): User-defined identifiers in inline assembly cannot contain '.'.
// DeclarationError 3927: (104-107): User-defined identifiers in inline assembly cannot contain '.'.
// DeclarationError 3927: (123-127): User-defined identifiers in inline assembly cannot contain '.'.
