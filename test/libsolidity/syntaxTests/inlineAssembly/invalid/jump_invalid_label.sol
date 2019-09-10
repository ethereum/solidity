contract C {
  function f() public pure {
    assembly {
      jump(xy)
    }
  }
}
// ----
// DeclarationError: (68-70): Identifier not found.
// SyntaxError: (63-71): Jump instructions and labels are low-level EVM features that can lead to incorrect stack access. Because of that they are discouraged. Please consider using "switch", "if" or "for" statements instead.
