contract C {
  function f() public pure {
    assembly {
      let x := f
    }
  }
}
// ----
// TypeError: (72-73): Access to functions is not allowed in inline assembly.
