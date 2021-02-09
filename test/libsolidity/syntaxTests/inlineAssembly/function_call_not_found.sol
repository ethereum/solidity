contract C {
  function f() public pure {
    assembly {
      k()
    }
  }
}
// ----
// DeclarationError 4619: (63-64): Function "k" not found.
