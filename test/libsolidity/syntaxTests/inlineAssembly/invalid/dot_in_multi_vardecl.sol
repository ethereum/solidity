contract C {
  function g() public pure {
    assembly {
      function f() -> x, y, z {}
      let a., aa.b := f()
    }
  }
}
// ----
// DeclarationError 3927: (100-102): User-defined identifiers in inline assembly cannot contain '.'.
// DeclarationError 3927: (104-108): User-defined identifiers in inline assembly cannot contain '.'.
