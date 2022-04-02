contract C {
  function f() public pure {
    assembly {
      function f.() {}
      function g.f() {}
    }
  }
}
// ----
// DeclarationError 3927: (63-79='function f.() {}'): User-defined identifiers in inline assembly cannot contain '.'.
// DeclarationError 3927: (86-103='function g.f() {}'): User-defined identifiers in inline assembly cannot contain '.'.
