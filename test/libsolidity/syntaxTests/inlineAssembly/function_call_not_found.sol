contract C {
  function f() public pure {
    assembly {
      k()
    }
  }
}
// ----
// DeclarationError 4619: (63-64='k'): Function "k" not found.
