contract C {
  function f() public pure {
    assembly {
      k()
    }
  }
}
// ----
// DeclarationError: (63-64): Function not found.
