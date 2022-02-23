contract C {
  function f() public pure {
    assembly {
      let x := super
    }
  }
}
// ----
// DeclarationError 8198: (72-77): Identifier "super" not found.
