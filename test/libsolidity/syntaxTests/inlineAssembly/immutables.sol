contract C {
  function f() public pure {
    assembly {
      loadimmutable("abc", 0)
      setimmutable("abc")
    }
  }
}
// ----
// DeclarationError: (63-76): Function not found.
// DeclarationError: (93-105): Function not found.
