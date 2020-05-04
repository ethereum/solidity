contract C {
  function f() public pure {
    assembly {
      setimmutable("abc", 0)
      loadimmutable("abc")
    }
  }
}
// ----
// DeclarationError: (63-75): Function not found.
// DeclarationError: (92-105): Function not found.
