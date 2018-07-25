contract C {
  function f() public {
    assembly {
      super := 1
      f := 1
      C := 1
    }
  }
}
// ----
// TypeError: (58-63): Only local variables can be assigned to in inline assembly.
// TypeError: (75-76): Only local variables can be assigned to in inline assembly.
// TypeError: (88-89): Only local variables can be assigned to in inline assembly.
