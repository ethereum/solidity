contract C {
  function f() public pure {
    assembly {
      setimmutable("abc", 0)
      loadimmutable("abc")
    }
  }
}
// ----
// DeclarationError 4619: (63-75): Function "setimmutable" not found.
// DeclarationError 4619: (92-105): Function "loadimmutable" not found.
