contract C {
  function f() public pure {
    assembly {
      let x := 1

      x()
    }
  }
}
// ----
// TypeError: (81-82): Attempt to call variable instead of function.
