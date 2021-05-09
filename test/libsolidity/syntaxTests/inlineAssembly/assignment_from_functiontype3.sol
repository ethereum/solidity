function ff() {}

contract C {
  function f() public pure {
    assembly {
      let x := ff
    }
  }
}
// ----
// DeclarationError 2025: (90-92): Access to functions is not allowed in inline assembly.
