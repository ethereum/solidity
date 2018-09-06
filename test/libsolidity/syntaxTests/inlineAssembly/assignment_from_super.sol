contract C {
  function f() public pure {
    assembly {
      let x := super
    }
  }
}
// ----
// DeclarationError: (72-77): Identifier not found.
