contract C {
  function f() public pure {
    assembly {
      x := 1
    }
  }
}
// ----
// DeclarationError 4634: (63-64='x'): Variable not found or variable not lvalue.
