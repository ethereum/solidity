contract C {
  function f() public {
    assembly {
      super := 1
      this := 1
      msg := 1
      block := 1
      f := 1
      C := 1
    }
  }
}
// ----
// DeclarationError: (58-63): Variable not found or variable not lvalue.
// DeclarationError: (75-79): Variable not found or variable not lvalue.
// DeclarationError: (91-94): Variable not found or variable not lvalue.
// DeclarationError: (106-111): Variable not found or variable not lvalue.
// TypeError: (123-124): Only local variables can be assigned to in inline assembly.
// TypeError: (136-137): Only local variables can be assigned to in inline assembly.
